/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mpmm.h>

#include <mod_mpmm.h>
#include <mod_power_domain.h>
#include <mod_scmi_perf.h>

#include <interface_amu.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_string.h>

struct mod_mpmm_core_ctx {
    /* Core Identifier */
    fwk_id_t core_id;

    /* MPMM registers */
    struct mpmm_reg *mpmm;

    /* The core is online */
    bool online;

    /* Current selected threshold */
    uint32_t threshold;

    /* Cached counters */
    uint64_t *cached_counters;

    /* Thresholds delta */
    uint64_t *delta;

    /* Used to block the PD when transitioning from OFF to ON */
    bool pd_blocked;

    /* Cookie to un-block the PD transition from OFF to ON */
    uint32_t cookie;

    /* Identifier of the base AMU Auxiliry counter */
    fwk_id_t base_aux_counter_id;
};

struct mod_mpmm_domain_ctx {
    /* Context Domain ID */
    fwk_id_t domain_id;

    /* Number of cores to monitor */
    uint32_t num_cores;

    /* Number of cores online */
    uint32_t num_cores_online;

    /* Threshold map */
    uint32_t threshold_map;

    /* Latest perf level value as reported by the plugin handler */
    uint32_t current_perf_level;

    /* Latest perf limit value required by mpmm */
    uint32_t perf_limit;

    /* Wait for the report callback to confirm perf transition completion */
    bool wait_for_perf_transition;

    /* Core context */
    struct mod_mpmm_core_ctx core_ctx[MPMM_MAX_NUM_CORES_IN_DOMAIN];

    /* Domain configuration */
    const struct mod_mpmm_domain_config *domain_config;
};

static struct mod_mpmm_ctx {
    /* Number of MPMM domains */
    uint32_t mpmm_domain_count;

    /* Domain context table */
    struct mod_mpmm_domain_ctx *domain_ctx;

    /* Perf plugin API */
    struct perf_plugins_handler_api *perf_plugins_handler_api;

    /* AMU driver API */
    struct amu_api *amu_driver_api;

    /* AMU driver API for access AMU Auxiliry registers */
    fwk_id_t amu_driver_api_id;
} mpmm_ctx;

/*
 * MPMM and AMU Registers access functions
 */

/* Read the number of MPMM threshold levels. */
static void mpmm_core_get_number_of_thresholds(
    struct mod_mpmm_core_ctx *core_ctx,
    uint32_t *num_thresholds)
{
    *num_thresholds =
        ((core_ctx->mpmm->PPMCR >> MPMM_PPMCR_NUM_GEARS_POS) &
         MPMM_PPMCR_NUM_GEARS_MASK);
}

/* Check if the MPMM throttling and MPMM counters are enabled for a core. */
static void mpmm_core_check_enabled(
    struct mod_mpmm_core_ctx *core_ctx,
    bool *enabled)
{
    *enabled =
        (bool)((core_ctx->mpmm->MPMMCR >> MPMM_MPMMCR_EN_POS) & MPMM_MPMMCR_EN_MASK);
}

/* Set the MPMM threshold for a specific core. */
static void mpmm_core_set_threshold(struct mod_mpmm_core_ctx *core_ctx)
{
    core_ctx->mpmm->MPMMCR |=
        ((core_ctx->threshold & MPMM_MPMMCR_GEAR_MASK) << MPMM_MPMMCR_GEAR_POS);
}

/*
 * MPMM Module Helper Functions
 */
static struct mod_mpmm_domain_ctx *get_domain_ctx(fwk_id_t domain_id)
{
    uint32_t idx = fwk_id_get_element_idx(domain_id);

    if (idx < mpmm_ctx.mpmm_domain_count) {
        return &mpmm_ctx.domain_ctx[idx];
    } else {
        return NULL;
    }
}

static void mpmm_core_counters_delta(
    struct mod_mpmm_domain_ctx *domain_ctx,
    struct mod_mpmm_core_ctx *core_ctx)
{
    int status;
    uint32_t th_count = domain_ctx->domain_config->num_threshold_counters;
    uint32_t i;
    static uint64_t counter_buff[MPMM_MAX_THRESHOLD_COUNT];

    fwk_str_memset(counter_buff, 0, sizeof(counter_buff));

    status = mpmm_ctx.amu_driver_api->get_counters(
        core_ctx->base_aux_counter_id, counter_buff, th_count);
    if (status != FWK_SUCCESS) {
        FWK_LOG_DEBUG(
            "[MPMM] %s @%d: AMU counter read fail, error=%d",
            __func__,
            __LINE__,
            status);
        return;
    }

    /*
     * Each MPMM threshold has an associated counter. The counters are
     * indexed in the same order as the MPMM thresholds for the platform.
     */
    for (i = 0; i < th_count; i++) {
        /* Calculate the delta */
        if (counter_buff[i] < core_ctx->cached_counters[i]) {
            /* Counter wraparound case */
            core_ctx->delta[i] = UINT64_MAX - core_ctx->cached_counters[i];
            core_ctx->delta[i] += counter_buff[i];
        } else {
            core_ctx->delta[i] = counter_buff[i] - core_ctx->cached_counters[i];
        }
        /* Store the last value */
        core_ctx->cached_counters[i] = counter_buff[i];
    }
}

/*
 * This function returns the selected threshold based on the btc value.
 */
static uint32_t mpmm_core_threshold_policy(
    struct mod_mpmm_domain_ctx *domain_ctx,
    struct mod_mpmm_core_ctx *core_ctx)
{
    uint32_t thr_idx;
    uint32_t const highest_gear =
        domain_ctx->domain_config->num_threshold_counters;

    /*
     * Select the highest gear whose counter delta is just below the btc value.
     */
    for (thr_idx = 0; thr_idx < highest_gear; thr_idx++) {
        if (core_ctx->delta[thr_idx] <= domain_ctx->domain_config->btc) {
            return thr_idx;
        }
    }

    /*
     * It is not expected that all counters will cross the BTC. If this scenario
     * is encountered set throttling to a minimum.
     */
    return (highest_gear - 1);
}

/* set the threshold for all cores */
static void mpmm_domain_set_thresholds(struct mod_mpmm_domain_ctx *ctx)
{
    uint32_t core_idx;
    struct mod_mpmm_core_ctx *core_ctx;

    for (core_idx = 0; core_idx < ctx->num_cores; core_idx++) {
        core_ctx = &ctx->core_ctx[core_idx];
        if (core_ctx->online) {
            mpmm_core_set_threshold(core_ctx);
        }
    }
}

static void mpmm_core_evaluate_threshold(
    struct mod_mpmm_domain_ctx *domain_ctx,
    struct mod_mpmm_core_ctx *core_ctx)
{
    bool enabled;

    /* If counters are not enabled exit */
    mpmm_core_check_enabled(core_ctx, &enabled);
    if (!enabled) {
        core_ctx->threshold = domain_ctx->domain_config->num_threshold_counters;
        return;
    }

    /* Read counters */
    mpmm_core_counters_delta(domain_ctx, core_ctx);

    /* Threshold selection policy */
    core_ctx->threshold = mpmm_core_threshold_policy(domain_ctx, core_ctx);

    return;
}

static uint32_t find_perf_limit_from_pct(
    struct mod_mpmm_pct_table *pct_config,
    uint32_t threshold_map)
{
    int j;

    for (j = pct_config->num_perf_limits - 1; j >= 0; j--) {
        if (threshold_map <= pct_config->threshold_perf[j].threshold_bitmap) {
            return pct_config->threshold_perf[j].perf_limit;
        }
    }

    /* If no threshold_map was found select the default limits */
    return pct_config->default_perf_limit;
}

/* Convert the thresholds into a bitmap as described by the PCT */
static void mpmm_build_threshold_map(struct mod_mpmm_domain_ctx *ctx)
{
    uint32_t i, j, thr_tmp, thr_map = 0;
    uint32_t threshold_array[MPMM_MAX_NUM_CORES_IN_DOMAIN] = { 0 };

    /* Copy all CPU threshold values to an array */
    for (i = 0, j = 0; i < ctx->num_cores; i++) {
        if (ctx->core_ctx[i].online) {
            threshold_array[j] = ctx->core_ctx[i].threshold;
            j++;
        }
    }

    /* Threshold sorting in descending order */
    for (i = 0; i < ctx->num_cores_online; i++) {
        for (j = i + 1; j < ctx->num_cores_online; j++) {
            if (threshold_array[i] > threshold_array[j]) {
                thr_tmp = threshold_array[i];
                threshold_array[i] = threshold_array[j];
                threshold_array[j] = thr_tmp;
            }
        }
    }

    for (i = 0; i < ctx->num_cores_online; i++) {
        thr_map |= (threshold_array[i] << (MPMM_THRESHOLD_MAP_NUM_OF_BITS * i));
    }

    ctx->threshold_map = thr_map;
}

static uint32_t mpmm_evaluate_perf_limit(struct mod_mpmm_domain_ctx *ctx)
{
    struct mod_mpmm_pct_table *pct_config;
    size_t pct_size;
    int pct_idx;

    /* Parse PCT table from the bottom-up*/
    pct_config = ctx->domain_config->pct;
    pct_size = ctx->domain_config->pct_size;
    /* Start from the last index */
    for (pct_idx = (pct_size - 1); pct_idx >= 0; pct_idx--) {
        /* Find the entry based on the number of online cores */
        if (ctx->num_cores_online <= pct_config[pct_idx].cores_online) {
            /* Find the performance limit */
            return find_perf_limit_from_pct(
                &pct_config[pct_idx], ctx->threshold_map);
        }
    }

    /* If no entry was found, select the highest number of cores available */
    return find_perf_limit_from_pct(&pct_config[0], ctx->threshold_map);
}

/* Check CPU status and update performance limits accordingly */
static void mpmm_monitor_and_control(struct mod_mpmm_domain_ctx *domain_ctx)
{
    uint32_t core_idx;
    struct mod_mpmm_core_ctx *core_ctx;

    if (domain_ctx->num_cores_online == 0) {
        return;
    }

    /* Core level algorithm */
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        core_ctx = &domain_ctx->core_ctx[core_idx];

        if (!core_ctx->online) {
            continue;
        }

        mpmm_core_evaluate_threshold(domain_ctx, core_ctx);
    }

    mpmm_build_threshold_map(domain_ctx);

    /* Cache the last value */
    domain_ctx->perf_limit = mpmm_evaluate_perf_limit(domain_ctx);
}

/* Module APIs */

/*
 * update function should be called periodically to monitor the threshold
 * counters and update the performance limits.
 *
 */
static int mpmm_update(struct perf_plugins_perf_update *data)
{
    struct mod_mpmm_domain_ctx *domain_ctx;
    uint32_t domain_idx;
    /*
     * Get the performance element id from the sub-element provided in the
     * function argument.
     */
    fwk_id_t perf_id = FWK_ID_ELEMENT(
        FWK_MODULE_IDX_DVFS, fwk_id_get_element_idx(data->domain_id));

    for (domain_idx = 0; domain_idx < mpmm_ctx.mpmm_domain_count;
         domain_idx++) {
        if (fwk_id_is_equal(
                mpmm_ctx.domain_ctx[domain_idx].domain_config->perf_id,
                perf_id)) {
            break;
        }
    }

    if (domain_idx == mpmm_ctx.mpmm_domain_count) {
        return FWK_E_PARAM;
    }

    domain_ctx = &mpmm_ctx.domain_ctx[domain_idx];

    if (domain_ctx->num_cores_online == 0) {
        return FWK_SUCCESS;
    }

    mpmm_monitor_and_control(domain_ctx);

    /* Update the new performance limits */
    data->adj_max_limit[0] = domain_ctx->perf_limit;

    /* set the flag to wait for the transition to complete. */
    if (domain_ctx->perf_limit < domain_ctx->current_perf_level) {
        domain_ctx->wait_for_perf_transition = true;
        return FWK_SUCCESS;
    }

    mpmm_domain_set_thresholds(domain_ctx);
    return FWK_SUCCESS;
}

static int mpmm_report(struct perf_plugins_perf_report *data)
{
    int status;
    uint32_t domain_idx;
    uint32_t core_idx;
    struct fwk_event resp_notif;
    struct mod_mpmm_domain_ctx *domain_ctx;
    struct mod_pd_power_state_pre_transition_notification_resp_params
        *pd_resp_params =
            (struct mod_pd_power_state_pre_transition_notification_resp_params
                 *)resp_notif.params;
    /*
     * Get the performance element id from the sub-element provided in the
     * function argument.
     */
    fwk_id_t perf_id = FWK_ID_ELEMENT(
        FWK_MODULE_IDX_DVFS, fwk_id_get_element_idx(data->dep_dom_id));

    for (domain_idx = 0; domain_idx < mpmm_ctx.mpmm_domain_count;
         domain_idx++) {
        if (fwk_id_is_equal(
                mpmm_ctx.domain_ctx[domain_idx].domain_config->perf_id,
                perf_id)) {
            break;
        }
    }

    if (domain_idx == mpmm_ctx.mpmm_domain_count) {
        return FWK_E_PARAM;
    }

    domain_ctx = &mpmm_ctx.domain_ctx[domain_idx];

    domain_ctx->current_perf_level = data->level;

    if (!domain_ctx->wait_for_perf_transition) {
        return FWK_SUCCESS;
    }

    domain_ctx->wait_for_perf_transition = false;

    mpmm_domain_set_thresholds(domain_ctx);

    /*
     * If a previous core wake-up sequence was delayed to re-evaluate the MPMM
     * thresholds and perf limits, then respond to the power domain notification
     * so the core can now be turned on.
     */
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        if (domain_ctx->core_ctx[core_idx].pd_blocked) {
            domain_ctx->core_ctx[core_idx].pd_blocked = false;

            status = fwk_get_delayed_response(
                domain_ctx->domain_id,
                domain_ctx->core_ctx[core_idx].cookie,
                &resp_notif);
            if (status != FWK_SUCCESS) {
                return status;
            }

            pd_resp_params->status = FWK_SUCCESS;
            status = fwk_put_event(&resp_notif);
            if (status != FWK_SUCCESS) {
                return status;
            }
        }
    }
    return FWK_SUCCESS;
}

static struct perf_plugins_api perf_plugins_api = {
    .update = mpmm_update,
    .report = mpmm_report,
};

/*
 * Framework handlers
 */
static int mpmm_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (element_count == 0) {
        return FWK_E_PARAM;
    }

    mpmm_ctx.mpmm_domain_count = element_count;
    mpmm_ctx.domain_ctx =
        fwk_mm_calloc(element_count, sizeof(struct mod_mpmm_domain_ctx));
    mpmm_ctx.amu_driver_api_id = *(fwk_id_t *)data;

    return FWK_SUCCESS;
}

static int mpmm_element_init(
    fwk_id_t domain_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_mpmm_domain_ctx *domain_ctx;
    struct mod_mpmm_core_ctx *core_ctx;
    struct mod_mpmm_core_config const *core_config;
    uint32_t core_idx;
    uint32_t num_thresholds;

    if ((sub_element_count == 0) ||
        (sub_element_count > MPMM_MAX_NUM_CORES_IN_DOMAIN)) {
        return FWK_E_PARAM;
    }

    domain_ctx = get_domain_ctx(domain_id);
    domain_ctx->domain_id = domain_id;
    domain_ctx->num_cores = sub_element_count;
    domain_ctx->wait_for_perf_transition = false;

    /* Initialize the configuration */
    domain_ctx->domain_config = data;
    fwk_assert(domain_ctx->domain_config->pct != NULL);

    if (domain_ctx->domain_config->num_threshold_counters >
        MPMM_MAX_THRESHOLD_COUNT) {
        return FWK_E_SUPPORT;
    }

    /* Initialize each core */
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        core_ctx = &domain_ctx->core_ctx[core_idx];
        core_ctx->core_id = fwk_id_build_sub_element_id(domain_id, core_idx);
        core_config = &domain_ctx->domain_config->core_config[core_idx];
        core_ctx->mpmm = (struct mpmm_reg *)core_config->mpmm_reg_base;
        core_ctx->base_aux_counter_id = core_config->base_aux_counter_id;

        mpmm_core_get_number_of_thresholds(core_ctx, &num_thresholds);

        if (num_thresholds !=
            domain_ctx->domain_config->num_threshold_counters) {
            return FWK_E_DEVICE;
        }

        /* Create counters storage */
        core_ctx->cached_counters = fwk_mm_calloc(
            domain_ctx->domain_config->num_threshold_counters,
            sizeof(*core_ctx->cached_counters));
        core_ctx->delta = fwk_mm_calloc(
            domain_ctx->domain_config->num_threshold_counters,
            sizeof(*core_ctx->delta));

        if (core_config->core_starts_online) {
            domain_ctx->num_cores_online++;
            core_ctx->online = true;
        }
    }

    return FWK_SUCCESS;
}

static int mpmm_start(fwk_id_t id)
{
    int status;
    uint32_t i;
    struct mod_mpmm_domain_ctx *domain_ctx;

    if (fwk_module_is_valid_module_id(id)) {
        return FWK_SUCCESS;
    }

    /* Subscribe to core power state transition */
    domain_ctx = get_domain_ctx(id);

    for (i = 0; i < domain_ctx->num_cores; i++) {
        status = fwk_notification_subscribe(
            mod_pd_notification_id_power_state_pre_transition,
            domain_ctx->domain_config->core_config[i].pd_id,
            domain_ctx->domain_id);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = fwk_notification_subscribe(
            mod_pd_notification_id_power_state_transition,
            domain_ctx->domain_config->core_config[i].pd_id,
            domain_ctx->domain_id);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

static int mpmm_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct mod_pd_power_state_pre_transition_notification_params
        *pre_state_params;
    struct mod_pd_power_state_transition_notification_params *post_state_params;
    struct mod_pd_power_state_pre_transition_notification_resp_params
        *pd_resp_params =
            (struct mod_pd_power_state_pre_transition_notification_resp_params
                 *)resp_event->params;
    struct mod_mpmm_domain_ctx *domain_ctx;
    uint32_t core_idx;
    uint32_t perf_limit;

    fwk_assert(fwk_module_is_valid_element_id(event->target_id));
    domain_ctx = get_domain_ctx(event->target_id);
    if (domain_ctx == NULL) {
        return FWK_E_PARAM;
    }

    /* Find the corresponding core */
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        if (fwk_id_is_equal(
                domain_ctx->domain_config->core_config[core_idx].pd_id,
                event->source_id)) {
            break;
        }
    }

    if (core_idx >= domain_ctx->num_cores) {
        return FWK_E_PARAM;
    }

    if (fwk_id_is_equal(
            event->id, mod_pd_notification_id_power_state_pre_transition)) {
        pre_state_params =
            (struct mod_pd_power_state_pre_transition_notification_params *)
                event->params;
        pd_resp_params->status = FWK_SUCCESS;
        if (pre_state_params->target_state == MOD_PD_STATE_ON) {
            /* The core is transitioning to online */
            domain_ctx->num_cores_online++;
            domain_ctx->core_ctx[core_idx].online = true;
            /*
             * After core transition to ON the threshold is set to zero as
             * defined by the hardware. The next line modifies the threshold
             * bitmap to include this core threshold.
             */
            domain_ctx->threshold_map = domain_ctx->threshold_map
                << MPMM_THRESHOLD_MAP_NUM_OF_BITS;
            perf_limit = mpmm_evaluate_perf_limit(domain_ctx);

            /* Set the new limits */
            struct plugin_limits_req plugin_limit_req = {
                .domain_id = domain_ctx->domain_config->perf_id,
                .max_limit = perf_limit,
            };
            mpmm_ctx.perf_plugins_handler_api->plugin_set_limits(
                &plugin_limit_req);

            domain_ctx->perf_limit = perf_limit;

            /*
             * If the perf limit requested will not trigger a dvfs change, there
             * is no need to block the power domain state transition.
             */
            if (perf_limit >= domain_ctx->current_perf_level) {
                return FWK_SUCCESS;
            }

            /* Block the power domain until the new level is applied */
            domain_ctx->core_ctx[core_idx].pd_blocked = true;
            domain_ctx->wait_for_perf_transition = true;
            resp_event->is_delayed_response = true;
            domain_ctx->core_ctx[core_idx].cookie = event->cookie;
        }

    } else if (fwk_id_is_equal(
                   event->id, mod_pd_notification_id_power_state_transition)) {
        post_state_params =
            (struct mod_pd_power_state_transition_notification_params *)
                event->params;
        if (post_state_params->state != MOD_PD_STATE_ON) {
            /* The core transitioned to offline */
            domain_ctx->num_cores_online--;
            domain_ctx->core_ctx[core_idx].online = false;
        }
    }

    return FWK_SUCCESS;
}

static int mpmm_bind(fwk_id_t id, unsigned int round)
{
    int status = FWK_E_DATA;
    /* Bind in the second round */
    if ((round == 0) || (!fwk_module_is_valid_module_id(id))) {
        return FWK_SUCCESS;
    }

    status = fwk_module_bind(
        FWK_ID_MODULE(mpmm_ctx.amu_driver_api_id.common.module_idx),
        mpmm_ctx.amu_driver_api_id,
        &mpmm_ctx.amu_driver_api);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF),
        FWK_ID_API(FWK_MODULE_IDX_SCMI_PERF, MOD_SCMI_PERF_PLUGINS_API),
        &mpmm_ctx.perf_plugins_handler_api);
}

static int mpmm_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    if (fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF))) {
        *api = &perf_plugins_api;
    } else {
        return FWK_E_ACCESS;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_mpmm = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = 1,
    .init = mpmm_init,
    .element_init = mpmm_element_init,
    .start = mpmm_start,
    .bind = mpmm_bind,
    .process_bind_request = mpmm_process_bind_request,
    .process_notification = mpmm_process_notification,
};

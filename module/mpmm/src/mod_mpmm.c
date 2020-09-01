/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_mpmm.h>
#include <mod_power_domain.h>
#include <mod_scmi_perf.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_thread.h>

struct mod_mpmm_core_ctx {
    /* Core Identifier */
    fwk_id_t core_id;

    /* The core is online */
    bool online;

    /* Current selected threshold */
    uint32_t threshold;

    /* Cached counters */
    uint32_t *cached_counters;

    /* Thresholds delta */
    uint32_t *delta;

    /* Used to block the PD when transitioning from OFF to ON */
    bool pd_blocked;

    /* Cookie to un-block the PD transition from OFF to ON */
    uint32_t cookie;
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

    /* Latest perf level value as reported by the plugin */
    uint32_t current_perf_level;

    /* Latest perf limit value required by mpmm */
    uint32_t perf_limit;

    /* Wait for the report to confirm perf transition completion */
    bool wait_for_perf_transition;

    /* Core context */
    struct mod_mpmm_core_ctx core_ctx[MPMM_MAX_NUM_CORES_IN_DOMAIN];

    /* Perf list requested by the AP */
    struct perf_plugins_perf_update perf_list;

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
} mpmm_ctx;

/*
 * MPMM Module Helper Functions
 */
static struct mod_mpmm_domain_ctx *get_domain_ctx(fwk_id_t domain_id)
{
    uint32_t idx = fwk_id_get_element_idx(domain_id);

    if (idx < mpmm_ctx.mpmm_domain_count)
        return &mpmm_ctx.domain_ctx[idx];
    else
        return NULL;
}

static int mpmm_core_counters_delta(
    struct mod_mpmm_domain_ctx *domain_ctx,
    struct mod_mpmm_core_ctx *core_ctx)
{
    uint32_t th_count = domain_ctx->domain_config->num_threshold_counters;
    uint32_t i, status;
    uint32_t counter_value;

    for (i = 0; i < th_count; i++) {
        status = mpmm_core_counter_read(
            domain_ctx->domain_id, core_ctx->core_id, i, &counter_value);
        if (status != FWK_SUCCESS)
            return status;

        /* Calculate the delta */
        if (counter_value < core_ctx->cached_counters[i]) {
            /* Counter wraparound case */
            core_ctx->delta[i] = UINT32_MAX - core_ctx->cached_counters[i];
            core_ctx->delta[i] += counter_value;
        } else
            core_ctx->delta[i] = counter_value - core_ctx->cached_counters[i];

        /* Store the last value */
        core_ctx->cached_counters[i] = counter_value;
    }

    return FWK_SUCCESS;
}

/* This function returns the selected threshold based on a pre-defined policy */
static uint32_t mpmm_core_threshold_policy(
    struct mod_mpmm_domain_ctx *domain_ctx,
    struct mod_mpmm_core_ctx *core_ctx)
{
    uint32_t thr_idx;

    if ((core_ctx->delta[0] == 0) && (domain_ctx->domain_config->btc == 0))
        return 0;

    for (thr_idx = 0;
         thr_idx < domain_ctx->domain_config->num_threshold_counters;
         thr_idx++) {
        if (core_ctx->delta[thr_idx] <= domain_ctx->domain_config->btc) {
            return thr_idx;
        }
    }
    return (domain_ctx->domain_config->num_threshold_counters - 1);
}

/* set the threshold for all cores */
static int mpmm_domain_set_thresholds(struct mod_mpmm_domain_ctx *ctx)
{
    uint32_t core_idx;
    int status;
    struct mod_mpmm_core_ctx *core_ctx;

    for (core_idx = 0; core_idx < ctx->num_cores; core_idx++) {
        core_ctx = &ctx->core_ctx[core_idx];
        if (core_ctx->online) {
            status = mpmm_core_set_threshold(
                ctx->domain_id, core_ctx->core_id, core_ctx->threshold);
            if (status != FWK_SUCCESS)
                return status;
        }
    }
    return FWK_SUCCESS;
}

static int mpmm_core_evaluate_threshold(
    struct mod_mpmm_domain_ctx *domain_ctx,
    struct mod_mpmm_core_ctx *core_ctx)
{
    uint32_t status;
    bool enabled;

    /* If counters are not enabled exit */
    status = mpmm_core_check_enabled(
        domain_ctx->domain_id, core_ctx->core_id, &enabled);
    if (status != FWK_SUCCESS)
        return status;
    if (!enabled)
        return FWK_SUCCESS;

    /* Read counters */
    status = mpmm_core_counters_delta(domain_ctx, core_ctx);
    if (status != FWK_SUCCESS)
        return status;

    /* Threshold selection policy */
    core_ctx->threshold = mpmm_core_threshold_policy(domain_ctx, core_ctx);

    return FWK_SUCCESS;
}

static uint32_t find_perf_limit_from_pct(
    struct mod_mpmm_threshold_perf *threshold_perf_table,
    uint32_t num_cells,
    uint32_t threshold_map)
{
    uint32_t j;

    for (j = num_cells; j > 0; j--) {
        if (threshold_map <= threshold_perf_table->threshold_bitmap)
            return threshold_perf_table->perf_limit;
        threshold_perf_table--;
    }
    return 0;
}

/* This function will update the threshold based on the bias policy */
static void mpmm_domain_performance_bias(struct mod_mpmm_domain_ctx *domain_ctx)
{
    uint32_t core_idx;
    uint32_t max_level, max_thr, thr_limit, count = 0;
    struct mod_mpmm_core_ctx *core_ctx;

    if (domain_ctx->num_cores_online == 0 || domain_ctx->num_cores == 1)
        return;

    /* Find max threshold selected and max perf level requested by the AP */
    max_thr = 0;
    max_level = 0;
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        core_ctx = &domain_ctx->core_ctx[core_idx];
        if (!core_ctx->online)
            continue;
        /* Is this the max threshold */
        if (core_ctx->threshold > max_thr)
            max_thr = core_ctx->threshold;
        /* Is this the max perf level */
        if (domain_ctx->perf_list.level[core_idx] > max_level)
            max_level = domain_ctx->perf_list.level[core_idx];
    }

    /* If more than one core at max threshold and max perf ignore the bias */
    count = 0;
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        core_ctx = &domain_ctx->core_ctx[core_idx];
        if (!core_ctx->online)
            continue;
        if ((core_ctx->threshold == max_thr) &&
            (domain_ctx->perf_list.level[core_idx] == max_level))
            count++;
        if (count > 1)
            return;
    }

    /* Bias loop */
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        core_ctx = &domain_ctx->core_ctx[core_idx];
        if (!core_ctx->online)
            continue;
        /*
         * If the core whose threshold is highest make highest performance
         * request, increase the threshold by one if available.
         */
        if ((core_ctx->threshold == max_thr) &&
            (domain_ctx->perf_list.level[core_idx] == max_level)) {
            thr_limit = domain_ctx->domain_config->num_threshold_counters - 1;
            if (core_ctx->threshold < (thr_limit - 1)) {
                core_ctx->threshold += 1;
                break;
            }
        }
    }
}

/* Convert the thresholds into a bitmap as described by the PCT */
static void mpmm_build_threshold_map(struct mod_mpmm_domain_ctx *ctx)
{
    uint32_t i, j, thr_tmp, thr_map = 0;
    uint32_t threshold_array[MPMM_MAX_NUM_CORES_IN_DOMAIN];

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
    struct mod_mpmm_threshold_perf *threshold_perf_cell;
    struct mod_mpmm_pct_table *pct_config;
    size_t pct_size;
    uint32_t num_perf_cells;
    int pct_idx;

    /* Parse PCT table from the bottom-up*/
    pct_config = ctx->domain_config->pct;
    pct_size = ctx->domain_config->pct_size;
    /* Start from the last index */
    for (pct_idx = (pct_size - 1); pct_idx >= 0; pct_idx--) {
        /* Find the entry based on the number of online cores */
        if (ctx->num_cores_online <= pct_config[pct_idx].cores_online) {
            num_perf_cells = pct_config[pct_idx].num_perf_limits;
            threshold_perf_cell =
                &(pct_config[pct_idx].threshold_perf[num_perf_cells - 1]);
            /* Find the performance limit */
            return find_perf_limit_from_pct(
                threshold_perf_cell, num_perf_cells, ctx->threshold_map);
        }
    }

    return 0;
}

/* Check CPU status and update performance limits accordingly */
static int mpmm_monitor_and_control(struct mod_mpmm_domain_ctx *domain_ctx)
{
    int status;
    uint32_t core_idx, perf_limit;
    struct mod_mpmm_core_ctx *core_ctx;

    if (!domain_ctx->num_cores_online)
        return FWK_SUCCESS;

    /* Core level algorithm */
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        core_ctx = &domain_ctx->core_ctx[core_idx];

        if (!core_ctx->online)
            continue;

        status = mpmm_core_evaluate_threshold(domain_ctx, core_ctx);
        if (status != FWK_SUCCESS)
            return status;
    }

    /* Domain level algorithm */
    mpmm_domain_performance_bias(domain_ctx);

    mpmm_build_threshold_map(domain_ctx);

    perf_limit = mpmm_evaluate_perf_limit(domain_ctx);

    if (perf_limit == 0)
        return FWK_E_PARAM;

    /* Cache the last value */
    domain_ctx->perf_limit = perf_limit;

    return FWK_SUCCESS;
}

/* Module APIs */

/*
 * update function should be called periodically to monitor the threshold
 * counters and update the performance limits.
 *
 */
static int mpmm_update(struct perf_plugins_perf_update *data)
{
    uint32_t core_idx;
    struct mod_mpmm_domain_ctx *domain_ctx;
    int status;

    domain_ctx = &mpmm_ctx.domain_ctx[fwk_id_get_element_idx(data->domain_id)];
    if (domain_ctx->wait_for_perf_transition)
        return FWK_SUCCESS;

    /* Store the latest perf requests from the AP */
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        domain_ctx->perf_list.level[core_idx] = data->level[core_idx];
    }

    status = mpmm_monitor_and_control(domain_ctx);
    if (status != FWK_SUCCESS)
        return status;

    /*
     * Update the new performance limits and set the flag to wait for the
     * transition to complete.
     */
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        data->adj_max_limit[core_idx] = domain_ctx->perf_limit;
        if (domain_ctx->perf_limit < domain_ctx->current_perf_level)
            domain_ctx->wait_for_perf_transition = true;
    }

    if (!domain_ctx->wait_for_perf_transition)
        return mpmm_domain_set_thresholds(domain_ctx);

    return FWK_SUCCESS;
}

static int mpmm_report(struct perf_plugins_perf_report *data)
{
    int status;
    uint32_t core_idx;
    struct fwk_event resp;
    struct mod_mpmm_domain_ctx *domain_ctx =
        &mpmm_ctx.domain_ctx[fwk_id_get_element_idx(data->dep_dom_id)];

    domain_ctx->current_perf_level = data->level;

    if (!domain_ctx->wait_for_perf_transition)
        return FWK_SUCCESS;
    domain_ctx->wait_for_perf_transition = false;

    status = mpmm_domain_set_thresholds(domain_ctx);
    if (status != FWK_SUCCESS)
        return status;

    /* Respond to the notification so the core power domain can be turned on */
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        if (domain_ctx->core_ctx[core_idx].pd_blocked) {
            domain_ctx->core_ctx[core_idx].pd_blocked = false;
            status = fwk_thread_get_delayed_response(
                data->dep_dom_id, domain_ctx->core_ctx[core_idx].cookie, &resp);
            if (status != FWK_SUCCESS)
                return status;

            status = fwk_thread_put_event(&resp);
            if (status != FWK_SUCCESS)
                return status;
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
    if (element_count == 0)
        return FWK_E_PARAM;

    mpmm_ctx.mpmm_domain_count = element_count;
    mpmm_ctx.domain_ctx =
        fwk_mm_calloc(element_count, sizeof(struct mod_mpmm_domain_ctx));

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

    if (sub_element_count == 0 ||
        sub_element_count > MPMM_MAX_NUM_CORES_IN_DOMAIN)
        return FWK_E_PARAM;

    domain_ctx = &mpmm_ctx.domain_ctx[fwk_id_get_element_idx(domain_id)];
    domain_ctx->domain_id = domain_id;
    domain_ctx->num_cores = sub_element_count;
    domain_ctx->wait_for_perf_transition = false;

    /* Initialize the configuration */
    domain_ctx->domain_config = data;
    fwk_assert(domain_ctx->domain_config->pct != NULL);

    if (domain_ctx->domain_config->num_threshold_counters >
        MPMM_MAX_THRESHOLD_COUNT)
        return FWK_E_SUPPORT;

    /* Initialize each core */
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        core_ctx = &domain_ctx->core_ctx[core_idx];
        core_ctx->core_id = fwk_id_build_sub_element_id(domain_id, core_idx);

        /* Create counters storage */
        core_ctx->cached_counters = fwk_mm_calloc(
            domain_ctx->domain_config->num_threshold_counters,
            sizeof(uint32_t));
        core_ctx->delta = fwk_mm_calloc(
            domain_ctx->domain_config->num_threshold_counters,
            sizeof(uint32_t));

        core_config = &domain_ctx->domain_config->core_config[core_idx];
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

    if (fwk_module_is_valid_module_id(id))
        return FWK_SUCCESS;

    /* Subscribe to core power state transition */
    domain_ctx = &mpmm_ctx.domain_ctx[fwk_id_get_element_idx(id)];

    for (i = 0; i < domain_ctx->num_cores; i++) {
        status = fwk_notification_subscribe(
            mod_pd_notification_id_power_state_pre_transition,
            domain_ctx->domain_config->core_config[i].pd_id,
            domain_ctx->domain_id);
        if (status != FWK_SUCCESS)
            return status;

        status = fwk_notification_subscribe(
            mod_pd_notification_id_power_state_transition,
            domain_ctx->domain_config->core_config[i].pd_id,
            domain_ctx->domain_id);
        if (status != FWK_SUCCESS)
            return status;
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
    struct mod_mpmm_domain_ctx *domain_ctx;
    uint32_t core_idx;
    uint32_t perf_limit;

    fwk_assert(fwk_module_is_valid_element_id(event->target_id));
    domain_ctx = get_domain_ctx(event->target_id);

    /* Find the corresponding core */
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        if (fwk_id_is_equal(
                domain_ctx->domain_config->core_config[core_idx].pd_id,
                event->source_id))
            break;
    }

    if (core_idx >= domain_ctx->num_cores)
        return FWK_E_PARAM;

    if (fwk_id_is_equal(
            event->id, mod_pd_notification_id_power_state_pre_transition)) {
        pre_state_params =
            (struct mod_pd_power_state_pre_transition_notification_params *)
                event->params;
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
            if (perf_limit == 0)
                return FWK_E_PARAM;

            /* Set the new limits */
            struct plugin_limits_req plugin_limit_req = {
                .domain_id = event->target_id,
                .max_limit = perf_limit,
            };
            mpmm_ctx.perf_plugins_handler_api->plugin_set_limits(
                &plugin_limit_req);

            /*
             * If the perf limit evaluated did not change, there is no need to
             * block the power domain state transition.
             */
            if (perf_limit >= domain_ctx->perf_limit) {
                domain_ctx->perf_limit = perf_limit;
                return FWK_SUCCESS;
            }

            domain_ctx->perf_limit = perf_limit;
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
    /* Bind in the second round */
    if ((round == 0) || (!fwk_module_is_valid_module_id(id)))
        return FWK_SUCCESS;

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
    if (fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF)))
        *api = &perf_plugins_api;
    else
        return FWK_E_ACCESS;

    return FWK_SUCCESS;
}

const struct fwk_module module_mpmm = {
    .name = "MPMM",
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = 1,
    .init = mpmm_init,
    .element_init = mpmm_element_init,
    .start = mpmm_start,
    .bind = mpmm_bind,
    .process_bind_request = mpmm_process_bind_request,
    .process_notification = mpmm_process_notification,
};

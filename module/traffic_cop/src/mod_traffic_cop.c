/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_power_domain.h>
#include <mod_scmi_perf.h>
#include <mod_traffic_cop.h>

#include <fwk_core.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <fwk_status.h>

struct mod_tcop_core_ctx {
    /* Core Identifier */
    fwk_id_t core_id;

    /* The core is online */
    bool online;

    /* Used to block the PD when transitioning from OFF to ON */
    bool pd_blocked;

    /* Cookie to un-block the PD transition from OFF to ON */
    uint32_t cookie;
};

struct mod_tcop_domain_ctx {
    /* Context Domain ID */
    fwk_id_t domain_id;

    /* Number of cores to monitor */
    uint32_t num_cores;

    /* Number of cores online */
    uint32_t num_cores_online;

    /* Latest perf level value as reported by the plugin handler */
    uint32_t current_perf_level;

    /* Latest perf limit value required by traffic cop */
    uint32_t perf_limit;

    /* Core context */
    struct mod_tcop_core_ctx *core_ctx;

    /* Domain configuration */
    const struct mod_tcop_domain_config *domain_config;
};

static struct mod_tcop_ctx {
    /* Number of domains */
    uint32_t tcop_domain_count;

    /* Domain context table */
    struct mod_tcop_domain_ctx *domain_ctx;

    /* Perf plugin API */
    struct perf_plugins_handler_api *perf_plugins_handler_api;
} tcop_ctx;

static struct mod_tcop_domain_ctx *get_domain_ctx(fwk_id_t domain_id)
{
    uint32_t idx = fwk_id_get_element_idx(domain_id);

    if (idx < tcop_ctx.tcop_domain_count) {
        return &tcop_ctx.domain_ctx[idx];
    } else {
        return NULL;
    }
}

static uint32_t tcop_evaluate_perf_limit(struct mod_tcop_domain_ctx *ctx)
{
    int pct_idx;
    size_t pct_size;
    struct mod_tcop_pct_table *pct_config;

    /* Parse PCT table from the bottom-up*/
    pct_config = ctx->domain_config->pct;
    pct_size = ctx->domain_config->pct_size;
    /* Start from last table index */
    for (pct_idx = (pct_size - 1); pct_idx >= 0; pct_idx--) {
        /* Search for the table entry that matches the number of cores online */
        if (ctx->num_cores_online <= pct_config[pct_idx].cores_online) {
            return pct_config[pct_idx].perf_limit;
        }
    }

    FWK_LOG_WARN(
        "[TRAFFIC_COP] No entry found in the PCT for %ld online cores",
        (long)ctx->num_cores_online);

    return 0;
}

/*
 * Update function will be called periodically. It needs to maintain the
 * performance limits.
 */
static int tcop_update(struct perf_plugins_perf_update *data)
{
    uint32_t domain_idx;
    struct mod_tcop_domain_ctx *domain_ctx;
    /*
     * Get the performance element id from the sub-element provided in the
     * function argument.
     */
    fwk_id_t perf_id = FWK_ID_ELEMENT(
        FWK_MODULE_IDX_DVFS, fwk_id_get_element_idx(data->domain_id));

    for (domain_idx = 0; domain_idx < tcop_ctx.tcop_domain_count;
         domain_idx++) {
        if (fwk_id_is_equal(
                tcop_ctx.domain_ctx[domain_idx].domain_config->perf_id,
                perf_id)) {
            break;
        }
    }

    if (domain_idx == tcop_ctx.tcop_domain_count) {
        return FWK_E_PARAM;
    }

    domain_ctx = &tcop_ctx.domain_ctx[domain_idx];

    /* Keep the last calculated performance limits. */
    data->adj_max_limit[0] = domain_ctx->perf_limit;

    return FWK_SUCCESS;
}

static int tcop_report(struct perf_plugins_perf_report *data)
{
    int status;
    uint32_t core_idx, domain_idx;
    struct fwk_event resp_notif;
    struct mod_tcop_domain_ctx *domain_ctx;
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

    for (domain_idx = 0; domain_idx < tcop_ctx.tcop_domain_count;
         domain_idx++) {
        if (fwk_id_is_equal(
                tcop_ctx.domain_ctx[domain_idx].domain_config->perf_id,
                perf_id)) {
            break;
        }
    }

    if (domain_idx == tcop_ctx.tcop_domain_count) {
        return FWK_E_PARAM;
    }

    domain_ctx = &tcop_ctx.domain_ctx[domain_idx];
    domain_ctx->current_perf_level = data->level;

    /*
     * If a previous core wake-up sequence was delayed to allow a period of time
     * for the perf transition to take place, then respond to the power domain
     * notification so the core can now be turned on.
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
    .update = tcop_update,
    .report = tcop_report,
};

/*
 * Framework handlers
 */
static int tcop_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (element_count == 0) {
        return FWK_E_PARAM;
    }

    tcop_ctx.tcop_domain_count = element_count;
    tcop_ctx.domain_ctx =
        fwk_mm_calloc(element_count, sizeof(struct mod_tcop_domain_ctx));

    return FWK_SUCCESS;
}

static int tcop_element_init(
    fwk_id_t domain_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_tcop_domain_ctx *domain_ctx;
    struct mod_tcop_core_ctx *core_ctx;
    struct mod_tcop_core_config const *core_config;
    uint32_t core_idx;

    if (sub_element_count == 0) {
        return FWK_E_PARAM;
    }

    domain_ctx = get_domain_ctx(domain_id);
    domain_ctx->domain_id = domain_id;
    domain_ctx->num_cores = sub_element_count;

    /* Initialize the configuration */
    domain_ctx->domain_config = data;
    fwk_assert(domain_ctx->domain_config->pct != NULL);

    domain_ctx->core_ctx =
        fwk_mm_calloc(sub_element_count, sizeof(struct mod_tcop_core_ctx));

    /* Initialize each core */
    for (core_idx = 0; core_idx < domain_ctx->num_cores; core_idx++) {
        core_ctx = &domain_ctx->core_ctx[core_idx];
        core_config = &domain_ctx->domain_config->core_config[core_idx];

        if (core_config->core_starts_online) {
            domain_ctx->num_cores_online++;
            core_ctx->online = true;
        }
    }

    return FWK_SUCCESS;
}

static int tcop_bind(fwk_id_t id, unsigned int round)
{
    /* Bind in the second round */
    if ((round == 0) || (!fwk_module_is_valid_module_id(id))) {
        return FWK_SUCCESS;
    }

    return fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_PERF),
        FWK_ID_API(FWK_MODULE_IDX_SCMI_PERF, MOD_SCMI_PERF_PLUGINS_API),
        &tcop_ctx.perf_plugins_handler_api);
}

static int tcop_process_bind_request(
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

static int tcop_start(fwk_id_t id)
{
    int status;
    uint32_t i;
    struct mod_tcop_domain_ctx *domain_ctx;

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

    /*
     * Evaluate limits right now to initialise the limits accordingly with the
     * known initial power status of cores.
     */
    domain_ctx->perf_limit = tcop_evaluate_perf_limit(domain_ctx);

    return FWK_SUCCESS;
}

static int tcop_process_notification(
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
    struct mod_tcop_domain_ctx *domain_ctx;
    uint32_t core_idx;

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
            domain_ctx->perf_limit = tcop_evaluate_perf_limit(domain_ctx);

            /* Set the new limits */
            struct plugin_limits_req plugin_limit_req = {
                .domain_id = domain_ctx->domain_config->perf_id,
                .max_limit = domain_ctx->perf_limit,
            };

            tcop_ctx.perf_plugins_handler_api->plugin_set_limits(
                &plugin_limit_req);

            /*
             * If the perf limit requested will not trigger a dvfs change, there
             * is no need to block the power domain state transition.
             */
            if (domain_ctx->perf_limit >= domain_ctx->current_perf_level) {
                return FWK_SUCCESS;
            }

            /* Block the power domain until the new level is applied */
            domain_ctx->core_ctx[core_idx].pd_blocked = true;
            domain_ctx->core_ctx[core_idx].cookie = event->cookie;
            resp_event->is_delayed_response = true;
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
            domain_ctx->perf_limit = tcop_evaluate_perf_limit(domain_ctx);
            if (domain_ctx->perf_limit == 0) {
                return FWK_E_PARAM;
            }
        }
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_traffic_cop = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = 1,
    .init = tcop_init,
    .element_init = tcop_element_init,
    .start = tcop_start,
    .bind = tcop_bind,
    .process_bind_request = tcop_process_bind_request,
    .process_notification = tcop_process_notification,
};

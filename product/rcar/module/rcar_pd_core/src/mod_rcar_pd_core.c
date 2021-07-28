/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mmio.h>
#include <rcar_mmap.h>
#include <rcar_pd_core.h>

#include <mod_rcar_pd_core.h>
#include <mod_system_power.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>

#include <arch_system.h>

#include <stdbool.h>

static struct rcar_pd_sysc_ctx rcar_pd_sysc_ctx;

/*
 * Functions not specific to any type of power domain
 */

static int rcar_pd_set_state(fwk_id_t pd_id, unsigned int state)
{
    struct rcar_pd_sysc_pd_ctx *pd_ctx;
    unsigned int core;

    pd_ctx = rcar_pd_sysc_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);
    core = fwk_id_get_element_idx(pd_id);

    switch (state) {
    case MOD_PD_STATE_ON:
        rcar_pwrc_cpuon(core);
        pd_ctx->current_state = state;
        pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_ON);
        break;

    case MOD_PD_STATE_OFF:
        rcar_pwrc_cpuoff(core);
        pd_ctx->current_state = state;
        pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_OFF);
        break;

    default:
        FWK_LOG_ERR("[PD] Requested power state (%i) is not supported.", state);
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int rcar_pd_get_state(fwk_id_t pd_id, unsigned int *state)
{
    struct rcar_pd_sysc_pd_ctx *pd_ctx;

    pd_ctx = rcar_pd_sysc_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);

    *state = pd_ctx->current_state;
    return FWK_SUCCESS;
}

static int rcar_pd_reset(fwk_id_t pd_id)
{
    return FWK_SUCCESS;
}

static const struct mod_pd_driver_api pd_driver = {
    .set_state = rcar_pd_set_state,
    .get_state = rcar_pd_get_state,
    .reset = rcar_pd_reset,
};

/*
 * Functions specific to core power domains
 */
static int rcar_core_pd_init(struct rcar_pd_sysc_pd_ctx *pd_ctx)
{
    return FWK_SUCCESS;
}

static int rcar_core_pd_set_state(fwk_id_t core_pd_id, unsigned int state)
{
    struct rcar_pd_sysc_pd_ctx *pd_ctx;
    unsigned int core;

    pd_ctx = rcar_pd_sysc_ctx.pd_ctx_table + fwk_id_get_element_idx(core_pd_id);
    core = fwk_id_get_element_idx(core_pd_id);

    switch (state) {
    case MOD_PD_STATE_OFF:
        rcar_pwrc_cpuoff(core);
        pd_ctx->current_state = state;
        pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_OFF);
        break;

    case MOD_PD_STATE_ON:
        rcar_pwrc_cpuon(core);
        pd_ctx->current_state = state;
        pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_ON);
        break;

    case MOD_PD_STATE_SLEEP:
        pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_ON);
        break;
    default:
        FWK_LOG_ERR("[PD] Requested power state (%i) is not supported.", state);
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int rcar_core_pd_reset(fwk_id_t core_pd_id)
{
    return FWK_SUCCESS;
}

static int rcar_core_pd_prepare_for_system_suspend(fwk_id_t core_pd_id)
{
    mmio_write_32(RCAR_CPGWPR, ~CPU_PWR_OFF);
    mmio_write_32(RCAR_CA57CPU0CR, CPU_PWR_OFF);

    mmio_write_32(RCAR_CA57CPUCMCR, MODE_L2_DOWN);
    _shutdown_request = R_SUSPEND;

    return FWK_SUCCESS;
}

static const struct mod_pd_driver_api core_pd_driver = {
    .set_state = rcar_core_pd_set_state,
    .get_state = rcar_pd_get_state,
    .reset = rcar_core_pd_reset,
    .prepare_core_for_system_suspend = rcar_core_pd_prepare_for_system_suspend
};

/*
 * Functions specific to cluster power domains
 */

static int rcar_cluster_pd_init(struct rcar_pd_sysc_pd_ctx *pd_ctx)
{
    return FWK_SUCCESS;
}

static int rcar_cluster_pd_set_state(fwk_id_t cluster_pd_id, unsigned int state)
{
    return FWK_SUCCESS;
}

static const struct mod_pd_driver_api cluster_pd_driver = {
    .set_state = rcar_cluster_pd_set_state,
    .get_state = rcar_pd_get_state,
    .reset = rcar_pd_reset,
};

/*
 * Framework handlers
 */

static int rcar_mod_init(
    fwk_id_t module_id,
    unsigned int pd_count,
    const void *unused)
{
    rcar_pd_sysc_ctx.pd_ctx_table =
        fwk_mm_calloc(pd_count, sizeof(struct rcar_pd_sysc_pd_ctx));
    if (rcar_pd_sysc_ctx.pd_ctx_table == NULL)
        return FWK_E_NOMEM;

    rcar_pd_sysc_ctx.pd_ctx_table_size = pd_count;

    return FWK_SUCCESS;
}

static int rcar_pd_init(fwk_id_t pd_id, unsigned int unused, const void *data)
{
    const struct mod_rcar_pd_core_pd_config *config = data;
    struct rcar_pd_sysc_pd_ctx *pd_ctx;

    if (config->pd_type >= RCAR_PD_TYPE_COUNT)
        return FWK_E_DATA;

    pd_ctx = rcar_pd_sysc_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);
    pd_ctx->config = config;
    pd_ctx->bound_id = FWK_ID_NONE;

    if (config->pd_type == RCAR_PD_TYPE_CLUSTER) {
        pd_ctx->data =
            fwk_mm_calloc(1, sizeof(struct rcar_pd_sysc_cluster_pd_ctx));
        if (pd_ctx->data == NULL)
            return FWK_E_NOMEM;
    }

    if (config->always_on) {
        switch (config->pd_type) {
        case RCAR_PD_TYPE_DEVICE:
            /* Fall through */
        case RCAR_PD_TYPE_DEVICE_DEBUG:
            /* Fall through */
        case RCAR_PD_TYPE_SYSTEM:
        case RCAR_PD_TYPE_CORE:
        case RCAR_PD_TYPE_CLUSTER:
            pd_ctx->current_state = MOD_PD_STATE_ON;
            break;

        default:
            assert(false);
            return FWK_E_SUPPORT;
        }
    }

    return FWK_SUCCESS;
}

static int rcar_post_init(fwk_id_t module_id)
{
    return FWK_SUCCESS;
}

static int rcar_core_bind(fwk_id_t id, unsigned int round)
{
    int status = FWK_SUCCESS;
    struct rcar_pd_sysc_pd_ctx *pd_ctx;

    /* Nothing to do during the first round of calls where the power module
       will bind to the power domains of this module. */
    if ((round == 0) || fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

#if 0
    /* In the case of the module, bind to the log component */
    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_LOG),
                                 FWK_ID_API(FWK_MODULE_IDX_LOG, 0),
                                 &rcar_pd_sysc_ctx.log_api);
        return status;
    }
#endif

    pd_ctx = rcar_pd_sysc_ctx.pd_ctx_table + fwk_id_get_element_idx(id);

    if (!fwk_id_is_equal(pd_ctx->config->observer_id, FWK_ID_NONE)) {
        if (pd_ctx->config->pd_type != RCAR_PD_TYPE_CLUSTER) {
            /* State observation only supported for clusters */
            assert(false);
            return FWK_E_SUPPORT;
        }

        status = fwk_module_bind(
            pd_ctx->config->observer_id,
            pd_ctx->config->observer_api,
            &pd_ctx->observer_api);
        if (status != FWK_SUCCESS)
            return status;
    }

    if (fwk_id_is_equal(pd_ctx->bound_id, FWK_ID_NONE))
        return FWK_SUCCESS;

    switch (fwk_id_get_module_idx(pd_ctx->bound_id)) {
    case FWK_MODULE_IDX_POWER_DOMAIN:
        return fwk_module_bind(
            pd_ctx->bound_id,
            mod_pd_api_id_driver_input,
            &pd_ctx->pd_driver_input_api);
        break;

    case FWK_MODULE_IDX_SYSTEM_POWER:
        return fwk_module_bind(
            pd_ctx->bound_id,
            mod_system_power_api_id_pd_driver_input,
            &pd_ctx->pd_driver_input_api);
        break;

    default:
        assert(false);
        return FWK_E_SUPPORT;
    }
}

static int rcar_core_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    struct rcar_pd_sysc_pd_ctx *pd_ctx;
    unsigned int api_idx;
    bool is_power_domain_module = false;
    bool is_system_power_module = false;

    api_idx = fwk_id_get_api_idx(api_id);

    if (api_idx != MOD_RCAR_PD_SYSC_API_IDX_POWER_DOMAIN_DRIVER)
        return FWK_E_SUPPORT;

    if (!fwk_module_is_valid_element_id(target_id))
        return FWK_E_PARAM;

    pd_ctx = rcar_pd_sysc_ctx.pd_ctx_table + fwk_id_get_element_idx(target_id);

    /* Allow multiple binding only for device power domain for now */
    if ((pd_ctx->config->pd_type != RCAR_PD_TYPE_DEVICE) &&
        (!fwk_id_is_equal(pd_ctx->bound_id, FWK_ID_NONE))) {
        assert(false);
        return FWK_E_ACCESS;
    }

    is_power_domain_module =
        (fwk_id_get_module_idx(source_id) == FWK_MODULE_IDX_POWER_DOMAIN);
    is_system_power_module =
        (fwk_id_get_module_idx(source_id) == FWK_MODULE_IDX_SYSTEM_POWER);

    switch (pd_ctx->config->pd_type) {
    case RCAR_PD_TYPE_CORE:
        if (is_power_domain_module) {
            *api = &core_pd_driver;
            pd_ctx->bound_id = source_id;
            return FWK_SUCCESS;
        }
        break;

    case RCAR_PD_TYPE_CLUSTER:
        if (is_power_domain_module) {
            *api = &cluster_pd_driver;
            pd_ctx->bound_id = source_id;
            return FWK_SUCCESS;
        }
        break;

    case RCAR_PD_TYPE_SYSTEM:
        if (is_power_domain_module || is_system_power_module) {
            *api = &pd_driver;
            pd_ctx->bound_id = source_id;
            return FWK_SUCCESS;
        }
        break;

    default:
        if (is_power_domain_module)
            pd_ctx->bound_id = source_id;
        *api = &pd_driver;
        return FWK_SUCCESS;
    }

    pd_ctx->bound_id = FWK_ID_NONE;
    return FWK_E_ACCESS;
}

static int rcar_core_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    const struct mod_rcar_pd_core_config *module_config;
    struct rcar_pd_sysc_pd_ctx *pd_ctx;
    struct mod_pd_power_state_transition_notification_params *params;

    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));
    module_config =
        fwk_module_get_data(fwk_id_build_module_id(event->target_id));
    assert(fwk_id_is_equal(event->id, module_config->pd_notification_id));
    (void)module_config;

    params = (struct mod_pd_power_state_transition_notification_params *)
                 event->params;

    if (params->state != MOD_PD_STATE_ON)
        return FWK_SUCCESS;

    pd_ctx = rcar_pd_sysc_ctx.pd_ctx_table +
        fwk_id_get_element_idx(event->target_id);

    switch (pd_ctx->config->pd_type) {
    case RCAR_PD_TYPE_CORE:
        return rcar_core_pd_init(pd_ctx);

    case RCAR_PD_TYPE_CLUSTER:
        return rcar_cluster_pd_init(pd_ctx);

    default:
        return FWK_SUCCESS;
    }
}

const struct fwk_module module_rcar_pd_core = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_RCAR_PD_SYSC_API_IDX_COUNT,
    .init = rcar_mod_init,
    .element_init = rcar_pd_init,
    .post_init = rcar_post_init,
    .bind = rcar_core_bind,
    .process_bind_request = rcar_core_process_bind_request,
    .process_notification = rcar_core_process_notification,
};

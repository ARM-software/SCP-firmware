/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <rcar_pd_sysc.h>

#include <mod_rcar_pd_sysc.h>
#include <mod_rcar_power_domain.h>
#include <mod_system_power.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*
 * Internal variables
 */
static struct rcar_sysc_ctx rcar_sysc_ctx;

/*
 * Power domain driver interface
 */
static int pd_set_state(fwk_id_t pd_id, unsigned int state)
{
    int status = FWK_SUCCESS;
    struct rcar_sysc_pd_ctx *pd_ctx;

    pd_ctx = rcar_sysc_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);

    switch (state) {
    case MOD_PD_STATE_ON:
        status = rcar_sysc_power(pd_ctx, true);
        pd_ctx->current_state = state;
        break;

    case MOD_PD_STATE_OFF:
        status = rcar_sysc_power(pd_ctx, false);
        pd_ctx->current_state = state;
        break;

    default:
        FWK_LOG_ERR("[PD] Requested power state (%i) is not supported.", state);
        return FWK_E_PARAM;
    }

    return status;
}

static int pd_get_state(fwk_id_t pd_id, unsigned int *state)
{
    struct rcar_sysc_pd_ctx *pd_ctx;

    pd_ctx = rcar_sysc_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);

    *state = pd_ctx->current_state;
    return FWK_SUCCESS;
}

static int pd_reset(fwk_id_t pd_id)
{
    return FWK_SUCCESS;
}

static const struct mod_pd_driver_api pd_driver = {
    .set_state = pd_set_state,
    .get_state = pd_get_state,
    .reset = pd_reset,
};

/*
 * Framework handlers
 */

static int rcar_sysc_mod_init(
    fwk_id_t module_id,
    unsigned int pd_count,
    const void *unused)
{
    rcar_sysc_ctx.pd_ctx_table =
        fwk_mm_calloc(pd_count, sizeof(struct rcar_sysc_pd_ctx));
    if (rcar_sysc_ctx.pd_ctx_table == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int rcar_sysc_pd_init(
    fwk_id_t pd_id,
    unsigned int unused,
    const void *data)
{
    const struct mod_rcar_pd_sysc_config *config = data;
    struct rcar_sysc_pd_ctx *pd_ctx;

    if (config->pd_type >= MOD_PD_TYPE_COUNT)
        return FWK_E_DATA;

    pd_ctx = rcar_sysc_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);
    pd_ctx->config = config;
    pd_ctx->bound_id = FWK_ID_NONE;

    switch (config->pd_type) {
    case MOD_PD_TYPE_DEVICE:
    case MOD_PD_TYPE_DEVICE_DEBUG:
    case MOD_PD_TYPE_SYSTEM:
        return FWK_SUCCESS;
    default:
        return FWK_E_SUPPORT;
    }
}

static int rcar_sysc_bind(fwk_id_t id, unsigned int round)
{
    struct rcar_sysc_pd_ctx *pd_ctx;

    /* Nothing to do during the first round of calls where the power module
       will bind to the power domains of this module. */
    if (round == 0)
        return FWK_SUCCESS;

#if 0
    /* In the case of the module, bind to the log component */
    if (fwk_module_is_valid_module_id(id)) {
        return fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_LOG),
                               FWK_ID_API(FWK_MODULE_IDX_LOG, 0),
                               &rcar_sysc_ctx.log_api);
    }
#endif

    pd_ctx = rcar_sysc_ctx.pd_ctx_table + fwk_id_get_element_idx(id);

    if (fwk_id_is_equal(pd_ctx->bound_id, FWK_ID_NONE))
        return FWK_SUCCESS;

    switch (fwk_id_get_module_idx(pd_ctx->bound_id)) {
    case FWK_MODULE_IDX_RCAR_POWER_DOMAIN:
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

static int rcar_sysc_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t not_used,
    const void **api)
{
    struct rcar_sysc_pd_ctx *pd_ctx;

    pd_ctx = rcar_sysc_ctx.pd_ctx_table + fwk_id_get_element_idx(target_id);

    switch (pd_ctx->config->pd_type) {
    case MOD_PD_TYPE_SYSTEM:
        if (!fwk_id_is_equal(pd_ctx->bound_id, FWK_ID_NONE)) {
            assert(false);
            return FWK_E_ACCESS;
        }
        /* Fallthrough */

    case MOD_PD_TYPE_DEVICE:
    case MOD_PD_TYPE_DEVICE_DEBUG:
        if (fwk_id_get_module_idx(source_id) ==
            FWK_MODULE_IDX_RCAR_POWER_DOMAIN) {
            pd_ctx->bound_id = source_id;
            *api = &pd_driver;
            break;
        }
        if (fwk_id_get_module_idx(source_id) == FWK_MODULE_IDX_SYSTEM_POWER) {
            *api = &pd_driver;
            break;
        }
        assert(false);
        return FWK_E_ACCESS;

    default:
        (void)pd_driver;
        return FWK_E_SUPPORT;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_rcar_pd_sysc = {
    .name = "RCAR_PD_SYSC",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = rcar_sysc_mod_init,
    .element_init = rcar_sysc_pd_init,
    .bind = rcar_sysc_bind,
    .process_bind_request = rcar_sysc_process_bind_request,
};

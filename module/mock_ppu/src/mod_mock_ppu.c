/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <mod_power_domain.h>
#include <mod_mock_ppu.h>
#ifdef BUILD_HAS_MOD_SYSTEM_POWER
#include <mod_system_power.h>
#endif

/* Power domain context */
struct mock_ppu_pd_ctx {
    /* Power domain configuration data */
    const struct mod_mock_ppu_pd_config *config;

    /* PPU registers */
    uint32_t *ppu;

    /* Identifier of the entity bound to the power domain driver API */
    fwk_id_t bound_id;

    /* Power module driver input API */
    struct mod_pd_driver_input_api *pd_driver_input_api;
};

/* Module context */
struct mock_ppu_ctx {
    /* Table of the power domain contexts */
    struct mock_ppu_pd_ctx *pd_ctx_table;
};

/*
 * Internal variables
 */

static struct mock_ppu_ctx mock_ppu_ctx;

/*
 * Power domain driver interface
 */
static int pd_set_state(fwk_id_t pd_id, unsigned int state)
{
    int status;
    struct mock_ppu_pd_ctx *pd_ctx;

    pd_ctx = mock_ppu_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);

    fwk_assert(pd_ctx->pd_driver_input_api != NULL);

    switch (state) {
    case MOD_PD_STATE_ON:
        *pd_ctx->ppu = MOD_PD_STATE_ON;
        status = pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_ON);
        assert(status == FWK_SUCCESS);
        break;

    case MOD_PD_STATE_OFF:
        *pd_ctx->ppu = MOD_PD_STATE_OFF;
        status = pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_OFF);
        assert(status == FWK_SUCCESS);
        break;

    default:
        FWK_LOG_ERR("[PD] Requested power state (%i) is not supported.", state);
        return FWK_E_PARAM;
    }

    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

static int pd_get_state(fwk_id_t pd_id, unsigned int *state)
{
    struct mock_ppu_pd_ctx *pd_ctx;

    pd_ctx = mock_ppu_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);

    *state = *pd_ctx->ppu;

    return FWK_SUCCESS;
}

static int pd_reset(fwk_id_t pd_id)
{
    struct mock_ppu_pd_ctx *pd_ctx;

    pd_ctx = mock_ppu_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);

    *pd_ctx->ppu = MOD_PD_STATE_ON;

    return FWK_SUCCESS;
}

static int ppu_v0_prepare_core_for_system_suspend(fwk_id_t core_pd_id)
{
    return FWK_SUCCESS;
}

static const struct mod_pd_driver_api pd_driver = {
    .set_state = pd_set_state,
    .get_state = pd_get_state,
    .reset = pd_reset,
    .prepare_core_for_system_suspend = ppu_v0_prepare_core_for_system_suspend,
};

/*
 * Framework handlers
 */

static int mock_ppu_mod_init(fwk_id_t module_id, unsigned int pd_count,
                           const void *unused)
{
    mock_ppu_ctx.pd_ctx_table = fwk_mm_calloc(pd_count,
                                            sizeof(struct mock_ppu_pd_ctx));
    if (mock_ppu_ctx.pd_ctx_table == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int mock_ppu_pd_init(fwk_id_t pd_id, unsigned int unused, const void *data)
{
    const struct mod_mock_ppu_pd_config *config = data;
    struct mock_ppu_pd_ctx *pd_ctx;

    if (config->pd_type >= MOD_PD_TYPE_COUNT)
        return FWK_E_DATA;

    pd_ctx = mock_ppu_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);
    pd_ctx->config = config;
    pd_ctx->ppu = (uint32_t *)(config->ppu.reg_base);
    pd_ctx->bound_id = FWK_ID_NONE;

    switch (config->pd_type) {
    case MOD_PD_TYPE_DEVICE:
    case MOD_PD_TYPE_DEVICE_DEBUG:
    case MOD_PD_TYPE_SYSTEM:
        if (config->default_power_on)
            *pd_ctx->ppu = MOD_PD_STATE_ON;

        return FWK_SUCCESS;

    default:
        return FWK_E_SUPPORT;
    }
}

static int mock_ppu_bind(fwk_id_t id, unsigned int round)
{
    struct mock_ppu_pd_ctx *pd_ctx;

    /* Nothing to do during the first round of calls where the power module
       will bind to the power domains of this module. */
    if (round == 0)
        return FWK_SUCCESS;

    /* In the case of the module, nothing to do */
    if (fwk_module_is_valid_module_id(id)) {
        return FWK_SUCCESS;
    }

    pd_ctx = mock_ppu_ctx.pd_ctx_table + fwk_id_get_element_idx(id);

    if (fwk_id_is_equal(pd_ctx->bound_id, FWK_ID_NONE))
        return FWK_SUCCESS;

    switch (fwk_id_get_module_idx(pd_ctx->bound_id)) {
    #ifdef BUILD_HAS_MOD_POWER_DOMAIN
    case FWK_MODULE_IDX_POWER_DOMAIN:
        return fwk_module_bind(pd_ctx->bound_id,
                               mod_pd_api_id_driver_input,
                               &pd_ctx->pd_driver_input_api);
        break;
    #endif

    #ifdef BUILD_HAS_MOD_SYSTEM_POWER
    case FWK_MODULE_IDX_SYSTEM_POWER:
        return fwk_module_bind(pd_ctx->bound_id,
                               mod_system_power_api_id_pd_driver_input,
                               &pd_ctx->pd_driver_input_api);
        break;
    #endif

    default:
        assert(false);
        return FWK_E_SUPPORT;
    }
}

static int mock_ppu_process_bind_request(fwk_id_t source_id,
                                       fwk_id_t target_id, fwk_id_t not_used,
                                       const void **api)
{
    struct mock_ppu_pd_ctx *pd_ctx;

    pd_ctx = mock_ppu_ctx.pd_ctx_table + fwk_id_get_element_idx(target_id);

    switch (pd_ctx->config->pd_type) {
    case MOD_PD_TYPE_SYSTEM:
        if (!fwk_id_is_equal(pd_ctx->bound_id, FWK_ID_NONE)) {
            assert(false);
            return FWK_E_ACCESS;
        }
    /* Fallthrough */

    case MOD_PD_TYPE_DEVICE:
    case MOD_PD_TYPE_DEVICE_DEBUG:
        #ifdef BUILD_HAS_MOD_POWER_DOMAIN
        if (fwk_id_get_module_idx(source_id) == FWK_MODULE_IDX_POWER_DOMAIN) {
            pd_ctx->bound_id = source_id;
            *api = &pd_driver;
            break;
        }
        #endif
        #ifdef BUILD_HAS_MOD_SYSTEM_POWER
        if (fwk_id_get_module_idx(source_id) == FWK_MODULE_IDX_SYSTEM_POWER) {
            pd_ctx->bound_id = source_id;
            *api = &pd_driver;
            break;
        }
        #endif
        assert(false);
        return FWK_E_ACCESS;

    default:
        (void)pd_driver;
        return FWK_E_SUPPORT;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_mock_ppu = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = mock_ppu_mod_init,
    .element_init = mock_ppu_pd_init,
    .bind = mock_ppu_bind,
    .process_bind_request = mock_ppu_process_bind_request,
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power State Management PPU v0 driver.
 */

#include <mod_power_domain.h>
#include <mod_ppu_v0.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>
#if BUILD_HAS_MOD_SYSTEM_POWER
#    include <mod_system_power.h>
#endif
#include <ppu_v0.h>

#include <fwk_status.h>

/* Power domain context */
struct ppu_v0_pd_ctx {
    /* Power domain configuration data */
    const struct mod_ppu_v0_pd_config *config;

    /* PPU registers */
    struct ppu_v0_reg *ppu;

    /* Identifier of the entity bound to the power domain driver API */
    fwk_id_t bound_id;

    /* Power module driver input API */
    struct mod_pd_driver_input_api *pd_driver_input_api;
};

/* Module context */
struct ppu_v0_ctx {
    /* Table of the power domain contexts */
    struct ppu_v0_pd_ctx *pd_ctx_table;
};

/*
 * Internal variables
 */

static struct ppu_v0_ctx ppu_v0_ctx;

#define MODE_UNSUPPORTED ~0U
static const uint8_t ppu_mode_to_power_state[] = {
    [PPU_V0_MODE_ON] = (uint8_t)MOD_PD_STATE_ON,
    [PPU_V0_MODE_FUNC_RET] = (uint8_t)MOD_PD_STATE_ON,
    [PPU_V0_MODE_MEM_OFF] = (uint8_t)MODE_UNSUPPORTED,
    [PPU_V0_MODE_FULL_RET] = (uint8_t)MOD_PD_STATE_ON,
    [PPU_V0_MODE_LOGIC_RET] = (uint8_t)MODE_UNSUPPORTED,
    [PPU_V0_MODE_MEM_RET] = (uint8_t)MOD_PD_STATE_ON,
    [PPU_V0_MODE_OFF] = (uint8_t)MOD_PD_STATE_OFF,
    [PPU_V0_MODE_WARM_RESET] = (uint8_t)MODE_UNSUPPORTED,
};

/*
 * Power domain driver interface
 */

/* Driver functions not specific to any type of power domain. */
static int get_state(struct ppu_v0_reg *ppu, unsigned int *state)
{
    enum ppu_v0_mode ppu_mode;

    /* Ensure ppu_mode_to_power_state has an entry for each PPU state */
    static_assert(
        FWK_ARRAY_SIZE(ppu_mode_to_power_state) == PPU_V0_MODE_COUNT,
        "[MOD_PPU_V0] ppu_mode_to_power_state size error");

    ppu_v0_get_power_mode(ppu, &ppu_mode);
    assert(ppu_mode < PPU_V0_MODE_COUNT);

    *state = ppu_mode_to_power_state[ppu_mode];
    if (*state == MODE_UNSUPPORTED) {
        FWK_LOG_ERR("[PPUV0] Unexpected PPU mode (%i).", ppu_mode);
        return FWK_E_DEVICE;
    }

    FWK_LOG_INFO("[PPUV0] get state reg=0x%p (0x%x)", (void *)ppu, *state);

    return FWK_SUCCESS;
}

static int pd_init(struct ppu_v0_pd_ctx *pd_ctx)
{
    ppu_v0_init(pd_ctx->ppu);

    return FWK_SUCCESS;
}

static int pd_set_state(fwk_id_t pd_id, unsigned int state)
{
    int status;
    struct ppu_v0_pd_ctx *pd_ctx;

    pd_ctx = ppu_v0_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);

    FWK_LOG_INFO(
        "[PPUV0] set_state start. reg=(0x%p) state=(0x%x)",
        (void *)pd_ctx->ppu,
        state);

    switch (state) {
    case MOD_PD_STATE_ON:
        ppu_v0_set_power_mode(pd_ctx->ppu, PPU_V0_MODE_ON);
        status = pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_ON);

        FWK_LOG_INFO(
            "[PPUV0] set_state end. reg=(0x%p) state=(0x%x)",
            (void *)pd_ctx->ppu,
            state);

        assert(status == FWK_SUCCESS);
        break;

    case MOD_PD_STATE_OFF:
        if (pd_ctx->config->pd_type == MOD_PD_TYPE_SYSTEM) {
            FWK_LOG_INFO(
                "[PPUV0] SYNQUACER SYSTEM module will shutdown the system");
            break;
        }

        /*
         * As it is not guaranteed that the PACTIVE signals of the core are low
         * as the core may not be in WFI for example, deactivate the check of
         * the PACTIVE signals by the PPU logic and the handshake with the core
         * P-channel before to ask for the core to be powered off.
         */
        pd_ctx->ppu->POWER_CFG &= ~PPU_PCR_DEV_ACTIVE_EN;
        pd_ctx->ppu->POWER_CFG &= ~PPU_PCR_DEV_REQ_EN;

        ppu_v0_set_power_mode(pd_ctx->ppu, PPU_V0_MODE_OFF);
        status = pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_OFF);

        FWK_LOG_INFO(
            "[PPUV0] set_state end. reg=(0x%p) state=(0x%x)",
            (void *)pd_ctx->ppu,
            state);

        assert(status == FWK_SUCCESS);
        break;

    default:
        FWK_LOG_ERR(
            "[PPUV0] Requested power state (%i) is not supported.", state);
        return FWK_E_PARAM;
    }

    (void)status;
    return FWK_SUCCESS;
}

static int pd_get_state(fwk_id_t pd_id, unsigned int *state)
{
    struct ppu_v0_pd_ctx *pd_ctx;

    pd_ctx = ppu_v0_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);

    return get_state(pd_ctx->ppu, state);
}

static int pd_reset(fwk_id_t pd_id)
{
    int status;
    struct ppu_v0_pd_ctx *pd_ctx;

    pd_ctx = ppu_v0_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);

    /* Model does not support warm reset at the moment. Using OFF instead. */
    status = ppu_v0_set_power_mode(pd_ctx->ppu, PPU_V0_MODE_OFF);
    if (status == FWK_SUCCESS)
        status = ppu_v0_set_power_mode(pd_ctx->ppu, PPU_V0_MODE_ON);

    return status;
}

static int ppu_v0_prepare_core_for_system_suspend(fwk_id_t core_pd_id)
{
    struct ppu_v0_pd_ctx *pd_ctx;

    pd_ctx = ppu_v0_ctx.pd_ctx_table + fwk_id_get_element_idx(core_pd_id);
    ppu_v0_request_power_mode(pd_ctx->ppu, PPU_V0_MODE_OFF);

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

static int ppu_v0_mod_init(
    fwk_id_t module_id,
    unsigned int pd_count,
    const void *unused)
{
    ppu_v0_ctx.pd_ctx_table =
        fwk_mm_calloc(pd_count, sizeof(struct ppu_v0_pd_ctx));

    return FWK_SUCCESS;
}

static int ppu_v0_pd_init(fwk_id_t pd_id, unsigned int unused, const void *data)
{
    const struct mod_ppu_v0_pd_config *config = data;
    struct ppu_v0_pd_ctx *pd_ctx;
    int status;

    if (config->pd_type >= MOD_PD_TYPE_COUNT)
        return FWK_E_DATA;

    pd_ctx = ppu_v0_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);
    pd_ctx->config = config;
    pd_ctx->ppu = (struct ppu_v0_reg *)(config->ppu.reg_base);
    pd_ctx->bound_id = FWK_ID_NONE;

    switch (config->pd_type) {
    case MOD_PD_TYPE_DEVICE:
    case MOD_PD_TYPE_DEVICE_DEBUG:
    case MOD_PD_TYPE_SYSTEM:
    case MOD_PD_TYPE_CORE:
    case MOD_PD_TYPE_CLUSTER:
        status = pd_init(pd_ctx);
        if (status != FWK_SUCCESS)
            return status;

        if (config->default_power_on)
            return ppu_v0_set_power_mode(pd_ctx->ppu, PPU_V0_MODE_ON);

        return FWK_SUCCESS;

    default:
        return FWK_E_SUPPORT;
    }
}

static int ppu_v0_bind(fwk_id_t id, unsigned int round)
{
    struct ppu_v0_pd_ctx *pd_ctx;

    /* Nothing to do during the first round of calls where the power module
       will bind to the power domains of this module. */
    if (round == 0)
        return FWK_SUCCESS;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    pd_ctx = ppu_v0_ctx.pd_ctx_table + fwk_id_get_element_idx(id);

    if (fwk_id_is_equal(pd_ctx->bound_id, FWK_ID_NONE))
        return FWK_SUCCESS;

    switch (fwk_id_get_module_idx(pd_ctx->bound_id)) {
#if BUILD_HAS_MOD_POWER_DOMAIN
    case FWK_MODULE_IDX_POWER_DOMAIN:
        return fwk_module_bind(
            pd_ctx->bound_id,
            mod_pd_api_id_driver_input,
            &pd_ctx->pd_driver_input_api);
        break;
#endif

#if BUILD_HAS_MOD_SYSTEM_POWER
    case FWK_MODULE_IDX_SYSTEM_POWER:
        return fwk_module_bind(
            pd_ctx->bound_id,
            mod_system_power_api_id_pd_driver_input,
            &pd_ctx->pd_driver_input_api);
        break;
#endif

    default:
        assert(false);
        return FWK_E_SUPPORT;
    }
}

static int ppu_v0_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t not_used,
    const void **api)
{
    struct ppu_v0_pd_ctx *pd_ctx;

    pd_ctx = ppu_v0_ctx.pd_ctx_table + fwk_id_get_element_idx(target_id);

    switch (pd_ctx->config->pd_type) {
    case MOD_PD_TYPE_SYSTEM:
        if (!fwk_id_is_equal(pd_ctx->bound_id, FWK_ID_NONE)) {
            assert(false);
            return FWK_E_ACCESS;
        }
        /* Fallthrough */

    case MOD_PD_TYPE_DEVICE:
    case MOD_PD_TYPE_DEVICE_DEBUG:
    case MOD_PD_TYPE_CLUSTER:
    case MOD_PD_TYPE_CORE:
#if BUILD_HAS_MOD_POWER_DOMAIN
        if (fwk_id_get_module_idx(source_id) == FWK_MODULE_IDX_POWER_DOMAIN) {
            pd_ctx->bound_id = source_id;
            *api = &pd_driver;
            break;
        }
#endif
#if BUILD_HAS_MOD_SYSTEM_POWER
        if (fwk_id_get_module_idx(source_id) == FWK_MODULE_IDX_SYSTEM_POWER) {
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

const struct fwk_module module_ppu_v0_synquacer = {
    .name = "PPU_V0_SYNQUACER",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = ppu_v0_mod_init,
    .element_init = ppu_v0_pd_init,
    .bind = ppu_v0_bind,
    .process_bind_request = ppu_v0_process_bind_request,
};

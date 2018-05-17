/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <fwk_assert.h>
#include <fwk_errno.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <mod_juno_ppu.h>
#include <mod_power_domain.h>
#include <mod_timer.h>
#include <mod_system_power.h>
#include <juno_ppu.h>
#include <juno_ppu_idx.h>
#include <scp_config.h>
#include <system_mmap.h>

#define PPU_SET_STATE_AND_WAIT_TIMEOUT_US       (100 * 1000)

struct set_power_status_check_param {
    enum ppu_mode mode;
    struct ppu_reg *reg;
};

static unsigned int ppu_mode_to_pd_state[] = {
    [PPU_MODE_OFF] = MOD_PD_STATE_OFF,
    [PPU_MODE_ON] = MOD_PD_STATE_ON,
    [PPU_MODE_MEM_RET] = MOD_PD_STATE_ON,
    [PPU_MODE_WARM_RESET] = MOD_PD_STATE_ON,
};

static enum ppu_mode pd_state_to_ppu_mode[] = {
    [MOD_PD_STATE_OFF] = PPU_MODE_OFF,
    [MOD_PD_STATE_SLEEP] = PPU_MODE_OFF,
    [MOD_PD_STATE_ON] = PPU_MODE_ON,
};

static struct module_ctx juno_ppu_ctx;

/*
 * Utility Functions
 */
static bool check_mode(enum ppu_mode mode)
{
    return ((mode == PPU_MODE_OFF) ||
            (mode == PPU_MODE_MEM_RET) ||
            (mode == PPU_MODE_WARM_RESET) ||
            (mode == PPU_MODE_ON));
}

static bool set_power_status_check(void *data)
{
    struct set_power_status_check_param *param;

    fwk_assert(data != NULL);
    param = (struct set_power_status_check_param *)data;

    return ((param->reg->POWER_STATUS & PPU_REG_PPR_PSR) == param->mode);
}

static int get_ctx(fwk_id_t id, struct ppu_ctx **ppu_ctx)
{
    int status;

    fwk_assert(fwk_module_is_valid_element_id(id));

    status = fwk_module_check_call(id);
    if (!fwk_expect(status == FWK_SUCCESS))
        return FWK_E_ACCESS;

    *ppu_ctx = juno_ppu_ctx.ppu_ctx_table + fwk_id_get_element_idx(id);

    return FWK_SUCCESS;
}

static void disable_scp_remap(void)
{
    /* The memory under the SYSTOP power domain is accessible */
    SCP_CONFIG->SCP_CONTROL &= ~SCP_CONFIG_SCP_CONTROL_SCPSYSREMAPEN;
}

static void power_state_systop_init(struct ppu_reg *reg)
{
    /* Wait for SYSTOP to power up */
    while ((reg->POWER_STATUS & PPU_REG_PPR_PSR) != PPU_MODE_ON)
        continue;

    disable_scp_remap();
}

static int ppu_set_state_and_wait(struct ppu_reg *reg,
                                  enum ppu_mode mode,
                                  fwk_id_t ppu_id)
{
    int status;
    struct ppu_ctx *ppu_ctx;
    const struct mod_juno_ppu_element_config *dev_config;
    struct set_power_status_check_param param;

    fwk_assert(reg != NULL);

    status = get_ctx(ppu_id, &ppu_ctx);
    if (status != FWK_SUCCESS)
        return status;

    dev_config = ppu_ctx->config;

    if (!fwk_expect(check_mode(mode)))
        return FWK_E_PARAM;

    reg->POWER_POLICY = mode;

    param.mode = mode;
    param.reg = reg;

    if (fwk_id_is_equal(dev_config->timer_id, FWK_ID_NONE)) {
        /* Wait for the PPU to set */
        while (!set_power_status_check(&param))
            continue;
    } else {
        /* Allow the new PPU policy to set within a pre-defined timeout */
        status = ppu_ctx->timer_api->wait(dev_config->timer_id,
                                          PPU_SET_STATE_AND_WAIT_TIMEOUT_US,
                                          set_power_status_check,
                                          &param);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int ppu_get_state(struct ppu_reg *reg, enum ppu_mode *mode)
{
    *mode = (enum ppu_mode)(reg->POWER_STATUS & PPU_REG_PPR_PSR);

    return FWK_SUCCESS;
}

/*
 * Power Domain driver API
 */
static int pd_set_state(fwk_id_t ppu_id, unsigned int state)
{
    int status;
    struct ppu_ctx *ppu_ctx;
    enum ppu_mode mode;

    status = get_ctx(ppu_id, &ppu_ctx);
    if (status != FWK_SUCCESS)
        return status;

    mode = pd_state_to_ppu_mode[state];
    if (!fwk_expect(state < MOD_PD_STATE_COUNT))
        return FWK_E_PARAM;

    switch (state) {
    case MOD_PD_STATE_ON:
    case MOD_PD_STATE_OFF:
        status = ppu_set_state_and_wait(ppu_ctx->reg, mode, ppu_id);
        if (status != FWK_SUCCESS)
            return status;
        break;

    default:
        return FWK_E_SUPPORT;
    }

    status = ppu_ctx->pd_api->report_power_state_transition(ppu_ctx->bound_id,
        state);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

static int pd_get_state(fwk_id_t ppu_id, unsigned int *state)
{
    int status;
    struct ppu_ctx *ppu_ctx;
    enum ppu_mode mode;

    if (!fwk_expect(state != NULL))
        return FWK_E_PARAM;

    status = get_ctx(ppu_id, &ppu_ctx);
    if (status != FWK_SUCCESS)
        return status;

    mode = ppu_ctx->reg->POWER_STATUS & PPU_REG_PPR_PSR;

    if (!check_mode(mode))
        return FWK_E_DEVICE;

    *state = ppu_mode_to_pd_state[mode];

    return FWK_SUCCESS;
}

static int pd_reset(fwk_id_t ppu_id)
{
    int status;
    struct ppu_ctx *ppu_ctx;

    status = get_ctx(ppu_id, &ppu_ctx);
    if (status != FWK_SUCCESS)
        return status;

    status = ppu_set_state_and_wait(ppu_ctx->reg, PPU_MODE_WARM_RESET, ppu_id);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    status = ppu_set_state_and_wait(ppu_ctx->reg, PPU_MODE_ON, ppu_id);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}

static int pd_shutdown(fwk_id_t ppu_id,
                       enum mod_pd_system_shutdown system_shutdown)
{
    if (system_shutdown == MOD_PD_SYSTEM_WARM_RESET)
        return FWK_E_SUPPORT;

    return FWK_SUCCESS;
}

static int pd_prepare_core_for_system_suspend(fwk_id_t ppu_id)
{
    return FWK_E_SUPPORT;
}

static struct mod_pd_driver_api pd_driver_api = {
    .set_state = pd_set_state,
    .get_state = pd_get_state,
    .reset = pd_reset,
    .prepare_core_for_system_suspend = pd_prepare_core_for_system_suspend,
    .shutdown = pd_shutdown,
};

/*
 * ROM API
 */
static int rom_set_state_and_wait(fwk_id_t ppu_id, unsigned int state)
{
    int status;
    enum ppu_mode mode;
    struct ppu_ctx *ppu_ctx;

    if (!fwk_expect(state < MOD_PD_STATE_COUNT))
        return FWK_E_PARAM;

    status = get_ctx(ppu_id, &ppu_ctx);
    if (status != FWK_SUCCESS)
        return status;

    mode = pd_state_to_ppu_mode[state];

    status = ppu_set_state_and_wait(ppu_ctx->reg, mode, ppu_id);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}

static struct mod_juno_ppu_rom_api rom_api = {
    .set_state_and_wait = rom_set_state_and_wait,
    .get_state = pd_get_state,
};

/*
 * Framework API
 */
static int juno_ppu_module_init(fwk_id_t module_id,
                                unsigned int element_count,
                                const void *data)
{
    if (!fwk_expect(element_count > 0))
        return FWK_E_PANIC;

    juno_ppu_ctx.ppu_ctx_table = fwk_mm_calloc(element_count,
        sizeof(struct ppu_ctx));
    if (juno_ppu_ctx.ppu_ctx_table == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int juno_ppu_element_init(fwk_id_t ppu_id,
                                 unsigned int subelement_count,
                                 const void *data)
{
    const struct mod_juno_ppu_element_config *dev_config = data;
    struct ppu_ctx *ppu_ctx;
    int status;
    enum ppu_mode mode;

    ppu_ctx = juno_ppu_ctx.ppu_ctx_table + fwk_id_get_element_idx(ppu_id);

    if (dev_config->reg_base == 0)
        return FWK_E_PANIC;

    ppu_ctx->config = dev_config;
    ppu_ctx->reg = (struct ppu_reg *)dev_config->reg_base;
    ppu_ctx->bound_id = FWK_ID_NONE;

    if (dev_config->pd_type == MOD_PD_TYPE_SYSTEM) {
        status = ppu_get_state(ppu_ctx->reg, &mode);
        if (status != FWK_SUCCESS)
            return status;

        if ((dev_config->reg_base == PPU_SYSTOP_BASE) && (mode != PPU_MODE_ON))
            power_state_systop_init(ppu_ctx->reg);
    }

    return FWK_SUCCESS;
}

static int juno_ppu_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct ppu_ctx *ppu_ctx;
    const struct mod_juno_ppu_element_config *dev_config;

    (void)dev_config;
    (void)status;

    /* Bind in the second round */
    if ((!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) || (round == 0))
        return FWK_SUCCESS;

    ppu_ctx = juno_ppu_ctx.ppu_ctx_table + fwk_id_get_element_idx(id);
    dev_config = ppu_ctx->config;

    #if BUILD_HAS_MOD_TIMER
    if (!fwk_id_is_equal(dev_config->timer_id, FWK_ID_NONE)) {
        /* Bind to the timer */
        status = fwk_module_bind(dev_config->timer_id,
            MOD_TIMER_API_ID_TIMER, &ppu_ctx->timer_api);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;
    }
    #endif

    if (!fwk_id_is_equal(ppu_ctx->bound_id, FWK_ID_NONE)) {
        /* Bind back to the entity that bound to us (if any) */
        switch (fwk_id_get_module_idx(ppu_ctx->bound_id)) {
        #if BUILD_HAS_MOD_POWER_DOMAIN
        case FWK_MODULE_IDX_POWER_DOMAIN:
            /* Bind back to the PD module */
            status = fwk_module_bind(ppu_ctx->bound_id,
                mod_pd_api_id_driver_input, &ppu_ctx->pd_api);
            if (status != FWK_SUCCESS)
                return FWK_E_PANIC;

            break;
        #endif

        #if BUILD_HAS_MOD_SYSTEM_POWER
        case FWK_MODULE_IDX_SYSTEM_POWER:
            /* Bind back to the System Power module */
            status = fwk_module_bind(ppu_ctx->bound_id,
                mod_system_power_api_id_pd_driver_input, &ppu_ctx->pd_api);
            if (status != FWK_SUCCESS)
                return FWK_E_PANIC;

            break;
        #endif

        default:
            fwk_assert(false);
            return FWK_E_SUPPORT;

            break;
        }
    }

    return FWK_SUCCESS;
}

static int juno_ppu_process_bind_request(fwk_id_t requester_id,
                                         fwk_id_t id,
                                         fwk_id_t api_id,
                                         const void **api)
{
    struct ppu_ctx *ppu_ctx;

    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_JUNO_PPU_API_IDX_PD:
        ppu_ctx = juno_ppu_ctx.ppu_ctx_table + fwk_id_get_element_idx(id);
        ppu_ctx->bound_id = requester_id;

        *api = &pd_driver_api;

        return FWK_SUCCESS;

    case MOD_JUNO_PPU_API_IDX_ROM:
        *api = &rom_api;

        return FWK_SUCCESS;

    default:
        return FWK_E_SUPPORT;
    }
}

const struct fwk_module module_juno_ppu = {
    .name = "Juno PPU",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_JUNO_PPU_API_COUNT,
    .init = juno_ppu_module_init,
    .element_init = juno_ppu_element_init,
    .bind = juno_ppu_bind,
    .process_bind_request = juno_ppu_process_bind_request,
};

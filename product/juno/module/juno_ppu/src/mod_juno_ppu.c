/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_ppu.h"
#include "juno_ppu_idx.h"
#include "juno_utils.h"
#include "scp_config.h"
#include "system_mmap.h"

#include <mod_juno_ppu.h>
#include <mod_power_domain.h>
#include <mod_system_power.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PPU_SET_STATE_AND_WAIT_TIMEOUT_US   (100 * 1000)

#define PPU_ALARM_TIMEOUT_SUSPEND_MS        5

#define CPU_WAKEUP_COMPOSITE_STATE  MOD_PD_COMPOSITE_STATE(MOD_PD_LEVEL_2, \
                                                           0, \
                                                           MOD_PD_STATE_ON, \
                                                           MOD_PD_STATE_ON, \
                                                           MOD_PD_STATE_ON)

struct set_power_status_check_params {
    enum ppu_mode mode;
    struct ppu_reg *reg;
};

/*
 * Internal variables
 */
static unsigned int ppu_mode_to_pd_state[] = {
    [PPU_MODE_OFF] = MOD_PD_STATE_OFF,
    [PPU_MODE_ON] = MOD_PD_STATE_ON,
    /* Only CSS can be placed in MEM_RET */
    [PPU_MODE_MEM_RET] = MOD_SYSTEM_POWER_POWER_STATE_SLEEP0,
    [PPU_MODE_WARM_RESET] = MOD_PD_STATE_ON,
};

static enum ppu_mode pd_state_to_ppu_mode[] = {
    [MOD_PD_STATE_OFF] = PPU_MODE_OFF,
    [MOD_PD_STATE_SLEEP] = PPU_MODE_OFF,
    [MOD_PD_STATE_ON] = PPU_MODE_ON,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = PPU_MODE_MEM_RET,
};

/*!
 * \brief Additional internal Juno power domain states.
 */
enum juno_power_domain_states {
    JUNO_POWER_DOMAIN_CSS_SLEEP0_EMU = MOD_SYSTEM_POWER_POWER_STATE_COUNT,
    JUNO_POWER_DOMAIN_STATE_COUNT
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
    struct set_power_status_check_params *params;

    fwk_assert(data != NULL);
    params = (struct set_power_status_check_params *)data;

    return ((params->reg->POWER_STATUS & PPU_REG_PPR_PSR) == params->mode);
}

static void get_ctx(fwk_id_t id, struct ppu_ctx **ppu_ctx)
{
    fwk_assert(fwk_module_is_valid_element_id(id));

    *ppu_ctx = &juno_ppu_ctx.ppu_ctx_table[fwk_id_get_element_idx(id)];
}

static int clear_pending_wakeup_irq(
    const struct mod_juno_ppu_element_config *config)
{
    int status;

    status = fwk_interrupt_clear_pending(config->wakeup_irq);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_interrupt_clear_pending(config->wakeup_fiq);
}

static int enable_wakeup_irq(const struct mod_juno_ppu_element_config *config)
{
    int status;

    status = fwk_interrupt_enable(config->wakeup_irq);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_interrupt_enable(config->wakeup_fiq);
}

static int disable_wakeup_irq(const struct mod_juno_ppu_element_config *config)
{
    int status;

    status = fwk_interrupt_disable(config->wakeup_irq);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_interrupt_disable(config->wakeup_fiq);
}

static void disable_scp_remap(void)
{
    /* The memory under the SYSTOP power domain is accessible */
    SCP_CONFIG->SCP_CONTROL &= ~SCP_CONFIG_SCP_CONTROL_SCPSYSREMAPEN;
}

static void enable_scp_remap(void)
{
    /* SCP re-map to catch invalid access to SYSTOP address space */
    SCP_CONFIG->SCP_CONTROL |= SCP_CONFIG_SCP_CONTROL_SCPSYSREMAPEN;
}

static void power_state_systop_init(struct ppu_reg *reg)
{
    /* Wait for SYSTOP to power up */
    while ((reg->POWER_STATUS & PPU_REG_PPR_PSR) != PPU_MODE_ON)
        continue;

    disable_scp_remap();
}

static int ppu_request_state(struct ppu_ctx *ppu_ctx,
                             enum ppu_mode mode)
{
    struct ppu_reg *reg;

    reg = ppu_ctx->reg;
    fwk_assert(reg != NULL);

    fwk_assert(check_mode(mode));

    reg->POWER_POLICY = mode;

    return FWK_SUCCESS;
}

static int ppu_set_state_and_wait(struct ppu_ctx *ppu_ctx,
                                  enum ppu_mode mode)
{
    int status;
    struct ppu_reg *reg;
    const struct mod_juno_ppu_element_config *dev_config;
    struct set_power_status_check_params params;

    reg = ppu_ctx->reg;
    fwk_assert(reg != NULL);

    dev_config = ppu_ctx->config;

    fwk_assert(check_mode(mode));

    reg->POWER_POLICY = mode;

    params.mode = mode;
    params.reg = reg;

    if (fwk_id_is_equal(dev_config->timer_id, FWK_ID_NONE)) {
        /* Wait for the PPU to set */
        while (!set_power_status_check(&params))
            continue;
    } else {
        /* Allow the new PPU policy to set within a pre-defined timeout */
        status = ppu_ctx->timer_api->wait(dev_config->timer_id,
                                          PPU_SET_STATE_AND_WAIT_TIMEOUT_US,
                                          set_power_status_check,
                                          &params);
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

static int get_state(struct ppu_ctx *ppu_ctx, unsigned int *state)
{
    enum ppu_mode mode = ppu_ctx->reg->POWER_STATUS & PPU_REG_PPR_PSR;

    if (!check_mode(mode))
        return FWK_E_DEVICE;

    *state = ppu_mode_to_pd_state[mode];

    return FWK_SUCCESS;
}

static void juno_ppu_alarm_callback(uintptr_t param)
{
    int status;
    unsigned int state = 0;
    struct ppu_ctx *ppu_ctx = (struct ppu_ctx *)param;
    const struct mod_juno_ppu_config *config;

    status = get_state(ppu_ctx, &state);
    fwk_assert(status == FWK_SUCCESS);

    if (state == MOD_PD_STATE_OFF) {
        config = fwk_module_get_data(fwk_module_id_juno_ppu);
        fwk_assert(config != NULL);

        status = ppu_ctx->pd_api->report_power_state_transition(
            ppu_ctx->bound_id,
            MOD_PD_STATE_OFF);
        fwk_assert(status == FWK_SUCCESS);

        status = juno_ppu_ctx.alarm_api->stop(config->timer_alarm_id);
        fwk_assert(status == FWK_SUCCESS);
    }
}

/*
 * Power Domain driver API
 */
static int pd_set_state(fwk_id_t ppu_id, unsigned int state)
{
    int status;
    struct ppu_ctx *ppu_ctx;
    enum ppu_mode mode;

    get_ctx(ppu_id, &ppu_ctx);

    if (!fwk_expect(state < MOD_PD_STATE_COUNT))
        return FWK_E_PARAM;

    mode = pd_state_to_ppu_mode[state];

    switch (state) {
    case MOD_PD_STATE_ON:
    case MOD_PD_STATE_OFF:
        status = ppu_set_state_and_wait(ppu_ctx, mode);
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
    struct ppu_ctx *ppu_ctx;

    if (!fwk_expect(state != NULL))
        return FWK_E_PARAM;

    get_ctx(ppu_id, &ppu_ctx);

    return get_state(ppu_ctx, state);
}

static int pd_reset(fwk_id_t ppu_id)
{
    int status;
    struct ppu_ctx *ppu_ctx;

    get_ctx(ppu_id, &ppu_ctx);

    status = ppu_set_state_and_wait(ppu_ctx, PPU_MODE_WARM_RESET);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    status = ppu_set_state_and_wait(ppu_ctx, PPU_MODE_ON);
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

static const struct mod_pd_driver_api pd_driver_api = {
    .set_state = pd_set_state,
    .get_state = pd_get_state,
    .reset = pd_reset,
    .prepare_core_for_system_suspend = pd_prepare_core_for_system_suspend,
};

/*
 * DBGSYS API
 */
static int dbgsys_set_state(fwk_id_t ppu_id, unsigned int state)
{
    int status;
    struct ppu_ctx *ppu_ctx;

    get_ctx(ppu_id, &ppu_ctx);

    if (state != MOD_PD_STATE_ON)
        return FWK_E_PWRSTATE;

    status = ppu_set_state_and_wait(ppu_ctx, PPU_MODE_ON);
    if (status != FWK_SUCCESS)
        return status;

    status = ppu_ctx->pd_api->report_power_state_transition(ppu_ctx->bound_id,
        state);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    juno_ppu_ctx.dbgsys_state = MOD_PD_STATE_ON;

    return FWK_SUCCESS;
}

static const struct mod_pd_driver_api dbgsys_pd_driver_api = {
    .set_state = dbgsys_set_state,
    .get_state = pd_get_state,
    .reset = pd_reset,
    .prepare_core_for_system_suspend = pd_prepare_core_for_system_suspend,
    .shutdown = pd_shutdown,
};

/*
 * CSS API
 */
static int css_set_state(fwk_id_t ppu_id, unsigned int state)
{
    enum ppu_mode mode;
    int status;
    struct ppu_ctx *ppu_ctx;

    get_ctx(ppu_id, &ppu_ctx);

    if (!fwk_expect(state < MOD_SYSTEM_POWER_POWER_STATE_COUNT))
        return FWK_E_PARAM;

    mode = pd_state_to_ppu_mode[state];

    switch (state) {
    case MOD_PD_STATE_ON:
        /* Resuming from emulated sleep, nothing to do */
        if (juno_ppu_ctx.css_state == JUNO_POWER_DOMAIN_CSS_SLEEP0_EMU) {
            juno_ppu_ctx.css_state = state;

            break;
        }

        /* Power up SYSTOP */
        status = ppu_set_state_and_wait(ppu_ctx, mode);
        if (status != FWK_SUCCESS)
            return status;

        juno_ppu_ctx.css_state = state;

        disable_scp_remap();

        break;

    case MOD_PD_STATE_OFF:
        if (juno_ppu_ctx.dbgsys_state == MOD_PD_STATE_ON) {
            juno_ppu_ctx.css_state = JUNO_POWER_DOMAIN_CSS_SLEEP0_EMU;

            break;
        }
        /* Fall through */

    case MOD_SYSTEM_POWER_POWER_STATE_SLEEP0:
        FWK_LOG_FLUSH();

        enable_scp_remap();

        status = ppu_set_state_and_wait(ppu_ctx, mode);
        if (status != FWK_SUCCESS)
            return status;

        juno_ppu_ctx.css_state = state;

        break;

    case MOD_SYSTEM_POWER_POWER_STATE_SLEEP1:
        return FWK_E_SUPPORT;

    default:
        return FWK_E_PARAM;
    }

    status = ppu_ctx->pd_api->report_power_state_transition(ppu_ctx->bound_id,
        state);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

static int css_get_state(fwk_id_t ppu_id, unsigned int *state)
{
    *state = juno_ppu_ctx.css_state;

    return FWK_SUCCESS;
}

static int css_reset(fwk_id_t ppu_id)
{
    return FWK_E_SUPPORT;
}

static const struct mod_pd_driver_api css_pd_driver_api = {
    .set_state = css_set_state,
    .get_state = css_get_state,
    .reset = css_reset,
};

/*
 * Cluster API
 */
static int cluster_set_state(fwk_id_t ppu_id, unsigned int state)
{
    enum ppu_mode mode;
    int status;
    volatile uint32_t *snoop_ctrl;
    struct ppu_ctx *ppu_ctx;

    get_ctx(ppu_id, &ppu_ctx);

    if ((uintptr_t)ppu_ctx->reg == PPU_BIG_SSTOP_BASE)
        snoop_ctrl = &SCP_CONFIG->BIG_SNOOP_CONTROL;
    else if ((uintptr_t)ppu_ctx->reg == PPU_LITTLE_SSTOP_BASE)
        snoop_ctrl = &SCP_CONFIG->LITTLE_SNOOP_CONTROL;
    else
        return FWK_E_PARAM;

    if (!fwk_expect(state < MOD_PD_STATE_COUNT))
        return FWK_E_PARAM;

    mode = pd_state_to_ppu_mode[state];

    switch (state) {
    case MOD_PD_STATE_ON:
        status = ppu_set_state_and_wait(ppu_ctx, mode);
        if (status != FWK_SUCCESS)
            return status;

        juno_utils_open_snoop_gate_and_wait(snoop_ctrl);
        break;

    case MOD_PD_STATE_OFF:
        fwk_assert(juno_ppu_ctx.dbgsys_state == MOD_PD_STATE_OFF);

        juno_utils_close_snoop_gate(snoop_ctrl);

        status = ppu_set_state_and_wait(ppu_ctx, mode);
        if (status != FWK_SUCCESS)
            return status;

        break;

    default:
        return FWK_E_PANIC;
    }

    status = ppu_ctx->pd_api->report_power_state_transition(ppu_ctx->bound_id,
        state);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

static bool cluster_deny(fwk_id_t ppu_id, unsigned int state)
{

    fwk_assert(fwk_module_is_valid_element_id(ppu_id));

    if (!fwk_expect(state < MOD_PD_STATE_COUNT))
        return true;

    if (state != MOD_PD_STATE_OFF)
        return false;

    return (juno_ppu_ctx.dbgsys_state == MOD_PD_STATE_ON);
}

static const struct mod_pd_driver_api cluster_pd_driver_api = {
    .set_state = cluster_set_state,
    .get_state = pd_get_state,
    .reset = pd_reset,
    .deny = cluster_deny,
};

/*
 * Cores API
 */
static int core_set_state(fwk_id_t ppu_id, unsigned int state)
{
    int status;
    enum ppu_mode mode;
    struct ppu_ctx *ppu_ctx;
    const struct mod_juno_ppu_element_config *dev_config;

    get_ctx(ppu_id, &ppu_ctx);

    dev_config = ppu_ctx->config;

    if (!fwk_expect(state < MOD_PD_STATE_COUNT))
        return FWK_E_PARAM;

    mode = pd_state_to_ppu_mode[state];

    switch (state) {
    case MOD_PD_STATE_OFF:
        status = ppu_set_state_and_wait(ppu_ctx, mode);
        if (status != FWK_SUCCESS)
            return status;

        status = clear_pending_wakeup_irq(dev_config);
        if (status != FWK_SUCCESS)
            return status;

        status = disable_wakeup_irq(dev_config);
        if (status != FWK_SUCCESS)
            return status;

        status = fwk_interrupt_disable(dev_config->warm_reset_irq);

        break;

    case MOD_PD_STATE_SLEEP:
        status = ppu_set_state_and_wait(ppu_ctx, mode);
        if (status != FWK_SUCCESS)
            return status;

        status = clear_pending_wakeup_irq(dev_config);
        if (status != FWK_SUCCESS)
            return status;

        status = enable_wakeup_irq(dev_config);
        if (status != FWK_SUCCESS)
            return status;

        status = fwk_interrupt_disable(dev_config->warm_reset_irq);

        break;

    case MOD_PD_STATE_ON:
        status = fwk_interrupt_clear_pending(dev_config->warm_reset_irq);
        if (status != FWK_SUCCESS)
            return status;

        status = fwk_interrupt_enable(dev_config->warm_reset_irq);
        if (status != FWK_SUCCESS)
            return status;

        status = ppu_set_state_and_wait(ppu_ctx, mode);

        break;

    default:
        fwk_assert(false);
        status = FWK_E_PANIC;

        break;
    }

    if (status != FWK_SUCCESS)
        return status;

    status = ppu_ctx->pd_api->report_power_state_transition(ppu_ctx->bound_id,
        state);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

static int core_reset(fwk_id_t ppu_id)
{
    int status;
    struct ppu_ctx *ppu_ctx;
    const struct mod_juno_ppu_element_config *dev_config;

    get_ctx(ppu_id, &ppu_ctx);

    dev_config = ppu_ctx->config;

    status = ppu_set_state_and_wait(ppu_ctx, PPU_MODE_WARM_RESET);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    status = fwk_interrupt_clear_pending(dev_config->warm_reset_irq);
    if (status != FWK_SUCCESS)
        return status;

    return ppu_set_state_and_wait(ppu_ctx, PPU_MODE_ON);
}

static int core_prepare_core_for_system_suspend(fwk_id_t ppu_id)
{
    struct ppu_ctx *ppu_ctx;
    int status;
    const struct mod_juno_ppu_element_config *dev_config;
    const struct mod_juno_ppu_config *config;

    get_ctx(ppu_id, &ppu_ctx);

    dev_config = ppu_ctx->config;

    status = disable_wakeup_irq(dev_config);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_interrupt_disable(dev_config->warm_reset_irq);
    if (status != FWK_SUCCESS)
        return status;

    status = ppu_request_state(ppu_ctx, PPU_MODE_OFF);
    if (status != FWK_SUCCESS)
        return status;

    config = fwk_module_get_data(fwk_module_id_juno_ppu);
    fwk_assert(config != NULL);

    /* Start the timer alarm to poll for the power policy change */
    return juno_ppu_ctx.alarm_api->start(
        config->timer_alarm_id,
        PPU_ALARM_TIMEOUT_SUSPEND_MS,
        MOD_TIMER_ALARM_TYPE_PERIODIC,
        juno_ppu_alarm_callback,
        (uintptr_t)ppu_ctx);
}

static const struct mod_pd_driver_api core_pd_driver_api = {
    .set_state = core_set_state,
    .get_state = pd_get_state,
    .reset = core_reset,
    .prepare_core_for_system_suspend = core_prepare_core_for_system_suspend,
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

    get_ctx(ppu_id, &ppu_ctx);

    mode = pd_state_to_ppu_mode[state];

    status = ppu_set_state_and_wait(ppu_ctx, mode);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}

static const struct mod_juno_ppu_rom_api rom_api = {
    .set_state_and_wait = rom_set_state_and_wait,
    .get_state = pd_get_state,
};

/*
 * Interrupt handlers
 */

static void core_wakeup_handler(uintptr_t param)
{
    struct ppu_ctx *ppu_ctx;
    int status;
    const struct mod_juno_ppu_element_config *dev_config;

    ppu_ctx = (struct ppu_ctx *)param;

    dev_config = ppu_ctx->config;

    status = disable_wakeup_irq(dev_config);
    fwk_assert(status == FWK_SUCCESS);

    status = ppu_ctx->pd_api->set_composite_state_async(ppu_ctx->bound_id,
        false, CPU_WAKEUP_COMPOSITE_STATE);
    fwk_assert(status == FWK_SUCCESS);
}

static void core_warm_reset_handler(uintptr_t param)
{
    struct ppu_ctx *ppu_ctx;
    int status;
    const struct mod_juno_ppu_element_config *dev_config;

    ppu_ctx = (struct ppu_ctx *)param;

    dev_config = ppu_ctx->config;

    status = fwk_interrupt_disable(dev_config->warm_reset_irq);
    fwk_assert(status == FWK_SUCCESS);

    status = ppu_set_state_and_wait(ppu_ctx, PPU_MODE_WARM_RESET);
    fwk_assert(status == FWK_SUCCESS);

    status = fwk_interrupt_clear_pending(dev_config->warm_reset_irq);
    fwk_assert(status == FWK_SUCCESS);

    status = ppu_set_state_and_wait(ppu_ctx, PPU_MODE_ON);
    fwk_assert(status == FWK_SUCCESS);

    status = fwk_interrupt_enable(dev_config->warm_reset_irq);
    fwk_assert(status == FWK_SUCCESS);
}

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

    juno_ppu_ctx.css_state = MOD_PD_STATE_ON;

    juno_ppu_ctx.dbgsys_state = MOD_PD_STATE_OFF;

    #if BUILD_HAS_MOD_TIMER
    fwk_assert(data != NULL);
    #endif

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
    const struct mod_juno_ppu_config *config;

    (void)config;
    (void)dev_config;
    (void)status;

    /* Bind in the second round only */
    if (round == 0)
        return FWK_SUCCESS;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        #if BUILD_HAS_MOD_TIMER
        config = fwk_module_get_data(fwk_module_id_juno_ppu);

        status = fwk_module_bind(
            config->timer_alarm_id,
            MOD_TIMER_API_ID_ALARM,
            &juno_ppu_ctx.alarm_api);
        if (status != FWK_SUCCESS)
            return FWK_E_HANDLER;
        #endif

        return FWK_SUCCESS;
    }

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
    const struct mod_juno_ppu_element_config *dev_config;

    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_JUNO_PPU_API_IDX_PD:
        ppu_ctx = juno_ppu_ctx.ppu_ctx_table + fwk_id_get_element_idx(id);

        ppu_ctx->bound_id = requester_id;

        dev_config = ppu_ctx->config;

        switch (dev_config->pd_type) {
        case MOD_PD_TYPE_CORE:
            *api = &core_pd_driver_api;

            break;

        case MOD_PD_TYPE_CLUSTER:
            *api = &cluster_pd_driver_api;

            break;

        case MOD_PD_TYPE_SYSTEM:
        case MOD_PD_TYPE_DEVICE:
        case MOD_PD_TYPE_DEVICE_DEBUG:
            if (fwk_id_get_element_idx(id) == JUNO_PPU_DEV_IDX_SYSTOP)
                *api = &css_pd_driver_api;
            else if (fwk_id_get_element_idx(id) == JUNO_PPU_DEV_IDX_DBGSYS)
                *api = &dbgsys_pd_driver_api;
            else
                *api = &pd_driver_api;

            break;

        default:
            return FWK_E_PARAM;
        }

        return FWK_SUCCESS;

    case MOD_JUNO_PPU_API_IDX_ROM:
        *api = &rom_api;

        return FWK_SUCCESS;

    default:
        return FWK_E_SUPPORT;
    }
}

static int juno_ppu_start(fwk_id_t id)
{
    int status;
    struct ppu_ctx *ppu_ctx;
    unsigned int warm_reset_irq;
    unsigned int wakeup_irq;
    unsigned int wakeup_fiq;
    enum ppu_mode mode;
    const struct mod_juno_ppu_element_config *dev_config;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    ppu_ctx = juno_ppu_ctx.ppu_ctx_table + fwk_id_get_element_idx(id);

    dev_config = ppu_ctx->config;

    if (ppu_ctx->config->pd_type != MOD_PD_TYPE_CORE)
        return FWK_SUCCESS;

    warm_reset_irq = ppu_ctx->config->warm_reset_irq;
    wakeup_irq = ppu_ctx->config->wakeup_irq;
    wakeup_fiq = ppu_ctx->config->wakeup_fiq;

    /*
     * Perform ISR configuration only when interrupts ID is provided
     */
    if ((warm_reset_irq == 0) || (wakeup_irq == 0) || (wakeup_fiq == 0))
        return FWK_SUCCESS;

    /*
     * Warm reset interrupt request
     */
    status = fwk_interrupt_set_isr_param(warm_reset_irq,
                                         core_warm_reset_handler,
                                         (uintptr_t)ppu_ctx);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    status = ppu_get_state(ppu_ctx->reg, &mode);
    if (status != FWK_SUCCESS)
        return status;

    if (mode == PPU_MODE_ON) {
        status = fwk_interrupt_clear_pending(dev_config->warm_reset_irq);
        if (status != FWK_SUCCESS)
            return status;

        status = fwk_interrupt_enable(dev_config->warm_reset_irq);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;
    }

    /*
     * Wake-up reset interrupt request
     */
    status = fwk_interrupt_set_isr_param(wakeup_irq,
                                         core_wakeup_handler,
                                         (uintptr_t)ppu_ctx);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    status = fwk_interrupt_set_isr_param(wakeup_fiq,
                                         core_wakeup_handler,
                                         (uintptr_t)ppu_ctx);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

const struct fwk_module module_juno_ppu = {
    .name = "Juno PPU",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_JUNO_PPU_API_COUNT,
    .init = juno_ppu_module_init,
    .element_init = juno_ppu_element_init,
    .bind = juno_ppu_bind,
    .process_bind_request = juno_ppu_process_bind_request,
    .start = juno_ppu_start,
};

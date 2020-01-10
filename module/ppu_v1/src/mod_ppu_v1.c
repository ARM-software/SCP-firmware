/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power State Management PPU v1 driver.
 */

#include <ppu_v1.h>

#include <mod_power_domain.h>
#include <mod_ppu_v1.h>

#if BUILD_HAS_MOD_SYSTEM_POWER
#    include <mod_system_power.h>
#endif

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>

#define CORE_PER_CLUSTER_COUNT_MAX 8

/* Power domain context */
struct ppu_v1_pd_ctx {
    /* Power domain configuration data */
    const struct mod_ppu_v1_pd_config *config;

    /* PPU registers */
    struct ppu_v1_reg *ppu;

    /* Identifier of the entity bound to the power domain driver API */
    fwk_id_t bound_id;

    /* Power module driver input API */
    struct mod_pd_driver_input_api *pd_driver_input_api;

    /* Context of the parent power domain (used only for core power domains) */
    struct ppu_v1_pd_ctx *parent_pd_ctx;

    /* Pointer to the power state observer API */
    const struct mod_ppu_v1_power_state_observer_api *observer_api;

    /* Context data specific to the type of power domain */
    void *data;
};

/* Cluster power domain specific context */
struct ppu_v1_cluster_pd_ctx {
    /*
     * Table of pointers to the contexts of the cores being part of the
     * cluster.
     */
    struct ppu_v1_pd_ctx *core_pd_ctx_table[CORE_PER_CLUSTER_COUNT_MAX];

    /* Number of cores */
    unsigned int core_count;
};

/* Module context */
struct ppu_v1_ctx {
    /* Table of the power domain contexts */
    struct ppu_v1_pd_ctx *pd_ctx_table;

    /* Number of power domains */
    size_t pd_ctx_table_size;
};

/*
 * Internal variables
 */

static struct ppu_v1_ctx ppu_v1_ctx;

#define MODE_UNSUPPORTED        ~0U
static const uint8_t ppu_mode_to_power_state[] = {
    [PPU_V1_MODE_OFF]         = (uint8_t)MOD_PD_STATE_OFF,
    [PPU_V1_MODE_OFF_EMU]     = (uint8_t)MOD_PD_STATE_OFF,
    [PPU_V1_MODE_MEM_RET]     = (uint8_t)MOD_PD_STATE_OFF,
    [PPU_V1_MODE_MEM_RET_EMU] = (uint8_t)MOD_PD_STATE_OFF,
    [PPU_V1_MODE_LOGIC_RET]   = (uint8_t)MOD_PD_STATE_ON,
    [PPU_V1_MODE_FULL_RET]    = (uint8_t)MOD_PD_STATE_ON,
    [PPU_V1_MODE_MEM_OFF]     = (uint8_t)MOD_PD_STATE_ON,
    [PPU_V1_MODE_FUNC_RET]    = (uint8_t)MOD_PD_STATE_ON,
    [PPU_V1_MODE_ON]          = (uint8_t)MOD_PD_STATE_ON,
    [PPU_V1_MODE_WARM_RST]    = (uint8_t)MODE_UNSUPPORTED,
    [PPU_V1_MODE_DBG_RECOV]   = (uint8_t)MODE_UNSUPPORTED
};

/*
 * Functions not specific to any type of power domain
 */

static int get_state(struct ppu_v1_reg *ppu, unsigned int *state)
{
    enum ppu_v1_mode mode;

    /* Ensure ppu_to_pd_state_v1 has an entry for each PPU state */
    static_assert((FWK_ARRAY_SIZE(ppu_mode_to_power_state) ==
        PPU_V1_MODE_COUNT), "[PPU_V1] ppu_mode_to_power_state size error");

    mode = ppu_v1_get_power_mode(ppu);
    assert(mode < PPU_V1_MODE_COUNT);

    *state = ppu_mode_to_power_state[mode];

    if ((*state == MOD_PD_STATE_OFF) && (ppu_v1_is_dynamic_enabled(ppu)))
        *state = MOD_PD_STATE_SLEEP;

    if (*state == MODE_UNSUPPORTED) {
        FWK_LOG_ERR("[PPU_V1] Unexpected PPU mode (%i).", mode);
        return FWK_E_DEVICE;
    }

    return FWK_SUCCESS;
}

static int ppu_v1_pd_set_state(fwk_id_t pd_id, unsigned int state)
{
    int status;
    struct ppu_v1_pd_ctx *pd_ctx;

    pd_ctx = ppu_v1_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);

    switch (state) {
    case MOD_PD_STATE_ON:
        ppu_v1_set_power_mode(pd_ctx->ppu, PPU_V1_MODE_ON);
        status = pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_ON);
        assert(status == FWK_SUCCESS);
        break;

    case MOD_PD_STATE_OFF:
        ppu_v1_set_power_mode(pd_ctx->ppu, PPU_V1_MODE_OFF);
        status = pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_OFF);
        assert(status == FWK_SUCCESS);
        break;

    default:
        FWK_LOG_ERR("[PD] Requested power state (%i) is not supported.", state);
        return FWK_E_PARAM;
    }

    return status;
}

static int ppu_v1_pd_get_state(fwk_id_t pd_id, unsigned int *state)
{
    struct ppu_v1_pd_ctx *pd_ctx;

    pd_ctx = ppu_v1_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);

    return get_state(pd_ctx->ppu, state);
}

static int ppu_v1_pd_reset(fwk_id_t pd_id)
{
    int status;
    struct ppu_v1_pd_ctx *pd_ctx;

    pd_ctx = ppu_v1_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);

    /* Model does not support warm reset at the moment. Using OFF instead. */
    status = ppu_v1_set_power_mode(pd_ctx->ppu, PPU_V1_MODE_OFF);
    if (status == FWK_SUCCESS)
        status = ppu_v1_set_power_mode(pd_ctx->ppu, PPU_V1_MODE_ON);

    return status;
}

static int ppu_v1_pd_shutdown(fwk_id_t core_pd_id,
    enum mod_pd_system_shutdown system_shutdown)
{
    return FWK_SUCCESS;
}

static const struct mod_pd_driver_api pd_driver = {
    .set_state = ppu_v1_pd_set_state,
    .get_state = ppu_v1_pd_get_state,
    .reset = ppu_v1_pd_reset,
    .shutdown = ppu_v1_pd_shutdown,
};

/*
 * Functions specific to core power domains
 */
static int ppu_v1_core_pd_init(struct ppu_v1_pd_ctx *pd_ctx)
{
    int status;
    struct ppu_v1_reg *ppu = pd_ctx->ppu;
    unsigned int state;

    ppu_v1_init(ppu);

    status = get_state(ppu, &state);
    if (status != FWK_SUCCESS)
        return status;

    if (state == MOD_PD_STATE_ON) {
        ppu_v1_interrupt_unmask(ppu, PPU_V1_IMR_DYN_POLICY_MIN_IRQ_MASK);
        ppu_v1_dynamic_enable(ppu, PPU_V1_MODE_OFF);
    }

    return FWK_SUCCESS;
}

static int ppu_v1_core_pd_set_state(fwk_id_t core_pd_id, unsigned int state)
{
    int status;
    struct ppu_v1_pd_ctx *pd_ctx;
    struct ppu_v1_reg *ppu;

    pd_ctx = ppu_v1_ctx.pd_ctx_table + fwk_id_get_element_idx(core_pd_id);
    ppu = pd_ctx->ppu;

    switch (state) {
    case MOD_PD_STATE_OFF:
        ppu_v1_set_input_edge_sensitivity(ppu,
                                          PPU_V1_MODE_ON,
                                          PPU_V1_EDGE_SENSITIVITY_MASKED);
        ppu_v1_interrupt_mask(ppu, PPU_V1_IMR_DYN_POLICY_MIN_IRQ_MASK);
        ppu_v1_set_power_mode(ppu, PPU_V1_MODE_OFF);
        ppu_v1_lock_off_disable(ppu);
        ppu_v1_off_unlock(ppu);
        status = pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_OFF);
        assert(status == FWK_SUCCESS);
        break;

    case MOD_PD_STATE_ON:
        ppu_v1_interrupt_unmask(ppu, PPU_V1_IMR_DYN_POLICY_MIN_IRQ_MASK);
        ppu_v1_set_input_edge_sensitivity(ppu,
                                          PPU_V1_MODE_ON,
                                          PPU_V1_EDGE_SENSITIVITY_MASKED);
        ppu_v1_set_power_mode(ppu, PPU_V1_MODE_ON);
        ppu_v1_dynamic_enable(ppu, PPU_V1_MODE_OFF);
        status = pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_ON);
        assert(status == FWK_SUCCESS);
        break;

    case MOD_PD_STATE_SLEEP:
        /*
         * If the dynamic transitions have been enabled then the core is
         * already in the SLEEP power state or will transit to the SLEEP power
         * state if the appropriate processing is done on AP side. Thus nothing
         * to do in that case. If the dynamic transitions are not enabled then
         * this is an OFF to SLEEP transition.
         */
        if (!ppu_v1_is_dynamic_enabled(ppu)) {
            ppu_v1_dynamic_enable(ppu, PPU_V1_MODE_OFF);
            ppu_v1_set_input_edge_sensitivity(ppu,
                                              PPU_V1_MODE_ON,
                                              PPU_V1_EDGE_SENSITIVITY_MASKED);
        }
        status = pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_SLEEP);
        assert(status == FWK_SUCCESS);
        break;

    default:
        FWK_LOG_ERR(
            "[PPU_V1] Requested CPU power state (%i) is not supported!", state);
        return FWK_E_PARAM;
    }

    return status;
}

static int ppu_v1_core_pd_reset(fwk_id_t core_pd_id)
{
    int status;

    status = ppu_v1_core_pd_set_state(core_pd_id, MOD_PD_STATE_OFF);
    if (status == FWK_SUCCESS)
        status = ppu_v1_core_pd_set_state(core_pd_id, MOD_PD_STATE_ON);

    return status;
}

static int ppu_v1_core_pd_prepare_for_system_suspend(fwk_id_t core_pd_id)
{
    struct ppu_v1_pd_ctx *pd_ctx;
    struct ppu_v1_reg *ppu;

    pd_ctx = ppu_v1_ctx.pd_ctx_table + fwk_id_get_element_idx(core_pd_id);
    ppu = pd_ctx->ppu;

    ppu_v1_set_input_edge_sensitivity(ppu,
                                      PPU_V1_MODE_ON,
                                      PPU_V1_EDGE_SENSITIVITY_MASKED);
    ppu_v1_request_power_mode(ppu, PPU_V1_MODE_OFF);

    return FWK_SUCCESS;
}

static void core_pd_ppu_interrupt_handler(struct ppu_v1_pd_ctx *pd_ctx)
{
    int status;
    struct ppu_v1_reg *ppu;

    ppu = pd_ctx->ppu;

    /* ON request interrupt */
    if (ppu_v1_is_power_active_edge_interrupt(ppu, PPU_V1_MODE_ON)) {
        ppu_v1_ack_power_active_edge_interrupt(ppu, PPU_V1_MODE_ON);
        ppu_v1_set_input_edge_sensitivity(ppu,
                                          PPU_V1_MODE_ON,
                                          PPU_V1_EDGE_SENSITIVITY_MASKED);
        ppu_v1_interrupt_unmask(ppu, PPU_V1_IMR_DYN_POLICY_MIN_IRQ_MASK);

        status = pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_ON);
        assert(status == FWK_SUCCESS);
        (void)status;
    /* Minimum policy reached interrupt */
    } else if (ppu_v1_is_dyn_policy_min_interrupt(ppu)) {
        ppu_v1_ack_interrupt(ppu, PPU_V1_ISR_DYN_POLICY_MIN_IRQ);
        ppu_v1_interrupt_mask(ppu, PPU_V1_IMR_DYN_POLICY_MIN_IRQ_MASK);

        status = pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_SLEEP);
        assert(status == FWK_SUCCESS);
        (void)status;

        /*
         * Enable the core PACTIVE ON signal rising edge interrupt then check if
         * the PACTIVE ON signal is high. If it is high, we may have missed the
         * transition from low to high. In that case, just disable the interrupt
         * and acknowledge it in case it is pending. There is no need to send an
         * update request as one has already been queued.
         */
        ppu_v1_set_input_edge_sensitivity(ppu,
                                          PPU_V1_MODE_ON,
                                          PPU_V1_EDGE_SENSITIVITY_RISING_EDGE);
        if (ppu_v1_is_power_devactive_high(ppu, PPU_V1_MODE_ON)) {
            ppu_v1_set_input_edge_sensitivity(ppu,
                                              PPU_V1_MODE_ON,
                                              PPU_V1_EDGE_SENSITIVITY_MASKED);
            ppu_v1_ack_power_active_edge_interrupt(ppu, PPU_V1_MODE_ON);
            ppu_v1_interrupt_unmask(ppu, PPU_V1_IMR_DYN_POLICY_MIN_IRQ_MASK);
        }
    }
}

static const struct mod_pd_driver_api core_pd_driver = {
    .set_state = ppu_v1_core_pd_set_state,
    .get_state = ppu_v1_pd_get_state,
    .reset = ppu_v1_core_pd_reset,
    .prepare_core_for_system_suspend =
        ppu_v1_core_pd_prepare_for_system_suspend,
    .shutdown = ppu_v1_pd_shutdown,
};

/*
 * Functions specific to cluster power domains
 */

static void unlock_all_cores(struct ppu_v1_pd_ctx *pd_ctx)
{
    struct ppu_v1_cluster_pd_ctx *cluster_pd_ctx;
    struct ppu_v1_reg *cpu_ppu;
    unsigned int core_idx;

    assert(pd_ctx != NULL);

    cluster_pd_ctx = pd_ctx->data;

    for (core_idx = 0; core_idx < cluster_pd_ctx->core_count; ++core_idx) {
        cpu_ppu = cluster_pd_ctx->core_pd_ctx_table[core_idx]->ppu;
        ppu_v1_lock_off_disable(cpu_ppu);
        ppu_v1_off_unlock(cpu_ppu);
    }
}

static bool lock_all_dynamic_cores(struct ppu_v1_pd_ctx *pd_ctx)
{
    struct ppu_v1_cluster_pd_ctx *cluster_pd_ctx;
    struct ppu_v1_reg *cpu_ppu;
    unsigned int core_idx;

    assert(pd_ctx != NULL);

    cluster_pd_ctx = pd_ctx->data;

    for (core_idx = 0; core_idx < cluster_pd_ctx->core_count; ++core_idx) {
        cpu_ppu = cluster_pd_ctx->core_pd_ctx_table[core_idx]->ppu;

        if (!ppu_v1_is_dynamic_enabled(cpu_ppu))
            continue;

        ppu_v1_lock_off_enable(cpu_ppu);
        while ((!ppu_v1_is_locked(cpu_ppu)) &&
               (!ppu_v1_is_power_devactive_high(cpu_ppu, PPU_V1_MODE_ON)))
            continue;

        if (ppu_v1_is_power_devactive_high(cpu_ppu, PPU_V1_MODE_ON))
            return false;
    }

    return true;
}

static bool cluster_off(struct ppu_v1_pd_ctx *pd_ctx)
{
    struct ppu_v1_reg *ppu;
    bool lock_successful;

    assert(pd_ctx != NULL);

    ppu = pd_ctx->ppu;

    ppu_v1_set_input_edge_sensitivity(ppu,
                                      PPU_V1_MODE_ON,
                                      PPU_V1_EDGE_SENSITIVITY_MASKED);

    lock_successful = lock_all_dynamic_cores(pd_ctx);
    if (!lock_successful) {
        unlock_all_cores(pd_ctx);
        return false;
    }

    ppu_v1_set_power_mode(ppu, PPU_V1_MODE_OFF);
    return true;
}

static void cluster_on(struct ppu_v1_pd_ctx *pd_ctx)
{
    int status;
    struct ppu_v1_reg *ppu;

    assert(pd_ctx != NULL);

    ppu = pd_ctx->ppu;

    ppu_v1_set_input_edge_sensitivity(ppu,
                                      PPU_V1_MODE_ON,
                                      PPU_V1_EDGE_SENSITIVITY_MASKED);

    ppu_v1_set_power_mode(ppu, PPU_V1_MODE_ON);
    status = pd_ctx->pd_driver_input_api->report_power_state_transition(
        pd_ctx->bound_id, MOD_PD_STATE_ON);
    assert(status == FWK_SUCCESS);
    (void)status;

    if (pd_ctx->observer_api != NULL)
        pd_ctx->observer_api->post_ppu_on(pd_ctx->config->post_ppu_on_param);

    unlock_all_cores(pd_ctx);
}

static int ppu_v1_cluster_pd_init(struct ppu_v1_pd_ctx *pd_ctx)
{
    int status;
    struct ppu_v1_reg *ppu = pd_ctx->ppu;
    unsigned int state;

    ppu_v1_init(ppu);

    status = get_state(ppu, &state);
    if (status != FWK_SUCCESS)
        return status;

    /* For clusters with operating mode support, enable the dynamic support */
    if (ppu_v1_get_num_opmode(ppu) > 1)
        ppu_v1_opmode_dynamic_enable(ppu, PPU_V1_OPMODE_00);

    if (state == MOD_PD_STATE_ON) {
        ppu_v1_set_input_edge_sensitivity(ppu,
                                          PPU_V1_MODE_ON,
                                          PPU_V1_EDGE_SENSITIVITY_FALLING_EDGE);
    }

    return FWK_SUCCESS;
}

static int ppu_v1_cluster_pd_set_state(fwk_id_t cluster_pd_id,
                                       unsigned int state)
{
    int status;
    struct ppu_v1_pd_ctx *pd_ctx;
    struct ppu_v1_reg *ppu;
    (void)ppu;


    pd_ctx = ppu_v1_ctx.pd_ctx_table + fwk_id_get_element_idx(cluster_pd_id);
    ppu = pd_ctx->ppu;

    switch (state) {
    case MOD_PD_STATE_ON:
        cluster_on(pd_ctx);
        #ifdef BUILD_HAS_MULTITHREADING
        ppu_v1_set_input_edge_sensitivity(ppu,
                                          PPU_V1_MODE_ON,
                                          PPU_V1_EDGE_SENSITIVITY_FALLING_EDGE);
        #endif
        return FWK_SUCCESS;

    case MOD_PD_STATE_OFF:
        if (!cluster_off(pd_ctx)) {
            /* Cluster failed to transition to off */
            #ifdef BUILD_HAS_MULTITHREADING
            ppu_v1_set_input_edge_sensitivity(ppu,
                PPU_V1_MODE_ON, PPU_V1_EDGE_SENSITIVITY_FALLING_EDGE);
            #endif
            return FWK_E_STATE;
        }
        status = pd_ctx->pd_driver_input_api->report_power_state_transition(
            pd_ctx->bound_id, MOD_PD_STATE_OFF);
        assert(status == FWK_SUCCESS);
        return status;

    default:
        FWK_LOG_ERR(
            "[PPU_V1] Requested CPU power state (%i) is not supported!", state);
        return FWK_E_PARAM;
    }
}

static void cluster_pd_ppu_interrupt_handler(struct ppu_v1_pd_ctx *pd_ctx)
{
    int status;
    struct ppu_v1_reg *ppu;
    enum ppu_v1_mode current_mode;

    assert(pd_ctx != NULL);

    ppu = pd_ctx->ppu;

    if (!ppu_v1_is_power_active_edge_interrupt(ppu, PPU_V1_MODE_ON))
        return; /* Spurious interrupt */

    ppu_v1_ack_power_active_edge_interrupt(ppu, PPU_V1_MODE_ON);
    current_mode = ppu_v1_get_power_mode(ppu);

    switch (current_mode) {
    case PPU_V1_MODE_OFF:
        /* Cluster has to be powered on */
        cluster_on(pd_ctx);
        ppu_v1_set_input_edge_sensitivity(ppu,
                                          PPU_V1_MODE_ON,
                                          PPU_V1_EDGE_SENSITIVITY_FALLING_EDGE);
        return;

    case PPU_V1_MODE_ON:
        /*
         * It may be possible to turn off the cluster, check all PACTIVE lines
         * to make sure it is not just requesting a low power mode.
         */
        while (current_mode > 0) {
            if (ppu_v1_is_power_devactive_high(ppu, current_mode--))
                return;
        }

        /* All PACTIVE lines are low, so the cluster can be turned off */
        if (cluster_off(pd_ctx)) {
            /* Cluster successfuly transitioned to off */
            ppu_v1_set_input_edge_sensitivity(ppu,
                PPU_V1_MODE_ON, PPU_V1_EDGE_SENSITIVITY_RISING_EDGE);
            status = pd_ctx->pd_driver_input_api->report_power_state_transition(
                pd_ctx->bound_id, MOD_PD_STATE_SLEEP);
            assert(status == FWK_SUCCESS);
            (void)status;
        } else {
            /* Cluster did not transition to off */
            ppu_v1_set_input_edge_sensitivity(ppu,
                PPU_V1_MODE_ON, PPU_V1_EDGE_SENSITIVITY_FALLING_EDGE);
        }
        return;

    default:
        /* Cluster is in an invalid power mode */
        assert(false);
        return;
    }
}

static const struct mod_pd_driver_api cluster_pd_driver = {
    .set_state = ppu_v1_cluster_pd_set_state,
    .get_state = ppu_v1_pd_get_state,
    .reset = ppu_v1_pd_reset,
    .shutdown = ppu_v1_pd_shutdown,
};

static void ppu_interrupt_handler(uintptr_t pd_ctx_param)
{
    struct ppu_v1_pd_ctx *pd_ctx = (struct ppu_v1_pd_ctx *)pd_ctx_param;

    assert(pd_ctx != NULL);

    if (pd_ctx->config->pd_type == MOD_PD_TYPE_CORE)
        core_pd_ppu_interrupt_handler(pd_ctx);
    else
        cluster_pd_ppu_interrupt_handler(pd_ctx);
}

static void ppu_isr_api_interrupt_handler(fwk_id_t pd_id)
{
    struct ppu_v1_pd_ctx *pd_ctx;

    if (!fwk_id_is_type(pd_id, FWK_ID_TYPE_ELEMENT))
        return;

    pd_ctx = ppu_v1_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);
    ppu_interrupt_handler((uintptr_t)pd_ctx);
}

static const struct ppu_v1_isr_api isr_api = {
    .ppu_interrupt_handler = ppu_isr_api_interrupt_handler,
};

static int ppu_power_mode_on(fwk_id_t pd_id)
{
    struct ppu_v1_pd_ctx *pd_ctx;

    if (!fwk_id_is_type(pd_id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_PARAM;

    pd_ctx = ppu_v1_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);

    return ppu_v1_set_power_mode(pd_ctx->ppu, PPU_V1_MODE_ON);
}

static const struct ppu_v1_boot_api boot_api = {
    .power_mode_on = ppu_power_mode_on,
};

/*
 * Framework handlers
 */

static int ppu_v1_mod_init(fwk_id_t module_id, unsigned int pd_count,
                           const void *unused)
{
    ppu_v1_ctx.pd_ctx_table = fwk_mm_calloc(pd_count,
                                            sizeof(struct ppu_v1_pd_ctx));

    ppu_v1_ctx.pd_ctx_table_size = pd_count;

    return FWK_SUCCESS;
}

static int ppu_v1_pd_init(fwk_id_t pd_id, unsigned int unused, const void *data)
{
    const struct mod_ppu_v1_pd_config *config = data;
    struct ppu_v1_pd_ctx *pd_ctx;

    if (config->pd_type >= MOD_PD_TYPE_COUNT)
        return FWK_E_DATA;

    pd_ctx = ppu_v1_ctx.pd_ctx_table + fwk_id_get_element_idx(pd_id);
    pd_ctx->config = config;
    pd_ctx->ppu = (struct ppu_v1_reg *)(config->ppu.reg_base);
    pd_ctx->bound_id = FWK_ID_NONE;

    if (config->ppu.irq != FWK_INTERRUPT_NONE) {
        fwk_interrupt_set_isr_param(config->ppu.irq,
                                    ppu_interrupt_handler,
                                    (uintptr_t)pd_ctx);
    }

    if (config->pd_type == MOD_PD_TYPE_CLUSTER) {
        pd_ctx->data = fwk_mm_calloc(1, sizeof(struct ppu_v1_cluster_pd_ctx));
    }

    if (config->default_power_on) {
        switch (config->pd_type) {
        case MOD_PD_TYPE_DEVICE:
            /* Fall through */
        case MOD_PD_TYPE_DEVICE_DEBUG:
            /* Fall through */
        case MOD_PD_TYPE_SYSTEM:
            ppu_v1_init(pd_ctx->ppu);
            return ppu_v1_set_power_mode(pd_ctx->ppu, PPU_V1_MODE_ON);

        default:
            assert(false);
            return FWK_E_SUPPORT;
        }
    }

    return FWK_SUCCESS;
}

static int ppu_v1_post_init(fwk_id_t module_id)
{
    unsigned int pd_idx;
    struct ppu_v1_pd_ctx *pd_ctx, *cluster_pd_ctx;
    const struct mod_ppu_v1_pd_config *config;
    fwk_id_t cluster_id;
    struct ppu_v1_cluster_pd_ctx *cluster_pd_specific_ctx;

    for (pd_idx = 0; pd_idx < ppu_v1_ctx.pd_ctx_table_size; pd_idx++) {
        pd_ctx = &ppu_v1_ctx.pd_ctx_table[pd_idx];
        config = pd_ctx->config;
        if (config->pd_type != MOD_PD_TYPE_CORE)
            continue;

        cluster_id = config->cluster_id;

        if ((!fwk_module_is_valid_element_id(cluster_id)) ||
            (fwk_id_get_module_idx(cluster_id) != FWK_MODULE_IDX_PPU_V1))
            return FWK_E_PARAM;

        cluster_pd_ctx = &ppu_v1_ctx.pd_ctx_table[
            fwk_id_get_element_idx(cluster_id)];
        cluster_pd_specific_ctx = cluster_pd_ctx->data;

       if (cluster_pd_specific_ctx->core_count >= CORE_PER_CLUSTER_COUNT_MAX)
           return FWK_E_NOMEM;

       cluster_pd_specific_ctx->core_pd_ctx_table[
           cluster_pd_specific_ctx->core_count++] = pd_ctx;
       pd_ctx->parent_pd_ctx = cluster_pd_ctx;
    }

    return FWK_SUCCESS;
}

static int ppu_v1_bind(fwk_id_t id, unsigned int round)
{
    int status = FWK_SUCCESS;
    struct ppu_v1_pd_ctx *pd_ctx;

    /* Nothing to do during the first round of calls where the power module
       will bind to the power domains of this module. */
    if (round == 0)
        return FWK_SUCCESS;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    pd_ctx = ppu_v1_ctx.pd_ctx_table + fwk_id_get_element_idx(id);

    if (!fwk_id_is_equal(pd_ctx->config->observer_id, FWK_ID_NONE)) {
        if (pd_ctx->config->pd_type != MOD_PD_TYPE_CLUSTER) {
            /* State observation only supported for clusters */
            assert(false);
            return FWK_E_SUPPORT;
        }

        status = fwk_module_bind(pd_ctx->config->observer_id,
                                 pd_ctx->config->observer_api,
                                 &pd_ctx->observer_api);
        if (status != FWK_SUCCESS)
            return status;
    }

    if (fwk_id_is_equal(pd_ctx->bound_id, FWK_ID_NONE))
        return FWK_SUCCESS;

    switch (fwk_id_get_module_idx(pd_ctx->bound_id)) {
    #if BUILD_HAS_MOD_POWER_DOMAIN
    case FWK_MODULE_IDX_POWER_DOMAIN:
        return fwk_module_bind(pd_ctx->bound_id,
                               mod_pd_api_id_driver_input,
                               &pd_ctx->pd_driver_input_api);
        break;
    #endif

    #if BUILD_HAS_MOD_SYSTEM_POWER
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

static int ppu_v1_process_bind_request(fwk_id_t source_id,
                                       fwk_id_t target_id, fwk_id_t api_id,
                                       const void **api)
{
    struct ppu_v1_pd_ctx *pd_ctx;
    unsigned int api_idx;
    bool is_power_domain_module = false;
    bool is_system_power_module = false;

    api_idx = fwk_id_get_api_idx(api_id);

    if (api_idx == MOD_PPU_V1_API_IDX_ISR) {
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_MODULE))
            return FWK_E_SUPPORT;

        *api = &isr_api;
        return FWK_SUCCESS;
    }

    if (api_idx == MOD_PPU_V1_API_IDX_BOOT) {
        *api = &boot_api;
        return FWK_SUCCESS;
    }

    if (api_idx != MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER)
        return FWK_E_SUPPORT;

    if (!fwk_module_is_valid_element_id(target_id))
        return FWK_E_PARAM;

    pd_ctx = ppu_v1_ctx.pd_ctx_table + fwk_id_get_element_idx(target_id);

    /* Allow multiple binding only for device power domain for now */
    if ((pd_ctx->config->pd_type != MOD_PD_TYPE_DEVICE) &&
        (!fwk_id_is_equal(pd_ctx->bound_id, FWK_ID_NONE))) {
        assert(false);
        return FWK_E_ACCESS;
    }

    #if BUILD_HAS_MOD_POWER_DOMAIN
    is_power_domain_module = (fwk_id_get_module_idx(source_id) ==
        FWK_MODULE_IDX_POWER_DOMAIN);
    #endif
    #if BUILD_HAS_MOD_SYSTEM_POWER
    is_system_power_module = (fwk_id_get_module_idx(source_id) ==
        FWK_MODULE_IDX_SYSTEM_POWER);
    #endif

    switch (pd_ctx->config->pd_type) {
    case MOD_PD_TYPE_CORE:
        if (is_power_domain_module) {
            *api = &core_pd_driver;
            pd_ctx->bound_id = source_id;
            return FWK_SUCCESS;
        }
        break;

    case MOD_PD_TYPE_CLUSTER:
        if (is_power_domain_module) {
            *api = &cluster_pd_driver;
            pd_ctx->bound_id = source_id;
            return FWK_SUCCESS;
        }
        break;

    case MOD_PD_TYPE_SYSTEM:
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

static int ppu_v1_start(fwk_id_t id)
{
    int status;
    struct ppu_v1_pd_ctx *pd_ctx;
    const struct mod_ppu_v1_config *module_config;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return FWK_SUCCESS;

    pd_ctx = ppu_v1_ctx.pd_ctx_table + fwk_id_get_element_idx(id);
    module_config = fwk_module_get_data(fwk_id_build_module_id(id));
    assert(module_config != NULL);

    /* Register for power domain transition notifications */
    status = fwk_notification_subscribe(
        module_config->pd_notification_id,
        module_config->pd_source_id,
        id);
    if (status != FWK_SUCCESS)
        return status;

    switch (pd_ctx->config->pd_type) {
    case MOD_PD_TYPE_CORE:
    case MOD_PD_TYPE_CLUSTER:
        fwk_interrupt_clear_pending(pd_ctx->config->ppu.irq);
        fwk_interrupt_enable(pd_ctx->config->ppu.irq);
        break;
    default:
        /* Nothing to be done for other types */
        break;
    }

    return FWK_SUCCESS;
}

static int ppu_v1_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    const struct mod_ppu_v1_config *module_config;
    struct ppu_v1_pd_ctx *pd_ctx;
    struct mod_pd_power_state_transition_notification_params *params;

    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));
    module_config =
        fwk_module_get_data(fwk_id_build_module_id(event->target_id));
    assert(
        fwk_id_is_equal(
            event->id,
            module_config->pd_notification_id));
    (void)module_config;

    params = (struct mod_pd_power_state_transition_notification_params *)
        event->params;

    if (params->state != MOD_PD_STATE_ON)
        return FWK_SUCCESS;

    pd_ctx = ppu_v1_ctx.pd_ctx_table + fwk_id_get_element_idx(event->target_id);

    switch (pd_ctx->config->pd_type) {
    case MOD_PD_TYPE_CORE:
        return ppu_v1_core_pd_init(pd_ctx);

    case MOD_PD_TYPE_CLUSTER:
        return ppu_v1_cluster_pd_init(pd_ctx);

    default:
        ppu_v1_init(pd_ctx->ppu);
        return FWK_SUCCESS;
    }
}

const struct fwk_module module_ppu_v1 = {
    .name = "PPU_V1",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_PPU_V1_API_IDX_COUNT,
    .init = ppu_v1_mod_init,
    .element_init = ppu_v1_pd_init,
    .post_init = ppu_v1_post_init,
    .bind = ppu_v1_bind,
    .start = ppu_v1_start,
    .process_bind_request = ppu_v1_process_bind_request,
    .process_notification = ppu_v1_process_notification,
};

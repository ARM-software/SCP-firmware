/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     DWT driver implementation which implements PMI interface
 *     defined in mod_pmi.h
 */

#include <mod_dwt_pmi.h>

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <string.h>

struct cntbase_reg {
    FWK_R uint32_t PCTL;
    FWK_R uint32_t PCTH;
};

/*
 * Enable DWT and ITM through DEMCR register
 * See Debug system register in ARMv7-M Architecture Reference Manual
 * C1.6.5 Debug Exception and Monitor Control Register, DEMCR
 */
#define DEBUG_SYSTEM_DEMCR_TRCENA (UINT32_C(1) << 24U)

/*
 * Enables CYCCNT in Control register, DWT_CTRL
 * See ARMv7-M Architecture Reference Manual
 */
#define DWT_CTRL_CYCCNTENA (UINT32_C(1) << 0U)

/*
 * Flag to check if cycle count register is enabled
 * in Control register, DWT_CTRL
 * See  ARMv7-M Architecture Reference Manual.
 */
#define DWT_CTRL_NOCYCCNT (UINT32_C(1) << 25U)

static uint32_t cycle_diff(uint64_t cstart, uint64_t cend)
{
    return (0xFFFFFFFF - (uint32_t)(cstart) + (uint32_t)(cend));
}

struct mod_dwt_pmi_ctx {
    /* Platform specific DWT configuration data */
    const struct mod_dwt_pmi_config *config;
    bool dwt_enabled;
    bool dwt_cyccnt_enable;
};

static struct mod_dwt_pmi_ctx ctx;

static bool check_cyccnt_supported(void)
{
    uint32_t val = *(ctx.config->dwt_ctrl_addr);

    /* Bit NOCYCCNT is set if no cycle count is supported */
    if ((DWT_CTRL_NOCYCCNT & val) == 0) {
        return true;
    }

    return false;
}

static void enable_dwt(void)
{
    if (!ctx.dwt_enabled) {
        *(ctx.config->debug_sys_demcr_addr) |= DEBUG_SYSTEM_DEMCR_TRCENA;
        ctx.dwt_enabled = true;
    }
}

static int enable_cycle_count(void)
{
    bool supported;

    supported = check_cyccnt_supported();
    /* If DWT_CYCNT feature is not available then do not enable cycle count */
    if (!supported) {
        return FWK_E_DEVICE;
    }

    if (!ctx.dwt_cyccnt_enable) {
        *(ctx.config->dwt_ctrl_addr) |= DWT_CTRL_CYCCNTENA;
        ctx.dwt_cyccnt_enable = true;
    }

    return FWK_SUCCESS;
}

static int disable_cycle_count(void)
{
    if (ctx.dwt_cyccnt_enable) {
        *(ctx.config->dwt_ctrl_addr) &= ~DWT_CTRL_CYCCNTENA;
        ctx.dwt_cyccnt_enable = false;
    }

    return FWK_SUCCESS;
}

static int get_dwt_cycle_count(uint64_t *cycle_count)
{
    if (!ctx.dwt_cyccnt_enable) {
        return FWK_E_DEVICE;
    }

    *cycle_count = (uint64_t)(*(ctx.config->dwt_cyccnt));

    return FWK_SUCCESS;
}

static int set_dwt_cycle_count(uint64_t cycle_count)
{
    if (!ctx.dwt_cyccnt_enable) {
        return FWK_E_DEVICE;
    }

    /* DWT cycle count register support only 32 bit count */
    *(ctx.config->dwt_cyccnt) = (uint32_t)cycle_count;

    return FWK_SUCCESS;
}

static uint64_t cycle_count_diff(uint64_t start, uint64_t end)
{
    /* Handle wrap codition */
    if (end < start) {
        return (uint64_t)cycle_diff(start, end);
    }

    return end - start;
}

/*
 * This function does not use DWT for getting the time,
 * instead it uses generic timer available on ARM platforms.
 * However for completeness, the PMI driver provides this
 * function. For more time related use cases, See fwk_time.h
 * and mod_gtimer.h. In the absence of generic timer, platform
 * owners should try to implement this function in their respective
 * PMI driver.
 */
static uint64_t get_current_time(void)
{
    uint32_t counter_low;
    uint32_t counter_high;

    /*
     * To avoid race conditions where the high half of the counter increments
     * after it has been sampled but before the low half is sampled, the values
     * are resampled until the high half has stabilized. This assumes that the
     * loop is faster than the high half incrementation.
     */

    do {
        counter_high = ctx.config->hw_timer->PCTH;
        counter_low = ctx.config->hw_timer->PCTL;
    } while (counter_high != ctx.config->hw_timer->PCTH);

    return ((uint64_t)counter_high << 32U) | counter_low;
}

static const struct mod_pmi_driver_api dwt_pmi_driver_api = {
    .start_cycle_count = enable_cycle_count,
    .stop_cycle_count = disable_cycle_count,
    .get_cycle_count = get_dwt_cycle_count,
    .set_cycle_count = set_dwt_cycle_count,
    .cycle_count_diff = cycle_count_diff,
    .get_current_time = get_current_time
};

static int dwt_pmi_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    ctx.config = (struct mod_dwt_pmi_config *)data;
    int status;

    enable_dwt();

    /* Start cycle count with 0 */
    set_dwt_cycle_count(0);

    status = enable_cycle_count();
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

static int process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    /* Only allow binding to the module */
    if (!fwk_id_is_equal(target_id, fwk_module_id_dwt_pmi)) {
        return FWK_E_PARAM;
    }

    *api = &dwt_pmi_driver_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_dwt_pmi = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = dwt_pmi_init,
    .process_bind_request = process_bind_request,
    .api_count = MOD_DWT_PMI_API_IDX_COUNT,
};

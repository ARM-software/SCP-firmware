/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <lcp_device.h>

#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#define MOD_NAME "[LCP_PLATFORM] "
#define LCP_TIMER_RELOAD 0xFFFFFU

volatile struct {
    uint32_t counter;
} lcp_platform_ctx;

void timer_isr()
{
    lcp_platform_ctx.counter++;
    /* clear interrupt flag */
    LCP_TIMER_REG_S->INTSTATUS = 1U;
}

void mod_lcp_config_timer()
{
    LCP_TIMER_REG_S->CTRL = CTRL_ENABLE_MASK | CTRL_INT_EN_MASK;

    LCP_TIMER_REG_S->RELOAD = LCP_TIMER_RELOAD;
}

static int mod_lcp_platform_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{
    /* No elements support */
    if (element_count > 0) {
        return FWK_E_DATA;
    }

    return FWK_SUCCESS;
}

static int mod_lcp_platform_start(fwk_id_t id)
{
    fwk_interrupt_set_isr(TIMER_IRQ, timer_isr);

    fwk_interrupt_enable(TIMER_IRQ);

    mod_lcp_config_timer();

    FWK_LOG_INFO(MOD_NAME "LCP RAM firmware initialized");

    return FWK_SUCCESS;
}

const struct fwk_module module_lcp_platform = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .init = mod_lcp_platform_init,
    .start = mod_lcp_platform_start,
};

const struct fwk_module_config config_lcp_platform = { 0 };

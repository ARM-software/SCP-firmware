/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_irq.h"
#include "juno_ppu_idx.h"
#include "juno_utils.h"
#include "juno_wdog_rom.h"
#include "scp_config.h"

#include <mod_juno_ppu.h>
#include <mod_power_domain.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>

/* Platform default dividers for debug clocks */
#define JUNO_DEBUG_ROM_DIVIDER_ATCLK        16
#define JUNO_DEBUG_ROM_DIVIDER_TRACECLK     16
#define JUNO_DEBUG_ROM_DIVIDER_PCLK         16

const struct mod_juno_ppu_rom_api *ppu_api;

static const fwk_id_t dbgsys_ppu_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PPU, JUNO_PPU_DEV_IDX_DBGSYS);

static const fwk_id_t big_sstop_ppu_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PPU, JUNO_PPU_DEV_IDX_BIG_SSTOP);

static const fwk_id_t little_sstop_ppu_id =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PPU, JUNO_PPU_DEV_IDX_LITTLE_SSTOP);

static const uint32_t debug_clk_mask = (SCP_CONFIG_CLOCK_ENABLE_PCLKDBGEN |
                                        SCP_CONFIG_CLOCK_ENABLE_TRACECLKINEN |
                                        SCP_CONFIG_CLOCK_ENABLE_ATCLKEN);

/*
 * Static helpers
 */

static void juno_debug_rom_set_clocks(void)
{
    /*
     * Setup ATCLK
     */
    juno_utils_atclk_clock_div_set(JUNO_DEBUG_ROM_DIVIDER_ATCLK);
    while (!juno_utils_atclk_clock_div_set_check(JUNO_DEBUG_ROM_DIVIDER_ATCLK))
        continue;

    juno_utils_atclk_clock_sel_set(SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
    while (!juno_utils_atclk_clock_sel_set_check(
        SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK))
        continue;

    /*
     * Setup TRACECLKIN
     */
    juno_utils_traceclk_clock_div_set(JUNO_DEBUG_ROM_DIVIDER_TRACECLK);
    while (!juno_utils_traceclk_clock_div_set_check(
        JUNO_DEBUG_ROM_DIVIDER_TRACECLK))
        continue;

    juno_utils_traceclk_clock_sel_set(
        SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK);
    while (!juno_utils_traceclk_clock_sel_set_check(
        SCP_CONFIG_STDCLK_CONTROL_CLKSEL_SYSINCLK))
        continue;

    /*
     * Setup PCLKDBG
     */
    juno_utils_pclkdbg_clock_div_set(JUNO_DEBUG_ROM_DIVIDER_PCLK);
    while (!juno_utils_pclkdbg_clock_div_set_check(JUNO_DEBUG_ROM_DIVIDER_PCLK))
        continue;
}

/*
 * Interrupt service routines
 */

static void juno_debug_cdbg_pwr_up_req_isr(void)
{
    int status;

    #ifndef BUILD_MODE_DEBUG
    juno_wdog_rom_reload();
    #endif

    if (SCP_CONFIG->DEBUG_STATUS & SCP_CONFIG_DEBUG_STATUS_CDBGPWRUPREQ) {
        #ifndef BUILD_MODE_DEBUG
        juno_wdog_rom_halt_on_debug_enable(true);
        #endif

        juno_utils_system_clock_enable(debug_clk_mask);

        /* Turn on DBGSYS */
        status = ppu_api->set_state_and_wait(dbgsys_ppu_id, MOD_PD_STATE_ON);
        fwk_assert(status == FWK_SUCCESS);

        status = ppu_api->set_state_and_wait(big_sstop_ppu_id, MOD_PD_STATE_ON);
        fwk_assert(status == FWK_SUCCESS);

        status =
            ppu_api->set_state_and_wait(little_sstop_ppu_id, MOD_PD_STATE_ON);
        fwk_assert(status == FWK_SUCCESS);

        juno_debug_rom_set_clocks();

        SCP_CONFIG->SYS_MANUAL_RESET.SET =
            SCP_CONFIG_SYS_MANUAL_RESET_DBGSYSRESET;
        SCP_CONFIG->SYS_MANUAL_RESET.CLEAR =
            SCP_CONFIG_SYS_MANUAL_RESET_DBGSYSRESET;

        /* Acknowledge end of debug power up */
        SCP_CONFIG->DEBUG_CONTROL |= SCP_CONFIG_DEBUG_CONTROL_CDBGPWRUPACK;

    } else {
        status = ppu_api->set_state_and_wait(dbgsys_ppu_id, MOD_PD_STATE_OFF);
        fwk_assert(status == FWK_SUCCESS);

        SCP_CONFIG->DEBUG_CONTROL &= ~SCP_CONFIG_DEBUG_CONTROL_CDBGPWRUPACK;

        #ifndef BUILD_MODE_DEBUG
        juno_wdog_rom_halt_on_debug_enable(false);
        #endif
    }
}

static void juno_debug_csys_pwr_up_req_isr(void)
{
    int status;

    #ifndef BUILD_MODE_DEBUG
    juno_wdog_rom_reload();
    #endif

    if (SCP_CONFIG->DEBUG_STATUS & SCP_CONFIG_DEBUG_STATUS_CSYSPWRUPREQ) {
        juno_utils_system_clock_enable(debug_clk_mask);

        status = ppu_api->set_state_and_wait(dbgsys_ppu_id, MOD_PD_STATE_ON);
        fwk_assert(status == FWK_SUCCESS);

        juno_debug_rom_set_clocks();

        SCP_CONFIG->DEBUG_CONTROL |= SCP_CONFIG_DEBUG_CONTROL_CSYSPWRUPACK;
    } else {
        status = ppu_api->set_state_and_wait(dbgsys_ppu_id, MOD_PD_STATE_OFF);
        fwk_assert(status == FWK_SUCCESS);

        SCP_CONFIG->DEBUG_CONTROL &= ~SCP_CONFIG_DEBUG_CONTROL_CSYSPWRUPACK;
    }
}

static void juno_debug_cdbg_rst_req_isr(void)
{
    #ifndef BUILD_MODE_DEBUG
    juno_wdog_rom_reload();
    #endif

    /* Reset debug system */
    SCP_CONFIG->SYS_MANUAL_RESET.SET = SCP_CONFIG_SYS_MANUAL_RESET_DBGSYSRESET;

    /* Acknowledge request */
    SCP_CONFIG->DEBUG_CONTROL |= SCP_CONFIG_DEBUG_CONTROL_CDBGRSTACK;

    /* Wait for CoreSight to de-assert request */
    while ((SCP_CONFIG->DEBUG_STATUS & SCP_CONFIG_DEBUG_STATUS_CDBGRSTREQ) ==
        SCP_CONFIG_DEBUG_STATUS_CDBGRSTREQ)
        continue;

    /* De-assert reset */
    SCP_CONFIG->SYS_MANUAL_RESET.SET = SCP_CONFIG_SYS_MANUAL_RESET_DBGSYSRESET;

    /* De-assert ACK */
    SCP_CONFIG->DEBUG_CONTROL &= ~SCP_CONFIG_DEBUG_CONTROL_CDBGRSTACK;
}

/*
 * Public interface
 */

int juno_debug_rom_init(const struct mod_juno_ppu_rom_api *rom_ppu_api)
{
    int status;
    ppu_api = rom_ppu_api;

    /*
     * IRQs are not yet enabled but there may be a pending debug power-up
     * request that was raised before SCP was released from reset
     */
    if (SCP_CONFIG->DEBUG_STATUS & SCP_CONFIG_DEBUG_STATUS_CDBGPWRUPREQ) {
        juno_debug_cdbg_pwr_up_req_isr();

        fwk_assert((SCP_CONFIG->DEBUG_CONTROL &
            SCP_CONFIG_DEBUG_CONTROL_CDBGPWRUPACK) != 0);
    }

    fwk_interrupt_clear_pending(CDBG_PWR_UP_REQ_IRQ);
    fwk_interrupt_clear_pending(CDBG_RST_REQ_IRQ);
    fwk_interrupt_clear_pending(CSYS_PWR_UP_REQ_IRQ);

    status = fwk_interrupt_set_isr(CDBG_PWR_UP_REQ_IRQ,
        juno_debug_cdbg_pwr_up_req_isr);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_interrupt_set_isr(CSYS_PWR_UP_REQ_IRQ,
        juno_debug_csys_pwr_up_req_isr);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_interrupt_set_isr(CDBG_RST_REQ_IRQ,
        juno_debug_cdbg_rst_req_isr);
    if (status != FWK_SUCCESS)
        return status;

    fwk_interrupt_enable(CDBG_PWR_UP_REQ_IRQ);
    fwk_interrupt_enable(CDBG_RST_REQ_IRQ);
    fwk_interrupt_enable(CSYS_PWR_UP_REQ_IRQ);

    return FWK_SUCCESS;
}

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Clock driver for the clocks of the Juno SoC
 */

#include "juno_id.h"
#include "juno_scc.h"
#include "scp_config.h"
#include "system_clock.h"

#include <mod_juno_soc_clock.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>

/*
 * Masks for clock control registers
 */
#define CLKSEL_MASK           UINT32_C(0x0000000F)
#define CLKDIV_MASK           UINT32_C(0x000000F0)
#define CRNTCLK_MASK          UINT32_C(0x00000F00)
#define CRNTCLKDIV_MASK       UINT32_C(0x0000F000)
#define LPI_ENTRY_DELAY_MASK  UINT32_C(0x00FF0000)

/*
 * Offsets for clock control registers
 */
#define CLKSEL_POS            0
#define CLKDIV_POS            4
#define CRNTCLK_POS           8
#define CRNTCLKDIV_POS        12
#define LPI_ENTRY_DELAY_POS   16

/*
 * Values of the CLKSEL field
 */
#define CLKSEL_GATED   0
#define CLKSEL_REFCLK  1
#define CLKSEL_SYSCLK  2

/*
 * Clock values
 */
#define CCICLK_CONTROL_CLOCK        (533 * FWK_MHZ)
#define NICSCPCLK_CONTROL_CLOCK     (100 * FWK_MHZ)
#define NICPERCLK_CONTROL_CLOCK     (266 * FWK_MHZ)
#define SPCLK_CONTROL_CLOCK         (533 * FWK_MHZ)
#define GICCLK_CONTROL_CLOCK        (533 * FWK_MHZ)
#define ATCLK_CONTROL_CLOCK_REV0    (533 * FWK_MHZ)
#define ATCLK_CONTROL_CLOCK         (400 * FWK_MHZ)
#define TRACECLKIN_CONTROL_CLOCK    (150 * FWK_MHZ)
#define SAXICLK_CLOCK               (266 * FWK_MHZ)
#define FAXICLK_CLOCK               (400 * FWK_MHZ)
#define HDLCDCLK_CLOCK              (400 * FWK_MHZ)

/*
 * Static helper functions
 */

static void pll_init(void)
{
    unsigned int pll_idx;
    const uint32_t pll_mask = SCP_CONFIG_SCP_STATUS_SYSINPLLLOCK |
                              SCP_CONFIG_SCP_STATUS_GPUINPLLLOCK |
                              SCP_CONFIG_SCP_STATUS_LITTLEINPLLLOCK |
                              SCP_CONFIG_SCP_STATUS_BIGINPLLLOCK;

    /* Release All system PLLs from reset */
    for (pll_idx = 0; pll_idx < PLL_IDX_COUNT; pll_idx++)
        SCC->PLL[pll_idx].REG0 &= ~PLL_REG0_PLL_RESET;

    /* Wait for PLLs to lock */
    while ((SCP_CONFIG->SCP_STATUS & pll_mask) != pll_mask)
        continue;
}

static void set_div(volatile uint32_t * const control_reg, uint32_t divider)
{
    fwk_assert(control_reg != NULL);
    fwk_assert(divider != 0);
    fwk_assert(divider <= 16);

    *control_reg = (*control_reg & ~CLKDIV_MASK) |
                   ((divider - 1) << CLKDIV_POS);
}

static void set_source(volatile uint32_t * const control_reg,
                       unsigned int source)
{
    fwk_assert(control_reg != NULL);
    fwk_assert(source <= CLKSEL_SYSCLK);

    *control_reg = (*control_reg & ~CLKSEL_MASK) | (source << CLKSEL_POS);
}

/*
 * Framework handler functions
 */

static int juno_soc_clock_init(fwk_id_t module_id, unsigned int clock_count,
                               const void *unused)
{
    int status;
    enum juno_idx_revision revision;

    status = juno_id_get_revision(&revision);
    if (!fwk_expect(status == FWK_SUCCESS))
        return status;

    pll_init();

    SCP_CONFIG->CLOCK_STOPPED_CLEAR =
        SCP_CONFIG_CLOCK_STOPPED_SYSINCLKSTOPPED |
        SCP_CONFIG_CLOCK_STOPPED_GPUINCLKSTOPPED |
        SCP_CONFIG_CLOCK_STOPPED_LITTLEINCLKSTOPPED |
        SCP_CONFIG_CLOCK_STOPPED_BIGINCLKSTOPPED;

    set_div(&SCP_CONFIG->CCICLK_CONTROL,
        CLOCK_RATE_SYSCLK / CCICLK_CONTROL_CLOCK);
    set_source(&SCP_CONFIG->CCICLK_CONTROL, CLKSEL_SYSCLK);

    set_div(&SCP_CONFIG->NICSCPCLK_CONTROL,
        CLOCK_RATE_SYSCLK / NICSCPCLK_CONTROL_CLOCK);
    set_source(&SCP_CONFIG->NICSCPCLK_CONTROL, CLKSEL_SYSCLK);

    set_div(&SCP_CONFIG->NICPERCLK_CONTROL,
        CLOCK_RATE_SYSCLK / NICPERCLK_CONTROL_CLOCK);
    set_source(&SCP_CONFIG->NICPERCLK_CONTROL, CLKSEL_SYSCLK);

    set_div(&SCP_CONFIG->SPCLK_CONTROL,
        CLOCK_RATE_SYSCLK / SPCLK_CONTROL_CLOCK);
    set_source(&SCP_CONFIG->SPCLK_CONTROL, CLKSEL_SYSCLK);

    set_div(&SCP_CONFIG->GICCLK_CONTROL,
        CLOCK_RATE_SYSCLK / GICCLK_CONTROL_CLOCK);
    set_source(&SCP_CONFIG->GICCLK_CONTROL, CLKSEL_SYSCLK);

    if (revision == JUNO_IDX_REVISION_R0) {
        set_div(&SCP_CONFIG->ATCLK_CONTROL,
            CLOCK_RATE_SYSCLK / ATCLK_CONTROL_CLOCK_REV0);
    } else {
        set_div(&SCP_CONFIG->ATCLK_CONTROL,
            CLOCK_RATE_SYSCLK / ATCLK_CONTROL_CLOCK);
    }

    set_div(&SCP_CONFIG->TRACECLKIN_CONTROL,
        CLOCK_RATE_SYSCLK / TRACECLKIN_CONTROL_CLOCK);
    set_div(&SCP_CONFIG->PCLKDBG_CONTROL, 1);

    if (revision != JUNO_IDX_REVISION_R0)
        set_div(&SCC->SAXICLK, CLOCK_RATE_SYSCLK / SAXICLK_CLOCK);
    set_source(&SCC->SAXICLK, CLKSEL_SYSCLK);

    set_div(&SCC->FAXICLK, CLOCK_RATE_SYSCLK / FAXICLK_CLOCK);
    set_source(&SCC->FAXICLK, CLKSEL_SYSCLK);

    set_div(&SCC->HDLCDCLK, CLOCK_RATE_SYSCLK / HDLCDCLK_CLOCK);
    set_source(&SCC->HDLCDCLK, CLKSEL_SYSCLK);

    if ((SCC->GPR0 & SCC_GPR0_SKIP_TLX_CLK_SETTING) == 0) {
        set_source(&SCC->TMIF2XCLK, CLKSEL_SYSCLK);
        set_source(&SCC->TSIF2XCLK, CLKSEL_SYSCLK);
    }

    set_source(&SCC->USBHCLK, CLKSEL_SYSCLK);

    set_source(&SCC->PCIEACLK, CLKSEL_SYSCLK);

    set_source(&SCC->PCIETLCLK, CLKSEL_SYSCLK);

    return FWK_SUCCESS;
}

const struct fwk_module module_juno_soc_clock = {
    .name = "Juno SoC Clock Driver",
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = juno_soc_clock_init,
};

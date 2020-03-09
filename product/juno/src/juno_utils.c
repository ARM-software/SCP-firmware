/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for Juno.
 */

#include "pl35x.h"
#include "scp_config.h"

#include <fwk_assert.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SNOOP_CONTROL_L2ACCREQ      UINT32_C(0x00000001)
#define SNOOP_CONTROL_L2ACCACK      UINT32_C(0x00000002)

#define SMC_INIT_SET_CYCLES         UINT32_C(0x000251FF)
#define SMC_INIT_SET_OP_MODE        UINT32_C(0x00000046)
#define SMC_INIT_CMD_SET_CS3        UINT32_C(3 << 23)
#define SMC_INIT_CMD_UPDATE_REG     UINT32_C(2 << 21)

void juno_utils_open_snoop_gate_and_wait(volatile uint32_t *snoop_ctrl)
{
    fwk_assert(snoop_ctrl != NULL);

    *snoop_ctrl = SNOOP_CONTROL_L2ACCREQ;
    while ((*snoop_ctrl & SNOOP_CONTROL_L2ACCACK) == 0)
        continue;
}

void juno_utils_close_snoop_gate(volatile uint32_t *snoop_ctrl)
{
    fwk_assert(snoop_ctrl != NULL);

    *snoop_ctrl &= ~SNOOP_CONTROL_L2ACCREQ;
}

void juno_utils_atclk_clock_div_set(uint32_t divider)
{
    fwk_assert(divider <= 16);
    fwk_assert(divider != 0);

    SCP_CONFIG->ATCLK_CONTROL =
        (SCP_CONFIG->ATCLK_CONTROL & ~SCP_CONFIG_STDCLK_CONTROL_CLKDIV_MASK) |
        ((divider - 1) << SCP_CONFIG_STDCLK_CONTROL_CLKDIV_POS);
}

bool juno_utils_atclk_clock_div_set_check(uint32_t divider)
{
    fwk_assert(divider <= 16);
    fwk_assert(divider != 0);

    return
        ((SCP_CONFIG->ATCLK_CONTROL & SCP_CONFIG_STDCLK_CONTROL_CRNTCLKDIV_MASK)
        == ((divider - 1) << SCP_CONFIG_STDCLK_CONTROL_CRNTCLKDIV_POS));
}

void juno_utils_atclk_clock_sel_set(uint32_t source)
{
    fwk_assert(source <= 2);

    SCP_CONFIG->ATCLK_CONTROL =
        (SCP_CONFIG->ATCLK_CONTROL & ~SCP_CONFIG_STDCLK_CONTROL_CLKSEL_MASK) |
        (source << SCP_CONFIG_STDCLK_CONTROL_CLKSEL_POS);
}

bool juno_utils_atclk_clock_sel_set_check(uint32_t source)
{
    fwk_assert(source <= 2);

    return
        ((SCP_CONFIG->ATCLK_CONTROL & SCP_CONFIG_STDCLK_CONTROL_CRNTCLK_MASK) ==
        (source << SCP_CONFIG_STDCLK_CONTROL_CRNTCLK_POS));
}

void juno_utils_traceclk_clock_div_set(uint32_t divider)
{
    fwk_assert(divider <= 16);
    fwk_assert(divider != 0);

    SCP_CONFIG->TRACECLKIN_CONTROL =
        (SCP_CONFIG->TRACECLKIN_CONTROL &
        ~SCP_CONFIG_STDCLK_CONTROL_CLKDIV_MASK) |
        ((divider - 1) << SCP_CONFIG_STDCLK_CONTROL_CLKDIV_POS);
}

bool juno_utils_traceclk_clock_div_set_check(uint32_t divider)
{
    fwk_assert(divider <= 16);
    fwk_assert(divider != 0);

    return ((SCP_CONFIG->TRACECLKIN_CONTROL &
        SCP_CONFIG_STDCLK_CONTROL_CRNTCLKDIV_MASK) ==
        ((divider - 1) << SCP_CONFIG_STDCLK_CONTROL_CRNTCLKDIV_POS));
}

void juno_utils_traceclk_clock_sel_set(uint32_t source)
{
    fwk_assert(source <= 2);

    SCP_CONFIG->TRACECLKIN_CONTROL =
        (SCP_CONFIG->TRACECLKIN_CONTROL &
        ~SCP_CONFIG_STDCLK_CONTROL_CLKSEL_MASK) |
        (source << SCP_CONFIG_STDCLK_CONTROL_CLKSEL_POS);
}

bool juno_utils_traceclk_clock_sel_set_check(uint32_t source)
{
    fwk_assert(source <= 2);

    return ((SCP_CONFIG->TRACECLKIN_CONTROL &
        SCP_CONFIG_STDCLK_CONTROL_CRNTCLK_MASK) ==
        (source << SCP_CONFIG_STDCLK_CONTROL_CRNTCLK_POS));
}

void juno_utils_pclkdbg_clock_div_set(uint32_t divider)
{
    fwk_assert(divider <= 16);
    fwk_assert(divider != 0);

    SCP_CONFIG->PCLKDBG_CONTROL =
        (SCP_CONFIG->PCLKDBG_CONTROL & ~SCP_CONFIG_STDCLK_CONTROL_CLKDIV_MASK) |
        ((divider - 1) << SCP_CONFIG_STDCLK_CONTROL_CLKDIV_POS);
}

bool juno_utils_pclkdbg_clock_div_set_check(uint32_t divider)
{
    fwk_assert(divider <= 16);
    fwk_assert(divider != 0);

    return ((SCP_CONFIG->PCLKDBG_CONTROL &
        SCP_CONFIG_STDCLK_CONTROL_CRNTCLKDIV_MASK) ==
        ((divider - 1) << SCP_CONFIG_STDCLK_CONTROL_CRNTCLKDIV_POS));
}

void juno_utils_system_clock_enable(uint32_t mask)
{
    fwk_assert((mask & ~SCP_CONFIG_CLOCK_ENABLE_ALL) == 0);

    SCP_CONFIG->CLOCK_ENABLE_SET |= mask;
    while ((SCP_CONFIG->CLOCK_ENABLE_STATUS & mask) != mask)
        continue;
}

void juno_utils_smc_init(void)
{
    /* Switch CS3 to 32 bit mode, with read and write sync */
    SMC->SET_CYCLES = SMC_INIT_SET_CYCLES;
    SMC->SET_OPMODE = SMC_INIT_SET_OP_MODE;
    SMC->DIRECT_CMD = SMC_INIT_CMD_SET_CS3 | SMC_INIT_CMD_UPDATE_REG;
}

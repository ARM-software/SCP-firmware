/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "coresight_soc400.h"
#include "juno_ssc.h"
#include "juno_wdog.h"
#include "system_mmap.h"

#include <stdbool.h>
#include <stdint.h>

#define CTI_TRIGGER_WDOG_DISABLE        UINT32_C(0x00000001)

#define CTI ((struct cti_reg *) CTI_BASE)

static const uint32_t wdog_interval = SCP_WDOG_MS_TO_TICKS(750);

void juno_wdog_rom_halt_on_debug_config(void)
{
    /* Enable the Halt On Debug functionality for wdog */
    SSC->SSC_SWDHOD = JUNO_SSC_SWDHOD_ALL;

    /* Configure Coresight */
    CTI->LAR = CTI_LAR_KEY;
    CTI->CTI_CONTROL = CTI_CONTROL_ENABLE;
    CTI->CTI_GATE = CTI_GATE_EN_NONE;
    CTI->CTI_OUT_EN[CTI_TRIGGER_WDOG_DISABLE] = CTI_OUT_EN_CH3;
    CTI->LAR = ~CTI_LAR_KEY;
}

void juno_wdog_rom_enable(void)
{
    WDOG->LOCK = SP805_LOCK_EN_ACCESS;
    WDOG->LOAD = wdog_interval;
    WDOG->CONTROL = SP805_CONTROL_INTR_EN | SP805_CONTROL_RESET_EN;
    WDOG->LOCK = ~SP805_LOCK_EN_ACCESS;
}

void juno_wdog_rom_reload(void)
{
    WDOG->LOCK = SP805_LOCK_EN_ACCESS;
    WDOG->LOAD = wdog_interval;
    WDOG->LOCK = ~SP805_LOCK_EN_ACCESS;
}

void juno_wdog_rom_halt_on_debug_enable(bool enable)
{
    if (enable) {
        /* Enable halt on debug */
        CTI->LAR = CTI_LAR_KEY;
        CTI->CTI_APP_SET = CTI_APP_SET_CH3;
        CTI->LAR = ~CTI_LAR_KEY;
    } else {
        /* Disable halt on debug */
        CTI->LAR = CTI_LAR_KEY;
        CTI->CTI_APP_CLEAR = CTI_APP_SET_CH3;
        CTI->CTI_OUT_EN[6] = CTI_OUT_EN_CH2;
        CTI->CTI_APP_PULSE = CTI_APP_SET_CH2;
        CTI->LAR = ~CTI_LAR_KEY;
    }
}

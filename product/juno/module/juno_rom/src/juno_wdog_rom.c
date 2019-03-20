/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <juno_wdog.h>

/* Default watchdog interval */
static const uint32_t wdog_interval = SCP_WDOG_MS_TO_TICKS(750);

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

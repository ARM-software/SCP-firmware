/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_wdog.h"

void juno_wdog_ram_disable(void)
{
    WDOG->LOCK = SP805_LOCK_EN_ACCESS;
    WDOG->CONTROL = 0;
    WDOG->LOCK = ~SP805_LOCK_EN_ACCESS;
}

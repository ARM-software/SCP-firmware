/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Workaround for the PLL lock error emitted by the motherboard, seen if
 *      the SCP fails to lock the system PLLs within a certain (very short)
 *      timeframe.
 */

#include "juno_scc.h"

#include <fwk_noreturn.h>

#ifdef __ARMCC_VERSION
#   define __wrap_arm_exception_reset $Sub$$arm_exception_reset
#   define __real_arm_exception_reset $Super$$arm_exception_reset
#endif

/*
 * These PLLs must be released from reset very shortly after the SCP is released
 * from reset, otherwise the motherboard microcontroller will kill the SCP and
 * error out. We do this at the earliest possible point in time in order to
 * ensure nothing delays it from happening.
 */
noreturn void __wrap_arm_exception_reset(void)
{
    extern noreturn void __real_arm_exception_reset(void);

    unsigned int pll_idx;

    /* Release All system PLLs from reset */
    for (pll_idx = 0; pll_idx < PLL_IDX_COUNT; pll_idx++)
        SCC->PLL[pll_idx].REG0 &= ~PLL_REG0_PLL_RESET;

    __real_arm_exception_reset();
}

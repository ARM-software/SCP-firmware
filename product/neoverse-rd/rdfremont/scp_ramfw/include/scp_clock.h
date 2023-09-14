/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP clock definitions.
 */

#ifndef SCP_CLOCK_H
#define SCP_CLOCK_H

/*
 * PLL clock indices.
 */
enum clock_pll_idx {
    CFGD_MOD_SYSTEM_PLL_EIDX_SYS,
    CFGD_MOD_SYSTEM_PLL_EIDX_INTERCONNECT,
    CFGD_MOD_SYSTEM_PLL_EIDX_COUNT
};

#endif /* SCP_CLOCK_H */

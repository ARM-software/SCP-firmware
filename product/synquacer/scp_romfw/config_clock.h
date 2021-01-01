/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_CLOCK_H
#define CONFIG_CLOCK_H

/*
 * PIK clock indexes.
 */
enum clock_pik_idx {
    CLOCK_PIK_IDX_CORECLK,
    CLOCK_PIK_IDX_ACLK,
    CLOCK_PIK_IDX_PPUCLK,
    CLOCK_PIK_IDX_CCNCLK,
    CLOCK_PIK_IDX_GICCLK,
    CLOCK_PIK_IDX_PCLKSCP,
    CLOCK_PIK_IDX_SYSPERCLK,
    CLOCK_PIK_IDX_SYSPCLKDBG,
    CLOCK_PIK_IDX_UARTCLK,
    CLOCK_PIK_IDX_DMCCLK,

    CLOCK_PIK_IDX_COUNT
};

#endif /* CONFIG_CLOCK_H */

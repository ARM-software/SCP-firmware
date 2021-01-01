/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_CLOCK_H
#define CONFIG_CLOCK_H

/*
 * Clock indexes.
 */
enum clock_idx {
    CLOCK_IDX_INTERCONNECT,
    CLOCK_IDX_CPU_GROUP0,
    CLOCK_IDX_CPU_GROUP1,
    CLOCK_IDX_COUNT
};

/*
 * PIK clock indexes.
 */
enum clock_pik_idx {
    CLOCK_PIK_IDX_CLUS0_CPU0,
    CLOCK_PIK_IDX_CLUS0_CPU1,
    CLOCK_PIK_IDX_CLUS0_CPU2,
    CLOCK_PIK_IDX_CLUS0_CPU3,
    CLOCK_PIK_IDX_CLUS1_CPU0,
    CLOCK_PIK_IDX_CLUS1_CPU1,
    CLOCK_PIK_IDX_CLUS1_CPU2,
    CLOCK_PIK_IDX_CLUS1_CPU3,
    CLOCK_PIK_IDX_DMC,
    CLOCK_PIK_IDX_INTERCONNECT,
    CLOCK_PIK_IDX_SCP,
    CLOCK_PIK_IDX_GIC,
    CLOCK_PIK_IDX_PCLKSCP,
    CLOCK_PIK_IDX_SYSPERCLK,
    CLOCK_PIK_IDX_UARTCLK,
    CLOCK_PIK_IDX_COUNT
};

/*
 * CSS clock indexes.
 */
enum clock_css_idx {
    CLOCK_CSS_IDX_CPU_GROUP0,
    CLOCK_CSS_IDX_CPU_GROUP1,
    CLOCK_CSS_IDX_COUNT
};

/*
 * PLL clock indexes.
 */
enum clock_pll_idx {
    CLOCK_PLL_IDX_CPU0,
    CLOCK_PLL_IDX_CPU1,
    CLOCK_PLL_IDX_SYS,
    CLOCK_PLL_IDX_DMC,
    CLOCK_PLL_IDX_INTERCONNECT,
    CLOCK_PLL_IDX_COUNT
};

#endif /* CONFIG_CLOCK_H */

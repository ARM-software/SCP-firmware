/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_TIMER_H
#define CONFIG_TIMER_H

enum config_timer_refclk_sub_element_idx {
    TC1_CONFIG_TIMER_DVFS_CPU_CORTEX_A510,
    TC1_CONFIG_TIMER_DVFS_CPU_CORTEX_A715,
    TC1_CONFIG_TIMER_DVFS_CPU_CORTEX_X3,
    TC1_CONFIG_TIMER_SUB_ELEMENT_IDX_COUNT,
};

#endif /* CONFIG_TIMER_H */

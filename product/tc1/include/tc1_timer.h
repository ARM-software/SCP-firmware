/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_TIMER_H
#define CONFIG_TIMER_H

enum config_timer_refclk_sub_element_idx {
    TC1_CONFIG_TIMER_DVFS_CPU_KLEIN,
    TC1_CONFIG_TIMER_DVFS_CPU_MAKALU,
    TC1_CONFIG_TIMER_DVFS_CPU_MAKALU_ELP,
    TC1_CONFIG_TIMER_SUB_ELEMENT_IDX_COUNT,
};

#endif /* CONFIG_TIMER_H */

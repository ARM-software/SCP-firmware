/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_TIMER_H
#define CONFIG_TIMER_H

enum config_timer_element_idx {
    CONFIG_TIMER_ELEMENT_IDX_REFCLK,
    CONFIG_TIMER_ELEMENT_IDX_COUNT,
};

enum config_timer_refclk_sub_element_idx {
    CONFIG_TIMER_DVFS_CPU_GROUP_LITTLE,
    CONFIG_TIMER_DVFS_CPU_GROUP_BIG,
    CONFIG_TIMER_DVFS_GPU,
    CONFIG_TIMER_DEBUGGER_CLI,
    CONFIG_TIMER_REFCLK_SUB_ELEMENT_IDX_COUNT,
};

#endif /* CONFIG_TIMER_H */

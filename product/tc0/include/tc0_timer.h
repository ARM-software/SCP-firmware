/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_TIMER_H
#define CONFIG_TIMER_H

enum config_timer_refclk_sub_element_idx {
    CONFIG_TIMER_DVFS_CPU_KLEIN,
    CONFIG_TIMER_DVFS_CPU_MATTERHORN,
    CONFIG_TIMER_DVFS_CPU_MATTERHORN_ELP_ARM,

#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
    CONFIG_TIMER_FAST_CHANNEL_TIMER_IDX,
#endif

    CONFIG_TIMER_SUB_ELEMENT_IDX_COUNT,
};

#endif /* CONFIG_TIMER_H */

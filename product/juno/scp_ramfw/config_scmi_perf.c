/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_dvfs.h"
#include "juno_alarm_idx.h"
#include "juno_scmi.h"
#include "software_mmap.h"

#include <internal/scmi_perf.h>
#include <mod_scmi_perf.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

#ifdef BUILD_HAS_MOD_STATISTICS
#    include <mod_stats.h>
#endif

#define FC_LEVEL_SET(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LIMIT_SET(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LEVEL_GET(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LIMIT_GET(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX))

#define FC_LEVEL_SET_AP(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - EXTERNAL_DEV_BASE)

#define FC_LIMIT_SET_AP(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_SET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - EXTERNAL_DEV_BASE)

#define FC_LEVEL_GET_AP(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - EXTERNAL_DEV_BASE)

#define FC_LIMIT_GET_AP(PERF_IDX) \
    (SCMI_FAST_CHANNEL_BASE + MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_GET + \
     (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * PERF_IDX) - EXTERNAL_DEV_BASE)

static const struct mod_scmi_perf_domain_config domains[] = {
    [DVFS_ELEMENT_IDX_LITTLE] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fast_channels_addr_scp = (uint64_t[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FC_LEVEL_SET(DVFS_ELEMENT_IDX_LITTLE),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FC_LIMIT_SET(DVFS_ELEMENT_IDX_LITTLE),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FC_LEVEL_GET(DVFS_ELEMENT_IDX_LITTLE),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FC_LIMIT_GET(DVFS_ELEMENT_IDX_LITTLE),
        },
        .fast_channels_addr_ap = (uint64_t[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FC_LEVEL_SET_AP(DVFS_ELEMENT_IDX_LITTLE),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FC_LIMIT_SET_AP(DVFS_ELEMENT_IDX_LITTLE),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FC_LEVEL_GET_AP(DVFS_ELEMENT_IDX_LITTLE),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FC_LIMIT_GET_AP(DVFS_ELEMENT_IDX_LITTLE),
        },
#endif
#ifdef BUILD_HAS_STATISTICS
        .stats_collected = true,
#endif
    },
    [DVFS_ELEMENT_IDX_BIG] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fast_channels_addr_scp = (uint64_t[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FC_LEVEL_SET(DVFS_ELEMENT_IDX_BIG),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FC_LIMIT_SET(DVFS_ELEMENT_IDX_BIG),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FC_LEVEL_GET(DVFS_ELEMENT_IDX_BIG),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FC_LIMIT_GET(DVFS_ELEMENT_IDX_BIG),
        },
        .fast_channels_addr_ap = (uint64_t[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FC_LEVEL_SET_AP(DVFS_ELEMENT_IDX_BIG),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FC_LIMIT_SET_AP(DVFS_ELEMENT_IDX_BIG),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FC_LEVEL_GET_AP(DVFS_ELEMENT_IDX_BIG),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FC_LIMIT_GET_AP(DVFS_ELEMENT_IDX_BIG),
        },
#endif
#ifdef BUILD_HAS_STATISTICS
        .stats_collected = true,
#endif
    },
    [DVFS_ELEMENT_IDX_GPU] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fast_channels_addr_scp = (uint64_t[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FC_LEVEL_SET(DVFS_ELEMENT_IDX_GPU),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FC_LIMIT_SET(DVFS_ELEMENT_IDX_GPU),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FC_LEVEL_GET(DVFS_ELEMENT_IDX_GPU),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FC_LIMIT_GET(DVFS_ELEMENT_IDX_GPU),
        },
        .fast_channels_addr_ap = (uint64_t[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FC_LEVEL_SET_AP(DVFS_ELEMENT_IDX_GPU),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FC_LIMIT_SET_AP(DVFS_ELEMENT_IDX_GPU),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FC_LEVEL_GET_AP(DVFS_ELEMENT_IDX_GPU),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FC_LIMIT_GET_AP(DVFS_ELEMENT_IDX_GPU),
        },
#endif
#ifdef BUILD_HAS_MOD_STATISTICS
        .stats_collected = true,
#endif
    },
};

struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config){
        .domains = &domains,
        .perf_doms_count = FWK_ARRAY_SIZE(domains),
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fast_channels_alarm_id = FWK_ID_SUB_ELEMENT_INIT(
            FWK_MODULE_IDX_TIMER,
            0,
            JUNO_SCMI_FAST_CHANNEL_IDX),
        .fast_channels_rate_limit = SCMI_PERF_FC_MIN_RATE_LIMIT,
#else
        .fast_channels_alarm_id = FWK_ID_NONE_INIT,
#endif
#ifdef BUILD_HAS_MOD_STATISTICS
        .stats_enabled = true,
#endif
    }),
};

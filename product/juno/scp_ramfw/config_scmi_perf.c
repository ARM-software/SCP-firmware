/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
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

static const struct mod_scmi_perf_domain_config
    domains[DVFS_ELEMENT_IDX_COUNT] = {
        [DVFS_ELEMENT_IDX_BIG] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
         .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(JUNO_TRANSPORT_SCMI_PERF_FCH_BIG_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(JUNO_TRANSPORT_SCMI_PERF_FCH_BIG_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(JUNO_TRANSPORT_SCMI_PERF_FCH_BIG_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(JUNO_TRANSPORT_SCMI_PERF_FCH_BIG_LIMIT_GET),
        },
        .supports_fast_channels = true,
#endif
#ifdef BUILD_HAS_STATISTICS
        .stats_collected = true,
#endif
        },
        [DVFS_ELEMENT_IDX_LITTLE] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
         .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(JUNO_TRANSPORT_SCMI_PERF_FCH_LITTLE_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(JUNO_TRANSPORT_SCMI_PERF_FCH_LITTLE_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(JUNO_TRANSPORT_SCMI_PERF_FCH_LITTLE_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(JUNO_TRANSPORT_SCMI_PERF_FCH_LITTLE_LIMIT_GET),
        },
        .supports_fast_channels = true,
#endif
#ifdef BUILD_HAS_STATISTICS
        .stats_collected = true,
#endif
        },
        [DVFS_ELEMENT_IDX_GPU] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
         .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(JUNO_TRANSPORT_SCMI_PERF_FCH_GPU_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(JUNO_TRANSPORT_SCMI_PERF_FCH_GPU_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(JUNO_TRANSPORT_SCMI_PERF_FCH_GPU_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(JUNO_TRANSPORT_SCMI_PERF_FCH_GPU_LIMIT_GET),
        },
        .supports_fast_channels = true,
#    ifdef BUILD_HAS_STATISTICS
        .stats_collected = true,
#    endif
#endif
        }
};

struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config){
        .domains = &domains,
        .perf_doms_count = FWK_ARRAY_SIZE(domains),
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fast_channels_rate_limit = SCMI_PERF_FC_MIN_RATE_LIMIT,
#else
        .fast_channels_alarm_id = FWK_ID_NONE_INIT,
#endif
#ifdef BUILD_HAS_MOD_STATISTICS
        .stats_enabled = true,
#endif
    }),
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_dvfs.h"
#include "platform_scmi.h"
#include "rd_alarm_idx.h"
#include "scp_software_mmap.h"

#include <internal/scmi_perf.h>

#include <mod_scmi_perf.h>

#include <fwk_module.h>

#include <stddef.h>
#include <stdint.h>

static const struct mod_scmi_perf_domain_config
    domains[DVFS_ELEMENT_IDX_COUNT] = {
        [DVFS_ELEMENT_IDX_CPU0] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU0_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU0_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU0_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU0_LIMIT_GET),
        },
        .supports_fast_channels = true,
#endif
    },
        [DVFS_ELEMENT_IDX_CPU1] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU1_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU1_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU1_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU1_LIMIT_GET),
        },
        .supports_fast_channels = true,
#endif
    },
        [DVFS_ELEMENT_IDX_CPU2] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU2_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU2_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU2_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU2_LIMIT_GET),
        },
        .supports_fast_channels = true,
#endif
    },
        [DVFS_ELEMENT_IDX_CPU3] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU3_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU3_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU3_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDV1MC_TRANSPORT_SCMI_PERF_FCH_CPU3_LIMIT_GET),
        },
        .supports_fast_channels = true,
#endif
    },
};

const struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config){
        .domains = &domains,
        .perf_doms_count = FWK_ARRAY_SIZE(domains),
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fast_channels_rate_limit = SCMI_PERF_FC_MIN_RATE_LIMIT,
#endif /* BUILD_HAS_SCMI_PERF_FAST_CHANNELS */
    }),
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_dvfs.h"
#include "platform_core.h"
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
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU0_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU0_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU0_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU0_LIMIT_GET),
        },
        .supports_fast_channels = true,
#endif
    },
        [DVFS_ELEMENT_IDX_CPU1] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU1_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU1_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU1_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU1_LIMIT_GET),
        },
        .supports_fast_channels = true,
#endif
    },
        [DVFS_ELEMENT_IDX_CPU2] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU2_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU2_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU2_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU2_LIMIT_GET),
        },
        .supports_fast_channels = true,
#endif
    },
        [DVFS_ELEMENT_IDX_CPU3] = {
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU3_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU3_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU3_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU3_LIMIT_GET),
        },
        .supports_fast_channels = true,
#endif
    },
#    if (NUMBER_OF_CLUSTERS > 4)
        [DVFS_ELEMENT_IDX_CPU4] = {
#    ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU4_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU4_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU4_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU4_LIMIT_GET),
        },
        .supports_fast_channels = true,
#    endif
    },
        [DVFS_ELEMENT_IDX_CPU5] = {
#    ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU5_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU5_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU5_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU5_LIMIT_GET),
        },
        .supports_fast_channels = true,
#    endif
    },
        [DVFS_ELEMENT_IDX_CPU6] = {
#    ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU6_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU6_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU6_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU6_LIMIT_GET),
        },
        .supports_fast_channels = true,
#    endif
    },
        [DVFS_ELEMENT_IDX_CPU7] = {
#    ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU7_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU7_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU7_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU7_LIMIT_GET),
        },
        .supports_fast_channels = true,
#    endif
    },
#    if (NUMBER_OF_CLUSTERS > 8)
        [DVFS_ELEMENT_IDX_CPU8] = {
#        ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU8_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU8_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU8_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU8_LIMIT_GET),
        },
        .supports_fast_channels = true,
#        endif
    },
        [DVFS_ELEMENT_IDX_CPU9] = {
#        ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU9_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU9_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU9_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU9_LIMIT_GET),
        },
        .supports_fast_channels = true,
#        endif
    },
        [DVFS_ELEMENT_IDX_CPU10] = {
#        ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU10_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU10_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU10_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU10_LIMIT_GET),
        },
        .supports_fast_channels = true,
#        endif
    },
        [DVFS_ELEMENT_IDX_CPU11] = {
#        ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU11_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU11_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU11_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU11_LIMIT_GET),
        },
        .supports_fast_channels = true,
#        endif
    },
        [DVFS_ELEMENT_IDX_CPU12] = {
#        ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU12_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU12_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU12_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU12_LIMIT_GET),
        },
        .supports_fast_channels = true,
#        endif
    },
        [DVFS_ELEMENT_IDX_CPU13] = {
#        ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU13_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU13_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU13_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU13_LIMIT_GET),
        },
        .supports_fast_channels = true,
#        endif
    },
        [DVFS_ELEMENT_IDX_CPU14] = {
#        ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU14_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU14_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU14_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU14_LIMIT_GET),
        },
        .supports_fast_channels = true,
#        endif
    },
        [DVFS_ELEMENT_IDX_CPU15] = {
#        ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fch_config = (struct scmi_perf_fch_config[]) {
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU15_LEVEL_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU15_LIMIT_SET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU15_LEVEL_GET),
            [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] =
                FCH_INIT(RDN2_TRANSPORT_SCMI_PERF_FCH_CPU15_LIMIT_GET),
        },
        .supports_fast_channels = true,
#        endif
    },
#    endif /* #if (NUMBER_OF_CLUSTERS > 4) */
#endif /* #if (NUMBER_OF_CLUSTERS > 8) */
};

const struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config){
        .domains = &domains,
        .perf_doms_count = FWK_ARRAY_SIZE(domains),
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fast_channels_rate_limit = SCMI_PERF_FC_MIN_RATE_LIMIT,
#else
        .fast_channels_alarm_id = FWK_ID_NONE_INIT,
#endif
    }),
};

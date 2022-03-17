/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rd_alarm_idx.h"
#include "scp_software_mmap.h"

#include <internal/scmi_perf.h>

#include <mod_scmi_perf.h>

#include <fwk_module.h>

#include <stddef.h>
#include <stdint.h>

#define FAST_CHANNEL_ADDRESS_SCP(n) \
    { \
        [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] = SCP_SCMI_FAST_CHANNEL_BASE + \
            MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_SET + \
            (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * n), \
        [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] = SCP_SCMI_FAST_CHANNEL_BASE + \
            MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_SET + \
            (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * n), \
        [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] = SCP_SCMI_FAST_CHANNEL_BASE + \
            MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_GET + \
            (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * n), \
        [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] = SCP_SCMI_FAST_CHANNEL_BASE + \
            MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_GET + \
            (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * n) \
    }

#define FAST_CHANNEL_ADDRESS_AP(n) \
    { \
        [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_SET] = SCP_SCMI_FAST_CHANNEL_BASE + \
            MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_SET + \
            (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * n) - \
            SCP_SYSTEM_ACCESS_PORT1_BASE, \
        [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_SET] = SCP_SCMI_FAST_CHANNEL_BASE + \
            MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_SET + \
            (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * n) - \
            SCP_SYSTEM_ACCESS_PORT1_BASE, \
        [MOD_SCMI_PERF_FAST_CHANNEL_LEVEL_GET] = SCP_SCMI_FAST_CHANNEL_BASE + \
            MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LEVEL_GET + \
            (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * n) - \
            SCP_SYSTEM_ACCESS_PORT1_BASE, \
        [MOD_SCMI_PERF_FAST_CHANNEL_LIMIT_GET] = SCP_SCMI_FAST_CHANNEL_BASE + \
            MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_LIMIT_GET + \
            (MOD_SCMI_PERF_FAST_CHANNEL_OFFSET_TOTAL * n) - \
            SCP_SYSTEM_ACCESS_PORT1_BASE \
    }

#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
static const struct mod_scmi_perf_domain_config domains[] = {
    [0] = { .fast_channels_addr_scp = (uint64_t[])FAST_CHANNEL_ADDRESS_SCP(0),
            .fast_channels_addr_ap = (uint64_t[])FAST_CHANNEL_ADDRESS_AP(0) },
    [1] = { .fast_channels_addr_scp = (uint64_t[])FAST_CHANNEL_ADDRESS_SCP(1),
            .fast_channels_addr_ap = (uint64_t[])FAST_CHANNEL_ADDRESS_AP(1) },
    [2] = { .fast_channels_addr_scp = (uint64_t[])FAST_CHANNEL_ADDRESS_SCP(2),
            .fast_channels_addr_ap = (uint64_t[])FAST_CHANNEL_ADDRESS_AP(2) },
    [3] = { .fast_channels_addr_scp = (uint64_t[])FAST_CHANNEL_ADDRESS_SCP(3),
            .fast_channels_addr_ap = (uint64_t[])FAST_CHANNEL_ADDRESS_AP(3) },
};
#else
static const struct mod_scmi_perf_domain_config domains[4] = { 0 };
#endif

const struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config){
        .domains = &domains,
        .perf_doms_count = FWK_ARRAY_SIZE(domains),
#ifdef BUILD_HAS_SCMI_PERF_FAST_CHANNELS
        .fast_channels_alarm_id = FWK_ID_SUB_ELEMENT_INIT(
            FWK_MODULE_IDX_TIMER,
            0,
            RD_SCMI_FAST_CHANNEL_IDX),
        .fast_channels_rate_limit = SCMI_PERF_FC_MIN_RATE_LIMIT,
#else
        .fast_channels_alarm_id = FWK_ID_NONE_INIT,
#endif
    }),
};

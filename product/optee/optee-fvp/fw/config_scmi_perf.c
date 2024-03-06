/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_dvfs.h"

#include <scmi_agents.h>

#include <mod_scmi_perf.h>

#include <fwk_module.h>

static const struct mod_scmi_perf_domain_config domains[] = {
    [DVFS_ELEMENT_IDX_LITTLE] = {
        .fast_channels_addr_scp = 0x0,
    },
    [DVFS_ELEMENT_IDX_BIG] = {
        .fast_channels_addr_scp = 0x0,
    },
    [DVFS_ELEMENT_IDX_GPU] = {
        .fast_channels_addr_scp = 0x0,
    },
};

struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config){
        .domains = &domains,
        .perf_doms_count = FWK_ARRAY_SIZE(domains),
        .fast_channels_alarm_id = FWK_ID_NONE_INIT,
    }),
};

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
    [DVFS_ELEMENT_IDX_LITTLE] = { 0 },
    [DVFS_ELEMENT_IDX_BIG] = { 0 },
    [DVFS_ELEMENT_IDX_GPU] = { 0 },
};

struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config){
        .domains = &domains,
        .perf_doms_count = FWK_ARRAY_SIZE(domains),
    }),
};

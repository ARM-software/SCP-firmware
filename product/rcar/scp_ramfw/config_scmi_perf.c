/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_dvfs.h>
#include <rcar_scmi.h>

#include <mod_scmi_perf.h>

#include <fwk_element.h>
#include <fwk_module.h>

#include <stdint.h>

static const struct mod_scmi_perf_domain_config domains[] = {
    [DVFS_ELEMENT_IDX_LITTLE] = {
        .stats_collected = true,
    },
    [DVFS_ELEMENT_IDX_BIG] = {
        .stats_collected = true,
    },
};

struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config){
        .domains = &domains,
        .perf_doms_count = FWK_ARRAY_SIZE(domains),
    }),
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tc1_dvfs.h"
#include "tc_scmi.h"

#include <mod_scmi_perf.h>

#include <fwk_module.h>

#include <stdint.h>

static const struct mod_scmi_perf_domain_config
    domains[DVFS_ELEMENT_IDX_COUNT] = {
        [DVFS_ELEMENT_IDX_CORTEX_A510] = {},
        [DVFS_ELEMENT_IDX_CORTEX_A715] = {},
        [DVFS_ELEMENT_IDX_CORTEX_X3] = {},
    };

const struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config){
        .domains = &domains,
        .perf_doms_count = FWK_ARRAY_SIZE(domains),
    }),
};

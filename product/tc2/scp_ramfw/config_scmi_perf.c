/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tc2_dvfs.h"
#include "tc2_scmi.h"

#include <mod_scmi_perf.h>

#include <fwk_module.h>

#include <stdint.h>

static const struct mod_scmi_perf_domain_config
    domains[DVFS_ELEMENT_IDX_COUNT] = {
        [DVFS_ELEMENT_IDX_HAYES] = {},
        [DVFS_ELEMENT_IDX_HUNTER] = {},
    };

const struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config){
        .domains = &domains,
        .perf_doms_count = FWK_ARRAY_SIZE(domains),
        .fast_channels_alarm_id = FWK_ID_NONE_INIT,
    }),
};

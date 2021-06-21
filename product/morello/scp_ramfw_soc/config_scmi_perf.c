/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_dvfs.h>
#include <morello_scp_scmi.h>

#include <mod_scmi_perf.h>

#include <fwk_element.h>
#include <fwk_module.h>

#include <stdint.h>

static const struct mod_scmi_perf_domain_config
    domains[DVFS_ELEMENT_IDX_COUNT] = {
        [DVFS_ELEMENT_IDX_CLUS0] = {},
        [DVFS_ELEMENT_IDX_CLUS1] = {},
        [DVFS_ELEMENT_IDX_GPU] = {},
    };

struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config){
        .domains = &domains,
        .fast_channels_alarm_id = FWK_ID_NONE_INIT,
        .perf_doms_count = FWK_ARRAY_SIZE(domains),
    }),
};

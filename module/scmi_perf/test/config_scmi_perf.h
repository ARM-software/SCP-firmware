/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_scmi_perf.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

#define TEST_MODULE_IDX       0x5
#define TEST_SCMI_AGENT_IDX_0 0x1

enum dvfs_element_idx {
    DVFS_ELEMENT_IDX_0,
    DVFS_ELEMENT_IDX_1,
    DVFS_ELEMENT_IDX_2,
    DVFS_ELEMENT_IDX_COUNT,
};

static const struct mod_scmi_perf_domain_config domains[] = {
    [DVFS_ELEMENT_IDX_0] = {},
    [DVFS_ELEMENT_IDX_1] = {},
    [DVFS_ELEMENT_IDX_2] = {},
};

struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config){
        .domains = &domains,
        .perf_doms_count = FWK_ARRAY_SIZE(domains),
        .fast_channels_alarm_id = FWK_ID_NONE_INIT,
    }),
};

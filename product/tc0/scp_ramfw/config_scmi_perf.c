/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tc0_dvfs.h"
#include "tc0_scmi.h"

#include <mod_scmi_perf.h>

#include <fwk_module.h>

#include <stdint.h>

static const struct mod_scmi_perf_domain_config domains[] = {
    [DVFS_ELEMENT_IDX_KLEIN] = {},
    [DVFS_ELEMENT_IDX_MATTERHORN] = {},
    [DVFS_ELEMENT_IDX_MATTERHORN_ELP_ARM] = {},
};

const struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config){
        .domains = &domains,
        .fast_channels_alarm_id = FWK_ID_NONE_INIT,
    }),
};

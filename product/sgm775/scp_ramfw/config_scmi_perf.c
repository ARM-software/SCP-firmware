/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_dvfs.h"
#include "sgm775_scmi.h"

#include <mod_scmi_perf.h>

#include <fwk_module.h>

#include <stddef.h>
#include <stdint.h>

static const struct mod_scmi_perf_domain_config domains[] = {
    [DVFS_ELEMENT_IDX_LITTLE] = {
    },
    [DVFS_ELEMENT_IDX_BIG] = {
    },
    [DVFS_ELEMENT_IDX_GPU] = {
    },
};

struct fwk_module_config config_scmi_perf = {
    .data = &((struct mod_scmi_perf_config){
        .domains = &domains,
        .fast_channels_alarm_id = FWK_ID_NONE_INIT,
    }),
};

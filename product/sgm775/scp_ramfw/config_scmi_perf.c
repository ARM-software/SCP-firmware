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
        .permissions = &(const uint32_t[]) {
            [SCMI_AGENT_ID_OSPM] = MOD_SCMI_PERF_PERMS_SET_LEVEL  |
                                   MOD_SCMI_PERF_PERMS_SET_LIMITS,
            [SCMI_AGENT_ID_PSCI] = MOD_SCMI_PERF_PERMS_NONE,
        }
    },
    [DVFS_ELEMENT_IDX_BIG] = {
        .permissions = &(const uint32_t[]) {
            [SCMI_AGENT_ID_OSPM] = MOD_SCMI_PERF_PERMS_SET_LEVEL  |
                                   MOD_SCMI_PERF_PERMS_SET_LIMITS,
            [SCMI_AGENT_ID_PSCI] = MOD_SCMI_PERF_PERMS_NONE,
        }
    },
    [DVFS_ELEMENT_IDX_GPU] = {
        .permissions = &(const uint32_t[]) {
            [SCMI_AGENT_ID_OSPM] = MOD_SCMI_PERF_PERMS_SET_LEVEL  |
                                   MOD_SCMI_PERF_PERMS_SET_LIMITS,
            [SCMI_AGENT_ID_PSCI] = MOD_SCMI_PERF_PERMS_NONE,
        }
    },
};

struct fwk_module_config config_scmi_perf = {
    .get_element_table = NULL,
    .data = &((struct mod_scmi_perf_config) {
        .domains = &domains,
    }),
};

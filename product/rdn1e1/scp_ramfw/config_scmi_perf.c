/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <fwk_element.h>
#include <fwk_module.h>
#include <scp_rdn1e1_scmi.h>
#include <mod_scmi_perf.h>

static const struct mod_scmi_perf_domain_config domains[] = {
    [0] = {
        .permissions = &(const uint32_t[]) {
            [SCP_SCMI_AGENT_ID_OSPM] = MOD_SCMI_PERF_PERMS_SET_LEVEL  |
                                       MOD_SCMI_PERF_PERMS_SET_LIMITS,
            [SCP_SCMI_AGENT_ID_PSCI] = MOD_SCMI_PERF_PERMS_NONE,
        }
    },
    [1] = {
        .permissions = &(const uint32_t[]) {
            [SCP_SCMI_AGENT_ID_OSPM] = MOD_SCMI_PERF_PERMS_SET_LEVEL  |
                                       MOD_SCMI_PERF_PERMS_SET_LIMITS,
            [SCP_SCMI_AGENT_ID_PSCI] = MOD_SCMI_PERF_PERMS_NONE,
        }
    },
};

const struct fwk_module_config config_scmi_perf = {
    .get_element_table = NULL,
    .data = &((struct mod_scmi_perf_config) {
        .domains = &domains,
    }),
};

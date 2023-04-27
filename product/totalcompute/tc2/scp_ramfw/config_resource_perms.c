/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "tc_scmi.h"

#include <mod_resource_perms.h>
#include <mod_scmi_std.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/*!
 * If the agent wants to modify permissions at run-time these tables
 * must be allocated in writable memory.
 */

#define AGENT_IDX(agent_id) (agent_id - 1)

static struct mod_res_agent_protocol_permissions agent_protocol_permissions[2] = {
        [AGENT_IDX(SCP_SCMI_AGENT_ID_OSPM)] = {
                .protocols = MOD_RES_PERMS_SCMI_ALL_PROTOCOLS_ALLOWED,
        },

        /* PSCI agent has no access to clock, perf and sensor protocol */
        [AGENT_IDX(SCP_SCMI_AGENT_ID_PSCI)] = {
                .protocols = MOD_RES_PERMS_SCMI_CLOCK_PROTOCOL_DENIED |
                    MOD_RES_PERMS_SCMI_PERF_PROTOCOL_DENIED |
                    MOD_RES_PERMS_SCMI_SENSOR_PROTOCOL_DENIED,
        },
};

/*
 * Messages have an index offset from 0x3 as all agents can access
 * the VERSION/ATTRIBUTES/MSG_ATTRIBUTES messages for all
 * protocols, hence message 0x3 maps to bit[0], message 0x4 maps
 * to bit[1], etc.
 */
static struct mod_res_agent_msg_permissions
    agent_msg_permissions[2] = {
            [AGENT_IDX(SCP_SCMI_AGENT_ID_OSPM)] = {
                    .messages = {
                        [MOD_RES_PERMS_SCMI_BASE_MESSAGE_IDX] = 0x0,
                        [MOD_RES_PERMS_SCMI_POWER_DOMAIN_MESSAGE_IDX] =
                            0x0,
                        [MOD_RES_PERMS_SCMI_SYS_POWER_MESSAGE_IDX] = 0x0,
                        [MOD_RES_PERMS_SCMI_PERF_MESSAGE_IDX] = 0x0,
                        [MOD_RES_PERMS_SCMI_CLOCK_MESSAGE_IDX] = 0x0,
                        [MOD_RES_PERMS_SCMI_SENSOR_MESSAGE_IDX] = 0x0,
                        [MOD_RES_PERMS_SCMI_RESET_DOMAIN_MESSAGE_IDX] =
                            0x0,
                    },
            },
            [AGENT_IDX(SCP_SCMI_AGENT_ID_PSCI)] = {
                    .messages = {
                        [MOD_RES_PERMS_SCMI_BASE_MESSAGE_IDX] = 0x0,
                        [MOD_RES_PERMS_SCMI_POWER_DOMAIN_MESSAGE_IDX] =
                            0x0,
                        [MOD_RES_PERMS_SCMI_SYS_POWER_MESSAGE_IDX] = 0x0,
                        [MOD_RES_PERMS_SCMI_PERF_MESSAGE_IDX] =
                            ((1
                            << (MOD_SCMI_PERF_DOMAIN_ATTRIBUTES -
                                MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                            (0
                            << (MOD_SCMI_PERF_DESCRIBE_LEVELS -
                                MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                            (1
                            << (MOD_SCMI_PERF_LIMITS_SET -
                                MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                            (1
                            << (MOD_SCMI_PERF_LIMITS_GET -
                                MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                            (1
                            << (MOD_SCMI_PERF_LEVEL_SET -
                                MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                            (1
                            << (MOD_SCMI_PERF_LEVEL_GET -
                                MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                            (1
                            << (MOD_SCMI_PERF_NOTIFY_LIMITS -
                                MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                            (1
                            << (MOD_SCMI_PERF_NOTIFY_LEVEL -
                                MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                            (1
                            << (MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL -
                                MOD_SCMI_PERF_DOMAIN_ATTRIBUTES))),
                        /* Clocks, no access */
                        [MOD_RES_PERMS_SCMI_CLOCK_MESSAGE_IDX] = 0xff,
                        [MOD_RES_PERMS_SCMI_SENSOR_MESSAGE_IDX] = 0x0,
                        [MOD_RES_PERMS_SCMI_RESET_DOMAIN_MESSAGE_IDX] =
                            0x0,
                    },
            },
    };

static struct mod_res_agent_permission agent_permissions = {
    .agent_protocol_permissions = agent_protocol_permissions,
    .agent_msg_permissions = agent_msg_permissions,
};

struct fwk_module_config config_resource_perms = {
    .data =
        &(struct mod_res_resource_perms_config){
            .agent_permissions = (uintptr_t)&agent_permissions,
            .agent_count = SCP_SCMI_AGENT_ID_COUNT,
            .protocol_count = 6,
        },
};

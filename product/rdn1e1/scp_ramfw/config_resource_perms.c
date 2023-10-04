/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_clock.h"
#include "scp_rdn1e1_scmi.h"

#include <mod_resource_perms.h>
#include <mod_scmi_apcore.h>
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

/*!
 * Agent 0 gets access to all resources.
 */
#define AGENT_IDX(agent_id) (agent_id - 1)

/*!
 * Note that permissions are denied when a bit is set.
 */
static struct mod_res_agent_protocol_permissions agent_protocol_permissions[] =
    {
        [AGENT_IDX(SCP_SCMI_AGENT_ID_OSPM)] =
            {
                .protocols = MOD_RES_PERMS_SCMI_ALL_PROTOCOLS_ALLOWED,
            },

        /* PSCI agent has no access to clock, perf and sensor protocol */
        [AGENT_IDX(SCP_SCMI_AGENT_ID_PSCI)] =
            {
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
static struct mod_res_agent_msg_permissions agent_msg_permissions[] = {
    [AGENT_IDX(SCP_SCMI_AGENT_ID_OSPM)] = {
        .messages = {
            /* Example, Base, disable unused msg 12 */
            [MOD_RES_PERMS_SCMI_BASE_MESSAGE_IDX] =
                (1 << 12),  /* Example, Base, disable unused msg 12 */
            /* Power Domain */
            [MOD_RES_PERMS_SCMI_POWER_DOMAIN_MESSAGE_IDX] = 0x0,
            /* System Power Domain */
            [MOD_RES_PERMS_SCMI_SYS_POWER_MESSAGE_IDX] = 0x0,
            /* Performance */
            [MOD_RES_PERMS_SCMI_PERF_MESSAGE_IDX] = 0x0,
            /* Clocks */
            [MOD_RES_PERMS_SCMI_CLOCK_MESSAGE_IDX] = 0x0,
            /* Sensors */
            [MOD_RES_PERMS_SCMI_SENSOR_MESSAGE_IDX] = 0x0,
            /* Reset Domains */
            [MOD_RES_PERMS_SCMI_RESET_DOMAIN_MESSAGE_IDX] = 0x0,
        },
    },

    [AGENT_IDX(SCP_SCMI_AGENT_ID_PSCI)] = {
        .messages = {
            [MOD_RES_PERMS_SCMI_BASE_MESSAGE_IDX] = 0x0,
            [MOD_RES_PERMS_SCMI_POWER_DOMAIN_MESSAGE_IDX] = 0x0,
            [MOD_RES_PERMS_SCMI_SYS_POWER_MESSAGE_IDX] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_MESSAGE_IDX] =
                ((1 << (MOD_SCMI_PERF_DOMAIN_ATTRIBUTES -
                        MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                /* DESCRIBE_LEVELS is required for some reason ... */
                (0 << (MOD_SCMI_PERF_DESCRIBE_LEVELS -
                        MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                (1 << (MOD_SCMI_PERF_LIMITS_SET -
                        MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                (1 << (MOD_SCMI_PERF_LIMITS_GET -
                        MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                (1 << (MOD_SCMI_PERF_LEVEL_SET -
                        MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                (1 << (MOD_SCMI_PERF_LEVEL_GET -
                        MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                (1 << (MOD_SCMI_PERF_NOTIFY_LIMITS -
                        MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                (1 << (MOD_SCMI_PERF_NOTIFY_LEVEL -
                        MOD_SCMI_PERF_DOMAIN_ATTRIBUTES)) |
                (1 << (MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL -
                        MOD_SCMI_PERF_DOMAIN_ATTRIBUTES))),
            /* Clocks, no access */
            [MOD_RES_PERMS_SCMI_CLOCK_MESSAGE_IDX] = 0xff,
            [MOD_RES_PERMS_SCMI_SENSOR_MESSAGE_IDX] = 0x0,
            [MOD_RES_PERMS_SCMI_RESET_DOMAIN_MESSAGE_IDX] = 0x0,
        },
    },
};

/*
 * Protocols have an index offset from SCMI_BASE protocol, 0x10
 * Note that the BASE and SYSTEM_POWER protocols are managed
 * on a protocol:command basis, there is no resource permissions
 * associated with the protocols.
 */

/*
 * We are tracking 5 SCMI Clock Protocol commands
 *
 *  0, SCMI_CLOCK_ATTRIBUTES
 *  1, SCMI_CLOCK_RATE_GET
 *  2, SCMI_CLOCK_RATE_SET
 *  3, SCMI_CLOCK_CONFIG_SET
 *  4, SCMI_CLOCK_DESCRIBE_RATES
 */
#define RDN1E1_CLOCK_RESOURCE_CMDS 5
#define RDN1E1_CLOCK_RESOURCE_ELEMENTS \
    ((CLOCK_IDX_COUNT >> MOD_RES_PERMS_TYPE_SHIFT) + 1)
static mod_res_perms_t scmi_clock_perms[][RDN1E1_CLOCK_RESOURCE_CMDS]
        [RDN1E1_CLOCK_RESOURCE_ELEMENTS] = {
    /* SCMI_PROTOCOL_ID_CLOCK */
    /* 0, SCMI_CLOCK_ATTRIBUTES */
    /* 1, SCMI_CLOCK_RATE_GET */
    /* 2, SCMI_CLOCK_RATE_SET */
    /* 3, SCMI_CLOCK_CONFIG_SET */
    /* 4, SCMI_CLOCK_DESCRIBE_RATES */
    [AGENT_IDX(SCP_SCMI_AGENT_ID_OSPM)] = {
        [MOD_RES_PERMS_SCMI_CLOCK_ATTRIBUTES_IDX][0] = 0x0,
        /*
         * Clocks 0, 1, 2, 4 do not allow set commands,
         * Clocks 3 and 5 allow rate_set/config_set
         */
        [MOD_RES_PERMS_SCMI_CLOCK_RATE_SET_IDX][0] =
            ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 4)),
        [MOD_RES_PERMS_SCMI_CLOCK_RATE_GET_IDX][0] =
            ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 4)),
        [MOD_RES_PERMS_SCMI_CLOCK_CONFIG_SET_IDX][0] = 0x0,
        [MOD_RES_PERMS_SCMI_CLOCK_DESCRIBE_RATE_IDX][0] = 0x0,
    },

    [AGENT_IDX(SCP_SCMI_AGENT_ID_PSCI)] = {
        /* No access to clocks for PSCI agent, set bits [4:0] */
        [MOD_RES_PERMS_SCMI_CLOCK_ATTRIBUTES_IDX][0] = 0x1f,
        [MOD_RES_PERMS_SCMI_CLOCK_RATE_SET_IDX][0] = 0x1f,
        [MOD_RES_PERMS_SCMI_CLOCK_RATE_GET_IDX][0] = 0x1f,
        [MOD_RES_PERMS_SCMI_CLOCK_CONFIG_SET_IDX][0] = 0x1f,
        [MOD_RES_PERMS_SCMI_CLOCK_DESCRIBE_RATE_IDX][0] = 0x1f,
    },
};

/*
 * Check whether an agent has access to a protocol.
 *
 * Note that we will always check the higher permissions levels
 * when called, so
 *
 *      protocol -> message -> resource
 *
 * This overrides the version in the resource_perms module
 * for the platform specific protocols.
 */
enum mod_res_perms_permissions mod_res_plat_agent_protocol_permissions(
    uint32_t agent_id,
    uint32_t protocol_id)
{
    if (protocol_id == MOD_SCMI_PROTOCOL_ID_APCORE)
        return MOD_RES_PERMS_ACCESS_ALLOWED;

    return MOD_RES_PERMS_ACCESS_DENIED;
}

/*
 * Check whether an agent can access a protocol:message.
 *
 * This overrides the version in the resource_perms module
 * for the platform specific protocol:messages.
 */
enum mod_res_perms_permissions mod_res_plat_agent_message_permissions(
    uint32_t agent_id,
    uint32_t protocol_id,
    uint32_t message_id)
{
    if ((protocol_id == MOD_SCMI_PROTOCOL_ID_APCORE) &&
        (message_id <= MOD_SCMI_APCORE_RESET_ADDRESS_GET))
        return MOD_RES_PERMS_ACCESS_ALLOWED;

    return MOD_RES_PERMS_ACCESS_DENIED;
}

/*
 * Check the permissions for agent:protocol:message:resource.
 *
 * This overrides the version in the resource_perms module
 * for the platform specific protocol:message:resources.
 */
enum mod_res_perms_permissions mod_res_plat_agent_resource_permissions(
    uint32_t agent_id,
    uint32_t protocol_id,
    uint32_t message_id,
    uint32_t resource_id)
{
    if ((protocol_id == MOD_SCMI_PROTOCOL_ID_APCORE) &&
        (message_id <= MOD_SCMI_APCORE_RESET_ADDRESS_GET))
        return MOD_RES_PERMS_ACCESS_ALLOWED;

    return MOD_RES_PERMS_ACCESS_DENIED;
}
static struct mod_res_agent_permission agent_permissions = {
    .agent_protocol_permissions = agent_protocol_permissions,
    .agent_msg_permissions = agent_msg_permissions,
    .scmi_clock_perms = &scmi_clock_perms[0][0][0],
};

struct fwk_module_config config_resource_perms = {
    .data =
        &(struct mod_res_resource_perms_config){
            .agent_permissions = (uintptr_t)&agent_permissions,
            .agent_count = SCP_SCMI_AGENT_ID_COUNT,
            .protocol_count = 6,
            .clock_counters.count = CLOCK_IDX_COUNT,
        },
};

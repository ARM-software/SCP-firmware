/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2021-2023, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rcar_scmi.h"
#include "clock_devices.h"
#include "reset_devices.h"
#include "config_power_domain.h"
#include "config_dvfs.h"
#include "config_sensor.h"
#include "config_resource_perms.h"

#include <mod_resource_perms.h>
#include <mod_scmi_std.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/*!
 * If the agent wants to modify permissions at run-time these tables
 * must be allocated in wrtiable memory.
 */

/*!
 * Agent 0 gets access to all resources.
 */
#define AGENT_IDX(agent_id) (agent_id - 1)

static struct mod_res_agent_protocol_permissions agent_protocol_permissions[] =
    {
        [AGENT_IDX(SCMI_AGENT_ID_OSPM)] =
            {
                .protocols = MOD_RES_PERMS_SCMI_ALL_PROTOCOLS_ALLOWED,
            },

        /*
         * PSCI agent has no access to clock, perf and reset and sensor protocol
         */
        [AGENT_IDX(SCMI_AGENT_ID_PSCI)] =
            {
                .protocols = MOD_RES_PERMS_SCMI_CLOCK_PROTOCOL_DENIED |
                    MOD_RES_PERMS_SCMI_PERF_PROTOCOL_DENIED |
                    MOD_RES_PERMS_SCMI_RESET_DOMAIN_PROTOCOL_DENIED |
                    MOD_RES_PERMS_SCMI_SENSOR_PROTOCOL_DENIED,
            },
        [AGENT_IDX(SCMI_AGENT_ID_VMM)] =
            {
                .protocols = MOD_RES_PERMS_SCMI_ALL_PROTOCOLS_ALLOWED,
            },
        [AGENT_IDX(SCMI_AGENT_ID_VM1)] =
            {
                .protocols = MOD_RES_PERMS_SCMI_ALL_PROTOCOLS_ALLOWED,
            },
        [AGENT_IDX(SCMI_AGENT_ID_VM2)] =
            {
                .protocols = MOD_RES_PERMS_SCMI_ALL_PROTOCOLS_ALLOWED,
            },
    };

/*
 * Messages have an index offset from 0x3 as all agents can access
 * the VERSION/ATTRIBUTES/MSG_ATTRIBUTES messages for all
 * protocols, hence message 0x3 maps to bit[0], message 0x4 maps
 * to bit[1], etc.
 */
static struct mod_res_agent_msg_permissions agent_msg_permissions[] = {
    [AGENT_IDX(SCMI_AGENT_ID_OSPM)] = {
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
    [AGENT_IDX(SCMI_AGENT_ID_PSCI)] = {
        .messages = {
            [MOD_RES_PERMS_SCMI_BASE_MESSAGE_IDX] = 0x0,
            [MOD_RES_PERMS_SCMI_POWER_DOMAIN_MESSAGE_IDX] = 0x0,
            [MOD_RES_PERMS_SCMI_SYS_POWER_MESSAGE_IDX] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_MESSAGE_IDX] =
                ((1 << (MOD_SCMI_PERF_DOMAIN_ATTRIBUTES -
                        MOD_RES_PERMS_SCMI_PERF_BITMASK_IDX)) |
                /* DESCRIBE_LEVELS is required for some reason ... */
                (0 << (MOD_SCMI_PERF_DESCRIBE_LEVELS -
                        MOD_RES_PERMS_SCMI_PERF_BITMASK_IDX)) |
                (1 << (MOD_SCMI_PERF_LIMITS_SET -
                        MOD_RES_PERMS_SCMI_PERF_BITMASK_IDX)) |
                (1 << (MOD_SCMI_PERF_LIMITS_GET -
                        MOD_RES_PERMS_SCMI_PERF_BITMASK_IDX)) |
                (1 << (MOD_SCMI_PERF_LEVEL_SET -
                        MOD_RES_PERMS_SCMI_PERF_BITMASK_IDX)) |
                (1 << (MOD_SCMI_PERF_LEVEL_GET -
                        MOD_RES_PERMS_SCMI_PERF_BITMASK_IDX)) |
                (1 << (MOD_SCMI_PERF_NOTIFY_LIMITS -
                        MOD_RES_PERMS_SCMI_PERF_BITMASK_IDX)) |
                (1 << (MOD_SCMI_PERF_NOTIFY_LEVEL -
                        MOD_RES_PERMS_SCMI_PERF_BITMASK_IDX)) |
                (1 << (MOD_SCMI_PERF_DESCRIBE_FAST_CHANNEL -
                        MOD_RES_PERMS_SCMI_PERF_BITMASK_IDX))),
            /* Clocks, no access */
            [MOD_RES_PERMS_SCMI_CLOCK_MESSAGE_IDX] = 0xff,
            [MOD_RES_PERMS_SCMI_SENSOR_MESSAGE_IDX] = 0x0,
            [MOD_RES_PERMS_SCMI_RESET_DOMAIN_MESSAGE_IDX] = 0x0,
        },

    },
    [AGENT_IDX(SCMI_AGENT_ID_VMM)] = {
        .messages = {
            /* Example, Base, disable unused msg 12 */
            [MOD_RES_PERMS_SCMI_BASE_MESSAGE_IDX] = (1 << 12),
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
    [AGENT_IDX(SCMI_AGENT_ID_VM1)] = {
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
    [AGENT_IDX(SCMI_AGENT_ID_VM2)] = {
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
#define RCAR_CLOCK_RESOURCE_CMDS 5
#define RCAR_CLOCK_RESOURCE_ELEMENTS \
    ((CLOCK_RCAR_COUNT >> MOD_RES_PERMS_TYPE_SHIFT) + 1)
static mod_res_perms_t scmi_clock_perms[]
    [RCAR_CLOCK_RESOURCE_CMDS][RCAR_CLOCK_RESOURCE_ELEMENTS] = {
        /* SCMI_PROTOCOL_ID_CLOCK */
        /* 0, SCMI_CLOCK_ATTRIBUTES */
        /* 1, SCMI_CLOCK_RATE_GET */
        /* 2, SCMI_CLOCK_RATE_SET */
        /* 3, SCMI_CLOCK_CONFIG_SET */
        /* 4, SCMI_CLOCK_DESCRIBE_RATES */
        [AGENT_IDX(SCMI_AGENT_ID_OSPM)] = {
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
        [AGENT_IDX(SCMI_AGENT_ID_PSCI)] = {
            /* No access to clocks for PSCI agent, so bits [4:0] set  */
            [MOD_RES_PERMS_SCMI_CLOCK_ATTRIBUTES_IDX][0] = 0x1f,
            [MOD_RES_PERMS_SCMI_CLOCK_RATE_SET_IDX][0] = 0x1f,
            [MOD_RES_PERMS_SCMI_CLOCK_RATE_GET_IDX][0] = 0x1f,
            [MOD_RES_PERMS_SCMI_CLOCK_CONFIG_SET_IDX][0] = 0x1f,
            [MOD_RES_PERMS_SCMI_CLOCK_DESCRIBE_RATE_IDX][0] = 0x1f,
        },
        [AGENT_IDX(SCMI_AGENT_ID_VMM)] = {
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
        [AGENT_IDX(SCMI_AGENT_ID_VM1)] = {
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
        [AGENT_IDX(SCMI_AGENT_ID_VM2)] = {
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
};

/*
 * We are tracking 4 SCMI Power Domain Protocol commands
 *
 *  0, SCMI_PD_POWER_DOMAIN_ATTRIBUTES
 *  1, SCMI_PD_POWER_STATE_SET
 *  2, SCMI_PD_POWER_STATE_GET
 *  3, SCMI_PD_POWER_STATE_NOTIFY
 */
#define RCAR_PD_RESOURCE_CMDS 4
#define RCAR_PD_RESOURCE_ELEMENTS \
    ((PD_RCAR_COUNT >> MOD_RES_PERMS_TYPE_SHIFT) + 1)
static mod_res_perms_t
    scmi_pd_perms[][RCAR_PD_RESOURCE_CMDS][RCAR_PD_RESOURCE_ELEMENTS] = {
        /* SCMI_PROTOCOL_ID_POWER_DOMAIN */
        /* 0, SCMI_PD_POWER_DOMAIN_ATTRIBUTES */
        /* 1, SCMI_PD_POWER_STATE_SET */
        /* 2, SCMI_PD_POWER_STATE_GET */
        /* 3, SCMI_PD_POWER_STATE_NOTIFY */
        [AGENT_IDX(SCMI_AGENT_ID_OSPM)] = {
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_ATTRIBUTES_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_GET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_NOTIFY_IDX][0] = 0x0,
            },
        [AGENT_IDX(SCMI_AGENT_ID_PSCI)] = {
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_ATTRIBUTES_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_GET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_NOTIFY_IDX][0] = 0x0,
            },
        [AGENT_IDX(SCMI_AGENT_ID_VMM)] = {
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_ATTRIBUTES_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_GET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_NOTIFY_IDX][0] = 0x0,
            },
        [AGENT_IDX(SCMI_AGENT_ID_VM1)] = {
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_ATTRIBUTES_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_GET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_NOTIFY_IDX][0] = 0x0,
            },
        [AGENT_IDX(SCMI_AGENT_ID_VM2)] = {
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_ATTRIBUTES_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_GET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_NOTIFY_IDX][0] = 0x0,
            },
};

/*
 * We are tracking 9 SCMI Performance Protocol commands
 *
 * 0, SCMI_PERF_DOMAIN_ATTRIBUTES
 * 1, SCMI_PERF_DESCRIBE_LEVELS
 * 2, SCMI_PERF_LIMITS_SET
 * 3, SCMI_PERF_LIMITS_GET
 * 4, SCMI_PERF_LEVEL_SET
 * 5, SCMI_PERF_LEVEL_GET
 * 6, SCMI_PERF_NOTIFY_LIMITS
 * 7, SCMI_PERF_NOTIFY_LEVEL
 * 8, SCMI_PERF_DESCRIBE_FAST_CHANNEL
 */
#define RCAR_PERF_RESOURCE_CMDS 9
#define RCAR_PERF_RESOURCE_ELEMENTS \
    ((DVFS_ELEMENT_IDX_COUNT >> MOD_RES_PERMS_TYPE_SHIFT) + 1)
static mod_res_perms_t
    scmi_perf_perms[][RCAR_PERF_RESOURCE_CMDS][RCAR_PERF_RESOURCE_ELEMENTS] = {
        /* SCMI_PROTOCOL_ID_PERF */
        /* 0, SCMI_PERF_DOMAIN_ATTRIBUTES */
        /* 1, SCMI_PERF_DESCRIBE_LEVELS */
        /* 2, SCMI_PERF_LIMITS_SET */
        /* 3, SCMI_PERF_LIMITS_GET */
        /* 4, SCMI_PERF_LEVEL_SET */
        /* 5, SCMI_PERF_LEVEL_GET */
        /* 6, SCMI_PERF_NOTIFY_LIMITS */
        /* 7, SCMI_PERF_NOTIFY_LEVEL */
        /* 8, SCMI_PERF_DESCRIBE_FAST_CHANNEL */
        [AGENT_IDX(SCMI_AGENT_ID_OSPM)] = {
            [MOD_RES_PERMS_SCMI_PERF_ATTRIBUTES_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_DESCRIBE_LEVELS_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LIMITS_SET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LIMITS_GET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LEVEL_SET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LEVEL_GET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_NOTIFY_LIMITS_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_NOTIFY_LEVEL_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_DESCRIBE_FAST_CHANNEL_IDX][0] = 0x0,
        },
        [AGENT_IDX(SCMI_AGENT_ID_PSCI)] = {
            [MOD_RES_PERMS_SCMI_PERF_ATTRIBUTES_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_DESCRIBE_LEVELS_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LIMITS_SET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LIMITS_GET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LEVEL_SET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LEVEL_GET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_NOTIFY_LIMITS_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_NOTIFY_LEVEL_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_DESCRIBE_FAST_CHANNEL_IDX][0] = 0x0,
        },
        [AGENT_IDX(SCMI_AGENT_ID_VMM)] = {
            [MOD_RES_PERMS_SCMI_PERF_ATTRIBUTES_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_DESCRIBE_LEVELS_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LIMITS_SET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LIMITS_GET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LEVEL_SET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LEVEL_GET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_NOTIFY_LIMITS_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_NOTIFY_LEVEL_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_DESCRIBE_FAST_CHANNEL_IDX][0] = 0x0,
        },
        [AGENT_IDX(SCMI_AGENT_ID_VM1)] = {
            [MOD_RES_PERMS_SCMI_PERF_ATTRIBUTES_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_DESCRIBE_LEVELS_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LIMITS_SET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LIMITS_GET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LEVEL_SET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LEVEL_GET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_NOTIFY_LIMITS_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_NOTIFY_LEVEL_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_DESCRIBE_FAST_CHANNEL_IDX][0] = 0x0,
        },
        [AGENT_IDX(SCMI_AGENT_ID_VM2)] = {
            [MOD_RES_PERMS_SCMI_PERF_ATTRIBUTES_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_DESCRIBE_LEVELS_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LIMITS_SET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LIMITS_GET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LEVEL_SET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_LEVEL_GET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_NOTIFY_LIMITS_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_NOTIFY_LEVEL_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_PERF_DESCRIBE_FAST_CHANNEL_IDX][0] = 0x0,
        },
};

/*
 * We are tracking 4 SCMI Sensor Protocol commands
 *
 * 0, SCMI_SENSOR_DESCRIPTION_GET
 * 1, SCMI_SENSOR_CONFIG_SET
 * 2, SCMI_SENSOR_TRIP_POINT_SET
 * 3, SCMI_SENSOR_READING_GET
 */
#define RCAR_SENSOR_RESOURCE_CMDS 4
#define RCAR_SENSOR_RESOURCE_ELEMENTS \
    ((R8A7795_SNSR_COUNT >> MOD_RES_PERMS_TYPE_SHIFT) + 1)
static mod_res_perms_t
    scmi_sensor_perms[][RCAR_SENSOR_RESOURCE_CMDS]
        [RCAR_SENSOR_RESOURCE_ELEMENTS] = {
            /* SCMI_PROTOCOL_ID_SENSOR */
            /* 0, SCMI_SENSOR_DESCRIPTION_GET */
            /* 1, SCMI_SENSOR_CONFIG_SET */
            /* 2, SCMI_SENSOR_TRIP_POINT_SET */
            /* 3, SCMI_SENSOR_READING_GET */
            [AGENT_IDX(SCMI_AGENT_ID_OSPM)] = {
                [MOD_RES_PERMS_SCMI_SENSOR_DESCRIPTION_GET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_CONFIG_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_TRIP_POINT_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_READING_GET_IDX][0] = 0x0,
            },
            [AGENT_IDX(SCMI_AGENT_ID_PSCI)] = {
                [MOD_RES_PERMS_SCMI_SENSOR_DESCRIPTION_GET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_CONFIG_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_TRIP_POINT_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_READING_GET_IDX][0] = 0x0,
            },
            [AGENT_IDX(SCMI_AGENT_ID_VMM)] = {
                [MOD_RES_PERMS_SCMI_SENSOR_DESCRIPTION_GET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_CONFIG_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_TRIP_POINT_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_READING_GET_IDX][0] = 0x0,
            },
            [AGENT_IDX(SCMI_AGENT_ID_VM1)] = {
                [MOD_RES_PERMS_SCMI_SENSOR_DESCRIPTION_GET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_CONFIG_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_TRIP_POINT_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_READING_GET_IDX][0] = 0x0,
            },
            [AGENT_IDX(SCMI_AGENT_ID_VM2)] = {
                [MOD_RES_PERMS_SCMI_SENSOR_DESCRIPTION_GET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_CONFIG_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_TRIP_POINT_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_READING_GET_IDX][0] = 0x0,
            },
    };

/*
 * We are tracking 3 SCMI Reset Domain Protocol commands
 *
 * 0, MOD_SCMI_RESET_DOMAIN_ATTRIBUTES
 * 1, MOD_SCMI_RESET_REQUEST
 * 2, MOD_SCMI_RESET_NOTIFY
 */
#ifdef BUILD_HAS_SCMI_RESET

#    define RCAR_RESET_DOMAIN_RESOURCE_CMDS 3
#    define RCAR_RESET_DOMAIN_RESOURCE_ELEMENTS \
        ((RESET_RCAR_COUNT >> MOD_RES_PERMS_TYPE_SHIFT) + 1)
static mod_res_perms_t
    scmi_reset_domain_perms[] [RCAR_RESET_DOMAIN_RESOURCE_CMDS]
        [RCAR_RESET_DOMAIN_RESOURCE_ELEMENTS] = {
     /* SCMI_PROTOCOL_ID_RESET_DOMAIN */
     /* 0, MOD_SCMI_RESET_DOMAIN_ATTRIBUTES */
     /* 1, MOD_SCMI_RESET_REQUEST */
     /* 2, MOD_SCMI_RESET_NOTIFY */
     [AGENT_IDX(SCMI_AGENT_ID_OSPM)] = {
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_ATTRIBUTES_IDX][0] = 0x0,
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_REQUEST_IDX][0] = 0x0,
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_NOTIFY_IDX][0] = 0x0,
     },
     [AGENT_IDX(SCMI_AGENT_ID_PSCI)] = {
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_ATTRIBUTES_IDX][0] = 0x0,
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_REQUEST_IDX][0] = 0x0,
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_NOTIFY_IDX][0] = 0x0,
     },
     [AGENT_IDX(SCMI_AGENT_ID_VMM)] = {
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_ATTRIBUTES_IDX][0] = 0x0,
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_REQUEST_IDX][0] = 0x0,
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_NOTIFY_IDX][0] = 0x0,
     },
     [AGENT_IDX(SCMI_AGENT_ID_VM1)] = {
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_ATTRIBUTES_IDX][0] = 0x0,
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_REQUEST_IDX][0] = 0x0,
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_NOTIFY_IDX][0] = 0x0,
     },
     [AGENT_IDX(SCMI_AGENT_ID_VM2)] = {
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_ATTRIBUTES_IDX][0] = 0x0,
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_REQUEST_IDX][0] = 0x0,
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_NOTIFY_IDX][0] = 0x0,
     },
 };

#endif

/*
 * rcar Platform Permissions
 */
static struct mod_res_agent_permission agent_permissions = {
    .agent_protocol_permissions = agent_protocol_permissions,
    .agent_msg_permissions = agent_msg_permissions,
    .scmi_clock_perms = &scmi_clock_perms[0][0][0],
    .scmi_pd_perms = &scmi_pd_perms[0][0][0],
    .scmi_perf_perms = &scmi_perf_perms[0][0][0],
    .scmi_sensor_perms = &scmi_sensor_perms[0][0][0],
#ifdef BUILD_HAS_SCMI_RESET
    .scmi_reset_domain_perms = &scmi_reset_domain_perms[0][0][0],
#endif
};

/*
 * rcar Platform devices
 *
 * Note that a device must be terminated with
 *  {FWK_ID_NONE, MOD_RES_DOMAIN_DEVICE_INVALID}
 *
 */
#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
static struct mod_res_domain_device devices_cpu[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SCMI_PERF, DVFS_ELEMENT_IDX_BIG),
        .type = MOD_RES_PERF_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SCMI_PERF,
                DVFS_ELEMENT_IDX_LITTLE),
        .type = MOD_RES_PERF_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_ether[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_ETHERAVB),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_ETHERAVB),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_S0D3),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_S0D6),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_hdmi0[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_HDMI0),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_HDMI0),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_hdmi1[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_HDMI1),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_HDMI1),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_i2c0[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_I2C0),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_I2C0),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_i2c1[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_I2C1),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_I2C1),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_i2c2[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_I2C2),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_I2C2),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_i2c3[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_I2C3),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_I2C3),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_i2c4[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_I2C4),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_I2C4),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_gpio0[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_GPIO0),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_GPIO0),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_gpio1[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_GPIO1),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_GPIO1),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_gpio2[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_GPIO2),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_GPIO2),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_sdhi0[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_SDIF0),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_SDIF0),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_SD0),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_sdhi1[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_SDIF1),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_SDIF1),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_SD1),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_sdhi2[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_SDIF2),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_SDIF2),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_SD2),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_sdhi3[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_SDIF3),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_SDIF3),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_SD3),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_intc_ap[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_INTC_AP),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_INTC_AP),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_dmac0[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_SYS_DMAC0),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_SYS_DMAC0),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_dmac1[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_SYS_DMAC1),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_SYS_DMAC1),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_dma2c[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_SYS_DMAC2),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_SYS_DMAC2),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_audmac0[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_AUDMAC0),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_AUDMAC0),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_audmac1[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_AUDMAC1),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_AUDMAC1),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_usb0[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_USB_DMAC0),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_USB_DMAC1),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_HSUSB),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_EHCI0),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_USB_DMAC0),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_USB_DMAC1),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_HSUSB),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_EHCI0),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_usb1[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_USB_DMAC0),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_USB_DMAC1),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_HSUSB),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_EHCI1),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_USB_DMAC0),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_USB_DMAC1),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_HSUSB),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_EHCI1),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_usb2[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_USB_DMAC30),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_USB_DMAC31),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_HSUSB3),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_EHCI2),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_USB_DMAC30),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_USB_DMAC31),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_HSUSB),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_EHCI2),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_usb3[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_USB_DMAC30),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_USB_DMAC31),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_HSUSB3),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN, RESET_RCAR_EHCI3),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RESET_DOMAIN,
                RESET_RCAR_USB3_IF0),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_USB_DMAC30),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_USB_DMAC31),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_HSUSB3),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_EHCI3),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLOCK_RCAR_USB3_IF0),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};
#endif /* BUILD_HAS_RESOURCE_PERMISSIONS */

static struct mod_res_device rcar_devices[] = {
#ifdef BUILD_HAS_RESOURCE_PERMISSIONS
    [RCAR_RES_PERMS_DEVICES_CPU] = {
        .device_id = RCAR_RES_PERMS_DEVICES_CPU,
        .domain_devices = devices_cpu,
    },
    [RCAR_RES_PERMS_DEVICES_ETHERAVB] = {
        .device_id = RCAR_RES_PERMS_DEVICES_ETHERAVB,
        .domain_devices = devices_ether,
    },
    [RCAR_RES_PERMS_DEVICES_HDMI0] = {
        .device_id = RCAR_RES_PERMS_DEVICES_HDMI0,
        .domain_devices = devices_hdmi0,
    },
    [RCAR_RES_PERMS_DEVICES_HDMI1] = {
        .device_id = RCAR_RES_PERMS_DEVICES_HDMI1,
        .domain_devices = devices_hdmi1,
    },
    [RCAR_RES_PERMS_DEVICES_I2C0] = {
        .device_id = RCAR_RES_PERMS_DEVICES_I2C0,
        .domain_devices = devices_i2c0,
    },
    [RCAR_RES_PERMS_DEVICES_I2C1] = {
        .device_id = RCAR_RES_PERMS_DEVICES_I2C1,
        .domain_devices = devices_i2c1,
    },
    [RCAR_RES_PERMS_DEVICES_I2C2] = {
        .device_id = RCAR_RES_PERMS_DEVICES_I2C2,
        .domain_devices = devices_i2c2,
    },
    [RCAR_RES_PERMS_DEVICES_I2C3] = {
        .device_id = RCAR_RES_PERMS_DEVICES_I2C3,
        .domain_devices = devices_i2c3,
    },
    [RCAR_RES_PERMS_DEVICES_I2C4] = {
        .device_id = RCAR_RES_PERMS_DEVICES_I2C4,
        .domain_devices = devices_i2c4,
    },
    [RCAR_RES_PERMS_DEVICES_GPIO0] = {
        .device_id = RCAR_RES_PERMS_DEVICES_GPIO0,
        .domain_devices = devices_gpio0,
    },
    [RCAR_RES_PERMS_DEVICES_GPIO1] = {
        .device_id = RCAR_RES_PERMS_DEVICES_GPIO1,
        .domain_devices = devices_gpio1,
    },
    [RCAR_RES_PERMS_DEVICES_GPIO2] = {
        .device_id = RCAR_RES_PERMS_DEVICES_GPIO2,
        .domain_devices = devices_gpio2,
    },
    [RCAR_RES_PERMS_DEVICES_SDHI0] = {
        .device_id = RCAR_RES_PERMS_DEVICES_SDHI0,
        .domain_devices = devices_sdhi0,
    },
    [RCAR_RES_PERMS_DEVICES_SDHI1] = {
        .device_id = RCAR_RES_PERMS_DEVICES_SDHI1,
        .domain_devices = devices_sdhi1,
    },
    [RCAR_RES_PERMS_DEVICES_SDHI2] = {
        .device_id = RCAR_RES_PERMS_DEVICES_SDHI2,
        .domain_devices = devices_sdhi2,
    },
    [RCAR_RES_PERMS_DEVICES_SDHI3] = {
        .device_id = RCAR_RES_PERMS_DEVICES_SDHI3,
        .domain_devices = devices_sdhi3,
    },
    /* Actually, INTC_AP should be directly controlled by Xen */
    [RCAR_RES_PERMS_DEVICES_INTC_AP] = {
        .device_id = RCAR_RES_PERMS_DEVICES_INTC_AP,
        .domain_devices = devices_intc_ap,
    },
    [RCAR_RES_PERMS_DEVICES_DMAC0] = {
        .device_id = RCAR_RES_PERMS_DEVICES_DMAC0,
        .domain_devices = devices_dmac0,
    },
    [RCAR_RES_PERMS_DEVICES_DMAC1] = {
        .device_id = RCAR_RES_PERMS_DEVICES_DMAC1,
        .domain_devices = devices_dmac1,
    },
    [RCAR_RES_PERMS_DEVICES_DMAC2] = {
        .device_id = RCAR_RES_PERMS_DEVICES_DMAC2,
        .domain_devices = devices_dma2c,
    },
    [RCAR_RES_PERMS_DEVICES_AUDMAC0] = {
        .device_id = RCAR_RES_PERMS_DEVICES_AUDMAC0,
        .domain_devices = devices_audmac0,
    },
    [RCAR_RES_PERMS_DEVICES_AUDMAC1] = {
        .device_id = RCAR_RES_PERMS_DEVICES_AUDMAC1,
        .domain_devices = devices_audmac1,
    },
    [RCAR_RES_PERMS_DEVICES_USB0] = {
        .device_id = RCAR_RES_PERMS_DEVICES_USB0,
        .domain_devices = devices_usb0,
    },
    [RCAR_RES_PERMS_DEVICES_USB1] = {
        .device_id = RCAR_RES_PERMS_DEVICES_USB1,
        .domain_devices = devices_usb1,
    },
    [RCAR_RES_PERMS_DEVICES_USB2] = {
        .device_id = RCAR_RES_PERMS_DEVICES_USB2,
        .domain_devices = devices_usb2,
    },
    [RCAR_RES_PERMS_DEVICES_USB3] = {
        .device_id = RCAR_RES_PERMS_DEVICES_USB3,
        .domain_devices = devices_usb3,
    },
#endif /* BUILD_HAS_RESOURCE_PERMISSIONS */
    { 0 },
};

struct fwk_module_config config_resource_perms = {
    .data =
        &(struct mod_res_resource_perms_config){
            .agent_permissions = (uintptr_t)&agent_permissions,
            .agent_count = SCMI_AGENT_ID_COUNT,
            .protocol_count = 7,
            .clock_counters.count = CLOCK_RCAR_COUNT,
            .sensor_counters.count = R8A7795_SNSR_COUNT,
            .pd_counters.count = PD_RCAR_COUNT,
            .perf_counters.count = DVFS_ELEMENT_IDX_COUNT,
            .perf_counters.cmd_count = RCAR_PERF_RESOURCE_CMDS,
            .perf_counters.resource_count = RCAR_PERF_RESOURCE_ELEMENTS,
            .device_count = RCAR_RES_PERMS_DEVICES_COUNT,
#ifdef BUILD_HAS_SCMI_RESET
            .reset_domain_count = RESET_RCAR_COUNT,
#endif
            .domain_devices = (uintptr_t)&rcar_devices,
        },
};

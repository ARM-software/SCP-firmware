/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_dvfs.h"
#include "config_power_domain.h"
#include "config_resource_perms.h"
#include "config_sensor.h"
#include "juno_clock.h"
#include "juno_scmi.h"

#ifdef BUILD_HAS_MOD_SCMI_RESET_DOMAIN
#    include <mod_juno_reset_domain.h>
#endif

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

/*!
 * Note that permissions are denied when a bit is set, the
 * permissions tables are being added for Juno as an example only.
 */
static struct mod_res_agent_protocol_permissions agent_protocol_permissions[] =
    {
        [AGENT_IDX(JUNO_SCMI_AGENT_IDX_OSPM)] =
            {
                .protocols =
                    (mod_res_perms_t)MOD_RES_PERMS_SCMI_ALL_PROTOCOLS_ALLOWED,
            },

        /* PSCI agent has no access to clock, perf and sensor protocol */
        [AGENT_IDX(JUNO_SCMI_AGENT_IDX_PSCI)] =
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
    [AGENT_IDX(JUNO_SCMI_AGENT_IDX_OSPM)] = {
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
        }
    },
    [AGENT_IDX(JUNO_SCMI_AGENT_IDX_PSCI)] = {
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
        }
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
#define JUNO_CLOCK_RESOURCE_CMDS 5
#define JUNO_CLOCK_RESOURCE_ELEMENTS \
    ((JUNO_CLOCK_IDX_COUNT >> MOD_RES_PERMS_TYPE_SHIFT) + 1)
static mod_res_perms_t scmi_clock_perms[]
    [JUNO_CLOCK_RESOURCE_CMDS][JUNO_CLOCK_RESOURCE_ELEMENTS] = {
        /* SCMI_PROTOCOL_ID_CLOCK */
        /* 0, SCMI_CLOCK_ATTRIBUTES */
        /* 1, SCMI_CLOCK_RATE_GET */
        /* 2, SCMI_CLOCK_RATE_SET */
        /* 3, SCMI_CLOCK_CONFIG_SET */
        /* 4, SCMI_CLOCK_DESCRIBE_RATES */
        [AGENT_IDX(JUNO_SCMI_AGENT_IDX_OSPM)] = {
            [MOD_RES_PERMS_SCMI_CLOCK_ATTRIBUTES_IDX][0] = 0x0,
            /*
             * Clocks 0, 1, 2, 4 do not allow set commands,
             * Clocks 3 and 5 allow rate_set/config_set
             */
            [MOD_RES_PERMS_SCMI_CLOCK_RATE_SET_IDX][0] =
                ((1 << 0) | (1 << 1) | (1 << 2) | (1 << 4)),
            [MOD_RES_PERMS_SCMI_CLOCK_RATE_GET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_CLOCK_CONFIG_SET_IDX][0] = 0x0,
            [MOD_RES_PERMS_SCMI_CLOCK_DESCRIBE_RATE_IDX][0] = 0x0,
        },
        [AGENT_IDX(JUNO_SCMI_AGENT_IDX_PSCI)] = {
            /* No access to clocks for PSCI agent, so bits [4:0] set  */
            [MOD_RES_PERMS_SCMI_CLOCK_ATTRIBUTES_IDX][0] = 0x1f,
            [MOD_RES_PERMS_SCMI_CLOCK_RATE_SET_IDX][0] = 0x1f,
            [MOD_RES_PERMS_SCMI_CLOCK_RATE_GET_IDX][0] = 0x1f,
            [MOD_RES_PERMS_SCMI_CLOCK_CONFIG_SET_IDX][0] = 0x1f,
            [MOD_RES_PERMS_SCMI_CLOCK_DESCRIBE_RATE_IDX][0] = 0x1f,
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
#define JUNO_PD_RESOURCE_CMDS 4
#define JUNO_PD_RESOURCE_ELEMENTS \
    ((POWER_DOMAIN_IDX_COUNT >> MOD_RES_PERMS_TYPE_SHIFT) + 1)
static mod_res_perms_t
    scmi_pd_perms[][JUNO_PD_RESOURCE_CMDS][JUNO_PD_RESOURCE_ELEMENTS] = {
        /* SCMI_PROTOCOL_ID_POWER_DOMAIN */
        /* 0, SCMI_PD_POWER_DOMAIN_ATTRIBUTES */
        /* 1, SCMI_PD_POWER_STATE_SET */
        /* 2, SCMI_PD_POWER_STATE_GET */
        /* 3, SCMI_PD_POWER_STATE_NOTIFY */
        /* For Juno, resource_id == power domain ID */
        [AGENT_IDX(JUNO_SCMI_AGENT_IDX_OSPM)] = {
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_ATTRIBUTES_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_SET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_GET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_POWER_DOMAIN_STATE_NOTIFY_IDX][0] = 0x0,
            },
        [AGENT_IDX(JUNO_SCMI_AGENT_IDX_PSCI)] = {
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
#define JUNO_PERF_RESOURCE_CMDS 9
#define JUNO_PERF_RESOURCE_ELEMENTS \
    ((DVFS_ELEMENT_IDX_COUNT >> MOD_RES_PERMS_TYPE_SHIFT) + 1)
static mod_res_perms_t
    scmi_perf_perms[][JUNO_PERF_RESOURCE_CMDS][JUNO_PERF_RESOURCE_ELEMENTS] = {
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
        /*
         * For Juno, resource_id == performance domain_id
         * see mod_scmi_perf_domain_config domains[]
         */
        [AGENT_IDX(JUNO_SCMI_AGENT_IDX_OSPM)] = {
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
        [AGENT_IDX(JUNO_SCMI_AGENT_IDX_PSCI)] = {
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
 * 1, SCMI_SENSOR_TRIP_POINT_NOTIFY
 * 2, SCMI_SENSOR_TRIP_POINT_CONFIG
 * 3, SCMI_SENSOR_READING_GET
 */
#define JUNO_SENSOR_RESOURCE_CMDS 4
#define JUNO_SENSOR_RESOURCE_ELEMENTS \
    ((MOD_JUNO_R1_R2_SENSOR_IDX_COUNT >> MOD_RES_PERMS_TYPE_SHIFT) + 1)
static mod_res_perms_t
    scmi_sensor_perms[][JUNO_SENSOR_RESOURCE_CMDS]
        [JUNO_SENSOR_RESOURCE_ELEMENTS] = {
            /* SCMI_PROTOCOL_ID_SENSOR */
            /* 0, SCMI_SENSOR_DESCRIPTION_GET */
            /* 1, SCMI_SENSOR_TRIP_POINT_NOTIFY */
            /* 2, SCMI_SENSOR_TRIP_POINT_CONFIG */
            /* 3, SCMI_SENSOR_READING_GET */
            [AGENT_IDX(JUNO_SCMI_AGENT_IDX_OSPM)] = {
                [MOD_RES_PERMS_SCMI_SENSOR_DESCRIPTION_GET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_TRIP_POINT_NOTIFY_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_TRIP_POINT_CONFIG_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_READING_GET_IDX][0] = 0x0,
            },
            [AGENT_IDX(JUNO_SCMI_AGENT_IDX_PSCI)] = {
                [MOD_RES_PERMS_SCMI_SENSOR_DESCRIPTION_GET_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_TRIP_POINT_NOTIFY_IDX][0] = 0x0,
                [MOD_RES_PERMS_SCMI_SENSOR_TRIP_POINT_CONFIG_IDX][0] = 0x0,
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
#ifdef BUILD_HAS_MOD_SCMI_RESET_DOMAIN

#    define JUNO_RESET_DOMAIN_RESOURCE_CMDS 3
#    define JUNO_RESET_DOMAIN_RESOURCE_ELEMENTS \
        ((JUNO_RESET_DOMAIN_IDX_COUNT >> MOD_RES_PERMS_TYPE_SHIFT) + 1)
static mod_res_perms_t
    scmi_reset_domain_perms[] [JUNO_RESET_DOMAIN_RESOURCE_CMDS]
        [JUNO_RESET_DOMAIN_RESOURCE_ELEMENTS] = {
     /* SCMI_PROTOCOL_ID_RESET_DOMAIN */
     /* 0, MOD_SCMI_RESET_DOMAIN_ATTRIBUTES */
     /* 1, MOD_SCMI_RESET_REQUEST */
     /* 2, MOD_SCMI_RESET_NOTIFY */
     [AGENT_IDX(JUNO_SCMI_AGENT_IDX_OSPM)] = {
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_ATTRIBUTES_IDX][0] = 0x0,
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_REQUEST_IDX][0] = 0x0,
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_NOTIFY_IDX][0] = 0x0,
     },
     [AGENT_IDX(JUNO_SCMI_AGENT_IDX_PSCI)] = {
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_ATTRIBUTES_IDX][0] = 0x0,
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_REQUEST_IDX][0] = 0x0,
         [MOD_RES_PERMS_SCMI_RESET_DOMAIN_RESET_NOTIFY_IDX][0] = 0x0,
     },
 };

#endif

/*
 * Juno Platform Permissions
 */
static struct mod_res_agent_permission agent_permissions = {
    .agent_protocol_permissions = agent_protocol_permissions,
    .agent_msg_permissions = agent_msg_permissions,
    .scmi_clock_perms = &scmi_clock_perms[0][0][0],
    .scmi_pd_perms = &scmi_pd_perms[0][0][0],
    .scmi_perf_perms = &scmi_perf_perms[0][0][0],
    .scmi_sensor_perms = &scmi_sensor_perms[0][0][0],
#ifdef BUILD_HAS_MOD_SCMI_RESET_DOMAIN
    .scmi_reset_domain_perms = &scmi_reset_domain_perms[0][0][0],
#endif
};

/*
 * Juno Platform devices
 *
 * Note that a device must be terminated with
 *  {FWK_ID_NONE, MOD_RES_DOMAIN_DEVICE_INVALID}
 *
 */
static struct mod_res_domain_device devices_cpu[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SCMI_PERF, DVFS_ELEMENT_IDX_BIG),
        .type = MOD_RES_PERF_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_CLOCK,
            JUNO_CLOCK_IDX_BIGCLK),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_POWER_DOMAIN,
            POWER_DOMAIN_IDX_BIG_CPU0),
        .type = MOD_RES_POWER_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_POWER_DOMAIN,
            POWER_DOMAIN_IDX_BIG_CPU1),
        .type = MOD_RES_POWER_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_POWER_DOMAIN,
            POWER_DOMAIN_IDX_BIG_SSTOP),
        .type = MOD_RES_POWER_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_PERF,
            DVFS_ELEMENT_IDX_LITTLE),
        .type = MOD_RES_PERF_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_CLOCK,
            JUNO_CLOCK_IDX_LITTLECLK),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_POWER_DOMAIN,
            POWER_DOMAIN_IDX_LITTLE_CPU0),
        .type = MOD_RES_POWER_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_POWER_DOMAIN,
            POWER_DOMAIN_IDX_LITTLE_CPU1),
        .type = MOD_RES_POWER_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_POWER_DOMAIN,
            POWER_DOMAIN_IDX_LITTLE_CPU2),
        .type = MOD_RES_POWER_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_POWER_DOMAIN,
            POWER_DOMAIN_IDX_LITTLE_CPU3),
        .type = MOD_RES_POWER_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_POWER_DOMAIN,
            POWER_DOMAIN_IDX_LITTLE_SSTOP),
        .type = MOD_RES_POWER_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_gpu[] = {
    {
        .device_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SCMI_PERF, DVFS_ELEMENT_IDX_GPU),
        .type = MOD_RES_PERF_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_CLOCK,
            JUNO_CLOCK_IDX_GPUCLK),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_POWER_DOMAIN,
            POWER_DOMAIN_IDX_GPUTOP),
        .type = MOD_RES_POWER_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_domain_device devices_io[] = {
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_CLOCK,
            JUNO_CLOCK_IDX_HDLCD0),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_CLOCK,
            JUNO_CLOCK_IDX_HDLCD1),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_SCMI_CLOCK,
            JUNO_CLOCK_IDX_I2SCLK),
        .type = MOD_RES_CLOCK_DOMAIN_DEVICE,
    },
#ifdef BUILD_HAS_MOD_SCMI_RESET_DOMAIN
    {
        .device_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_RESET_DOMAIN,
            JUNO_RESET_DOMAIN_IDX_UART),
        .type = MOD_RES_RESET_DOMAIN_DEVICE,
    },
#endif
    {
        .device_id = FWK_ID_NONE_INIT,
        .type = MOD_RES_DOMAIN_DEVICE_INVALID,
    },
};

static struct mod_res_device juno_devices[] = {
    {
        .device_id = (uint16_t)JUNO_RES_PERMS_DEVICES_CPU,
        .domain_devices = devices_cpu,
    },
    {
        .device_id = (uint16_t)JUNO_RES_PERMS_DEVICES_GPU,
        .domain_devices = devices_gpu,
    },
    {
        .device_id = (uint16_t)JUNO_RES_PERMS_DEVICES_IO,
        .domain_devices = devices_io,
    },
    { 0 },
};

struct fwk_module_config config_resource_perms = {
    .data =
        &(struct mod_res_resource_perms_config){
            .agent_permissions = (uintptr_t)&agent_permissions,
            .agent_count = (uint32_t)JUNO_SCMI_AGENT_IDX_COUNT,
            .protocol_count = 7,
            .clock_counters.count = (uint32_t)JUNO_CLOCK_IDX_COUNT,
            .sensor_counters.count = (uint32_t)MOD_JUNO_R0_SENSOR_IDX_COUNT,
            .pd_counters.count = (uint32_t)POWER_DOMAIN_IDX_COUNT,
            .perf_counters.count = (uint32_t)DVFS_ELEMENT_IDX_COUNT,
            .perf_counters.cmd_count = JUNO_PERF_RESOURCE_CMDS,
            .perf_counters.resource_count = JUNO_PERF_RESOURCE_ELEMENTS,
            .device_count = (uint32_t)JUNO_RES_PERMS_DEVICES_COUNT,
#ifdef BUILD_HAS_MOD_SCMI_RESET_DOMAIN
            .reset_domain_count = JUNO_RESET_DOMAIN_IDX_COUNT,
#endif
            .domain_devices = (uintptr_t)&juno_devices,
        },
};

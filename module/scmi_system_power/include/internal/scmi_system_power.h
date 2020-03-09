/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_SCMI_SYSTEM_POWER_H
#define INTERNAL_SCMI_SYSTEM_POWER_H

#include <stdint.h>

#define SCMI_PROTOCOL_ID_SYS_POWER      UINT32_C(0x12)
#define SCMI_PROTOCOL_VERSION_SYS_POWER UINT32_C(0x10000)

/*
 * Identifiers of the SCMI System Power Management Protocol commands
 */
enum scmi_sys_power_command_id {
    SCMI_SYS_POWER_STATE_SET    = 0x003,
    SCMI_SYS_POWER_STATE_GET    = 0x004,
    SCMI_SYS_POWER_STATE_NOTIFY = 0x005,
};

/*
 * PROTOCOL_MESSAGE_ATTRIBUTES
 */
#define SYS_POWER_STATE_SET_ATTRIBUTES_WARM_RESET UINT32_C(0x80000000)
#define SYS_POWER_STATE_SET_ATTRIBUTES_SUSPEND    UINT32_C(0x40000000)

/*
 * SYSTEM_POWER_STATE_SET
 */

#define STATE_SET_FLAGS_MASK 0x1
#define STATE_SET_FLAGS_GRACEFUL_REQUEST 0x1

enum scmi_system_state {
    SCMI_SYSTEM_STATE_SHUTDOWN,
    SCMI_SYSTEM_STATE_COLD_RESET,
    SCMI_SYSTEM_STATE_WARM_RESET,
    SCMI_SYSTEM_STATE_POWER_UP,
    SCMI_SYSTEM_STATE_SUSPEND,
    SCMI_SYSTEM_STATE_MAX
};

struct __attribute((packed)) scmi_sys_power_state_set_a2p {
    uint32_t flags;
    uint32_t system_state;
};

struct __attribute((packed)) scmi_sys_power_state_set_p2a {
    int32_t status;
};

/*
 * SYSTEM_POWER_STATE_GET
 */

struct __attribute((packed)) scmi_sys_power_state_get_p2a {
    int32_t status;
    uint32_t system_state;
};

/*
 * SYSTEM_POWER_STATE_NOTIFY
 */

#define STATE_NOTIFY_FLAGS_MASK 0x1

struct __attribute((packed)) scmi_sys_power_state_notify_a2p {
    uint32_t flags;
};

struct __attribute((packed)) scmi_sys_power_state_notify_p2a {
    int32_t status;
};

/*
 * SYSTEM_POWER_STATE_NOTIFIER
 */

struct __attribute((packed)) scmi_sys_power_state_notifier_p2a {
    uint32_t agent_id;
    uint32_t system_state;
};

#endif /* INTERNAL_SCMI_SYSTEM_POWER_H */

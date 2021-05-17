/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_SCMI_SYSTEM_POWER_H
#define INTERNAL_SCMI_SYSTEM_POWER_H

#include <stdint.h>

#define SCMI_PROTOCOL_VERSION_SYS_POWER UINT32_C(0x10000)

enum scmi_sys_power_notification_id {
    SCMI_SYS_POWER_STATE_SET_NOTIFY = 0x000,
};

/*
 * PROTOCOL_MESSAGE_ATTRIBUTES
 */
#define SYS_POWER_STATE_SET_ATTRIBUTES_WARM_RESET UINT32_C(0x80000000)
#define SYS_POWER_STATE_SET_ATTRIBUTES_SUSPEND    UINT32_C(0x40000000)

/*
 * SYSTEM_POWER_STATE_SET
 */

#define STATE_SET_FLAGS_MASK             0x1U
#define STATE_SET_FLAGS_GRACEFUL_REQUEST 0x1U

enum scmi_system_state {
    SCMI_SYSTEM_STATE_SHUTDOWN,
    SCMI_SYSTEM_STATE_COLD_RESET,
    SCMI_SYSTEM_STATE_WARM_RESET,
    SCMI_SYSTEM_STATE_POWER_UP,
    SCMI_SYSTEM_STATE_SUSPEND,
    SCMI_SYSTEM_STATE_MAX
};

struct scmi_sys_power_state_set_a2p {
    uint32_t flags;
    uint32_t system_state;
};

struct scmi_sys_power_state_set_p2a {
    int32_t status;
};

/*
 * SYSTEM_POWER_STATE_GET
 */

struct scmi_sys_power_state_get_p2a {
    int32_t status;
    uint32_t system_state;
};

/*
 * SYSTEM_POWER_STATE_NOTIFY
 */

#define STATE_NOTIFY_FLAGS_MASK 0x1U

struct scmi_sys_power_state_notify_a2p {
    uint32_t flags;
};

struct scmi_sys_power_state_notify_p2a {
    int32_t status;
};

/*
 * SYSTEM_POWER_STATE_NOTIFIER
 */

struct scmi_sys_power_state_notifier {
    uint32_t agent_id;
    uint32_t flags;
    uint32_t system_state;
};

#endif /* INTERNAL_SCMI_SYSTEM_POWER_H */

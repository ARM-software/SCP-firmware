/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) support.
 */

#ifndef INTERNAL_SCMI_POWER_H
#define INTERNAL_SCMI_POWER_H

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupSCMI_PERF SCMI Power Domain Management Protocol
 * @{
 */

#define SCMI_PROTOCOL_ID_POWER_DOMAIN      UINT32_C(0x11)
#define SCMI_PROTOCOL_VERSION_POWER_DOMAIN UINT32_C(0x10000)

#define SCMI_PD_DEVICE_STATE_ID_OFF    0
#define SCMI_PD_DEVICE_STATE_ID_ON     0
#define SCMI_PD_DEVICE_STATE_ID_MASK   0xFFFFFFF
#define SCMI_PD_DEVICE_STATE_TYPE      (1 << 30)

/*
 * Identifier of the SCMI Power Domain Management Protocol commands
 */
enum scmi_pd_command_id {
    SCMI_PD_POWER_DOMAIN_ATTRIBUTES = 0x03,
    SCMI_PD_POWER_STATE_SET         = 0x04,
    SCMI_PD_POWER_STATE_GET         = 0x05,
    SCMI_PD_POWER_STATE_NOTIFY      = 0x06,
};

/*
 * PROTOCOL_ATTRIBUTES
 */

struct __attribute((packed)) scmi_pd_protocol_attributes_p2a {
    int32_t status;
    uint32_t attributes;
    uint32_t statistics_address_low;
    uint32_t statistics_address_high;
    uint32_t statistics_len;
};

/*
 * POWER_DOMAIN_ATTRIBUTES
 */

struct __attribute((packed)) scmi_pd_power_domain_attributes_a2p {
    uint32_t domain_id;
};

#define SCMI_PD_POWER_STATE_SET_ASYNC    (1 << 30)
#define SCMI_PD_POWER_STATE_SET_SYNC     (1 << 29)

struct __attribute((packed)) scmi_pd_power_domain_attributes_p2a {
    int32_t status;
    uint32_t attributes;
    uint8_t name[16];
};

/*
 * POWER_STATE_SET
 */

#define SCMI_PD_POWER_STATE_SET_ASYNC_FLAG_MASK    (1 << 0)

struct __attribute((packed)) scmi_pd_power_state_set_a2p {
    uint32_t flags;
    uint32_t domain_id;
    uint32_t power_state;
};

struct __attribute((packed)) scmi_pd_power_state_set_p2a {
    int32_t status;
};

/*
 * POWER_STATE_GET
 */

struct __attribute((packed)) scmi_pd_power_state_get_a2p {
    uint32_t domain_id;
};

struct __attribute((packed)) scmi_pd_power_state_get_p2a {
    int32_t status;
    uint32_t power_state;
};

/*
 * POWER_STATE_NOTIFY
 */

struct __attribute((packed)) scmi_pd_power_state_notify_a2p {
    uint32_t domain_id;
    uint32_t notify_enable;
};

struct __attribute((packed)) scmi_pd_power_state_notify_p2a {
    int32_t status;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* INTERNAL_SCMI_POWER_DOMAIN_H */

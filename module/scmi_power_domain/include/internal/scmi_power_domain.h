/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
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
 * \{
 */

/*!
 * \defgroup GroupSCMIPowerDomain SCMI Power Domain Management Protocol
 * \{
 */

#define SCMI_PROTOCOL_VERSION_POWER_DOMAIN UINT32_C(0x20000)

#define SCMI_PD_DEVICE_STATE_ID_OFF  0U
#define SCMI_PD_DEVICE_STATE_ID_ON   0U
#define SCMI_PD_DEVICE_STATE_ID      0U
#define SCMI_PD_DEVICE_STATE_ID_MASK 0xFFFFFFFU
#define SCMI_PD_DEVICE_STATE_TYPE    (1U << 30)

/*!
 * \brief SCMI Power Domain Protocol Notifications message ids.
 */
enum scmi_pd_notification_id {
    SCMI_POWER_STATE_CHANGED = 0x000,
    SCMI_POWER_STATE_CHANGE_REQUESTED = 0x001,
};

/*
 * PROTOCOL_ATTRIBUTES
 */

struct scmi_pd_protocol_attributes_p2a {
    int32_t status;
    uint32_t attributes;
    uint32_t statistics_address_low;
    uint32_t statistics_address_high;
    uint32_t statistics_len;
};

/*
 * POWER_DOMAIN_ATTRIBUTES
 */

struct scmi_pd_power_domain_attributes_a2p {
    uint32_t domain_id;
};

#define SCMI_PD_POWER_STATE_CHANGE_NOTIFICATIONS (1UL << 31)
#define SCMI_PD_POWER_STATE_SET_ASYNC            (1U << 30)
#define SCMI_PD_POWER_STATE_SET_SYNC             (1U << 29)

struct scmi_pd_power_domain_attributes_p2a {
    int32_t status;
    uint32_t attributes;
    uint8_t name[16];
};

/*
 * POWER_STATE_SET
 */

#define SCMI_PD_POWER_STATE_SET_ASYNC_FLAG_MASK  (1U << 0)
#define SCMI_PD_POWER_STATE_SET_FLAGS_MASK       (1U << 0)
#define SCMI_PD_POWER_STATE_SET_POWER_STATE_MASK UINT32_C(0x4FFFFFFF)

struct scmi_pd_power_state_set_a2p {
    uint32_t flags;
    uint32_t domain_id;
    uint32_t power_state;
};

struct scmi_pd_power_state_set_p2a {
    int32_t status;
};

/*
 * POWER_STATE_GET
 */

struct scmi_pd_power_state_get_a2p {
    uint32_t domain_id;
};

struct scmi_pd_power_state_get_p2a {
    int32_t status;
    uint32_t power_state;
};

/*
 * POWER_STATE_NOTIFY
 */
#define SCMI_PD_NOTIFY_ENABLE_MASK UINT32_C(0x1)

struct scmi_pd_power_state_notify_a2p {
    uint32_t domain_id;
    uint32_t notify_enable;
};

struct scmi_pd_power_state_notify_p2a {
    int32_t status;
};

struct __attribute((packed)) scmi_pd_power_state_notification_message_p2a {
    uint32_t agent_id;
    uint32_t domain_id;
    uint32_t power_state;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* INTERNAL_SCMI_POWER_DOMAIN_H */

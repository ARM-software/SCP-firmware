/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) support for Reset Domain
 *      Management Protocol.
 */

#ifndef INTERNAL_SCMI_RESET_DOMAIN_H
#define INTERNAL_SCMI_RESET_DOMAIN_H

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupSCMI_RESET SCMI Reset Domain Management Protocol.
 * \{
 */

#define SCMI_PROTOCOL_VERSION_RESET_DOMAIN UINT32_C(0x10000)

#define SCMI_RESET_STATE_ARCH               (0 << 31)
#define SCMI_RESET_STATE_IMPL               (1 << 31)

/*
 * PROTOCOL_ATTRIBUTES
 */

struct scmi_reset_domain_protocol_attributes_p2a {
    int32_t status;
    uint32_t attributes;
};

/* Value for scmi_reset_domain_attributes_p2a:flags */
#define SCMI_RESET_DOMAIN_ATTR_ASYNC    (1UL << 31)
#define SCMI_RESET_DOMAIN_ATTR_NOTIF    (1UL << 30)

/* Value for scmi_reset_domain_attributes_p2a:latency */
#define SCMI_RESET_DOMAIN_ATTR_LATENCY_UNSUPPORTED  0xFFFFFFFF

/* Macro for scmi_reset_domain_attributes_p2a:name */
#define SCMI_RESET_DOMAIN_ATTR_NAME_SZ  16

struct scmi_reset_domain_attributes_a2p {
    uint32_t domain_id;
};

struct scmi_reset_domain_attributes_p2a {
    int32_t status;
    uint32_t flags;
    uint32_t latency;
    uint8_t name[SCMI_RESET_DOMAIN_ATTR_NAME_SZ];
};

/*
 * RESET
 */

/* Values for scmi_reset_domain_request_p2a:flags */
#define SCMI_RESET_DOMAIN_ASYNC      (1 << 2)
#define SCMI_RESET_DOMAIN_EXPLICIT   (1 << 1)
#define SCMI_RESET_DOMAIN_AUTO       (1 << 0)
#define SCMI_RESET_DOMAIN_FLAGS_MASK \
    (SCMI_RESET_DOMAIN_ASYNC | SCMI_RESET_DOMAIN_EXPLICIT | \
     SCMI_RESET_DOMAIN_AUTO)

#define SCMI_RESET_DOMAIN_RESET_STATE_TYPE_MASK (1UL << 31)
#define SCMI_RESET_DOMAIN_RESET_STATE_ID_MASK UINT32_C(0x7FFFFFFF)

struct scmi_reset_domain_request_a2p {
    uint32_t domain_id;
    uint32_t flags;
    uint32_t reset_state;
};

struct scmi_reset_domain_request_p2a {
    int32_t status;
};

/*
 * RESET_NOTIFY
 */

/* Values for scmi_reset_notify_p2a:flags */
#define SCMI_RESET_DOMAIN_DO_NOTIFY  (1 << 0)

struct scmi_reset_domain_notify_a2p {
    uint32_t domain_id;
    uint32_t notify_enable;
};

struct scmi_reset_domain_notify_p2a {
    int32_t status;
};

/*
 * RESET_COMPLETE
 */

struct scmi_reset_domain_complete_p2a {
    int32_t status;
    uint32_t domain_id;
};

/*
 * RESET_ISSUED
 */

struct scmi_reset_domain_issued_p2a {
    uint32_t agent_id;
    uint32_t domain_id;
    uint32_t reset_state;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* INTERNAL_SCMI_RESET_DOMAIN_H */

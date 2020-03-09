/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_SMT_H
#define INTERNAL_SMT_H

#include <stdint.h>

struct __attribute((packed)) mod_smt_memory {
    uint32_t reserved0;
    uint32_t status;
    uint64_t reserved1;
    uint32_t flags;
    uint32_t length; /* message_header + payload */
    uint32_t message_header;
    uint32_t payload[];
};

#define MOD_SMT_MAX_CHANNELS 8

#define MOD_SMT_MAILBOX_STATUS_FREE_POS 0
#define MOD_SMT_MAILBOX_STATUS_FREE_MASK \
    (UINT32_C(0x1) << MOD_SMT_MAILBOX_STATUS_FREE_POS)

#define MOD_SMT_MAILBOX_STATUS_ERROR_POS 1
#define MOD_SMT_MAILBOX_STATUS_ERROR_MASK \
    (UINT32_C(0x1) << MOD_SMT_MAILBOX_STATUS_ERROR_POS)

#define MOD_SMT_MAILBOX_FLAGS_IENABLED_POS 0
#define MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK \
    (UINT32_C(0x1) << MOD_SMT_MAILBOX_FLAGS_IENABLED_POS)

#define MOD_SMT_MIN_PAYLOAD_SIZE \
    sizeof(((struct mod_smt_memory *)NULL)->payload[0])

#define MOD_SMT_MIN_MAILBOX_SIZE \
    (sizeof(struct mod_smt_memory) + MOD_SMT_MIN_PAYLOAD_SIZE)

#define SCMI_MESSAGE_HEADER_MESSAGE_ID_POS  0
#define SCMI_MESSAGE_HEADER_PROTOCOL_ID_POS 10
#define SCMI_MESSAGE_HEADER_TOKEN_POS       18

#define SCMI_MESSAGE_HEADER_MESSAGE_ID_MASK \
    (UINT32_C(0x3FF) << SCMI_MESSAGE_HEADER_MESSAGE_ID_POS)
#define SCMI_MESSAGE_HEADER_PROTOCOL_ID_MASK \
    (UINT32_C(0xFF)  << SCMI_MESSAGE_HEADER_PROTOCOL_ID_POS)
#define SCMI_MESSAGE_HEADER_TOKEN_MASK  \
    (UINT32_C(0x3FF) << SCMI_MESSAGE_HEADER_TOKEN_POS)

#define SCMI_MESSAGE_HEADER(MESSAGE_ID, PROTOCOL_ID, TOKEN) \
    ((((MESSAGE_ID) << SCMI_MESSAGE_HEADER_MESSAGE_ID_POS) & \
        SCMI_MESSAGE_HEADER_MESSAGE_ID_MASK) | \
    (((PROTOCOL_ID) << SCMI_MESSAGE_HEADER_PROTOCOL_ID_POS) & \
        SCMI_MESSAGE_HEADER_PROTOCOL_ID_MASK) | \
    (((TOKEN) << SCMI_MESSAGE_HEADER_TOKEN_POS) & \
        SCMI_MESSAGE_HEADER_TOKEN_POS))

#endif /* INTERNAL_SMT_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_OPTEE_SMT_H
#define INTERNAL_OPTEE_SMT_H

#include <stdint.h>

struct __attribute((packed)) mod_optee_smt_memory {
    uint32_t reserved0;
    uint32_t status;
    uint64_t reserved1;
    uint32_t flags;
    uint32_t length; /* message_header + payload */
    uint32_t message_header;
    uint32_t payload[];
};

#define MOD_OPTEE_SMT_MAX_CHANNELS 8

#define MOD_OPTEE_SMT_MAILBOX_STATUS_FREE_POS 0
#define MOD_OPTEE_SMT_MAILBOX_STATUS_FREE_MASK \
    (UINT32_C(0x1) << MOD_OPTEE_SMT_MAILBOX_STATUS_FREE_POS)

#define MOD_OPTEE_SMT_MAILBOX_STATUS_ERROR_POS 1
#define MOD_OPTEE_SMT_MAILBOX_STATUS_ERROR_MASK \
    (UINT32_C(0x1) << MOD_OPTEE_SMT_MAILBOX_STATUS_ERROR_POS)

#define MOD_OPTEE_SMT_MAILBOX_FLAGS_IENABLED_POS 0
#define MOD_OPTEE_SMT_MAILBOX_FLAGS_IENABLED_MASK \
    (UINT32_C(0x1) << MOD_OPTEE_SMT_MAILBOX_FLAGS_IENABLED_POS)

#define MOD_OPTEE_SMT_MIN_PAYLOAD_SIZE \
    sizeof(((struct mod_optee_smt_memory *)NULL)->payload[0])

#define MOD_OPTEE_SMT_MIN_MAILBOX_SIZE \
    (sizeof(struct mod_optee_smt_memory) + MOD_OPTEE_SMT_MIN_PAYLOAD_SIZE)

#endif /* INTERNAL_OPTEE_SMT_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Core Configuration Protocol Support
 */

#ifndef INTERNAL_SCMI_APCORE_H
#define INTERNAL_SCMI_APCORE_H

#include <stdint.h>

#define SCMI_PROTOCOL_ID_APCORE      UINT32_C(0x90)
#define SCMI_PROTOCOL_VERSION_APCORE UINT32_C(0x10000)

/*
 * Identifiers of the SCMI Core Configuration Protocol commands
 */
enum scmi_apcore_command_id {
    SCMI_APCORE_RESET_ADDRESS_SET = 0x3,
    SCMI_APCORE_RESET_ADDRESS_GET = 0x4,
};

/*
 * Protocol Attributes
 */

#define SCMI_APCORE_PROTOCOL_ATTRIBUTES_64BIT_POS 0

#define SCMI_APCORE_PROTOCOL_ATTRIBUTES_64BIT_MASK \
    (UINT32_C(0x1) << SCMI_APCORE_PROTOCOL_ATTRIBUTES_64BIT_POS)

/*
 * Reset Address Set
 */

#define SCMI_APCORE_RESET_ADDRESS_SET_LOCK_POS 0

#define SCMI_APCORE_RESET_ADDRESS_SET_LOCK_MASK \
    (UINT32_C(0x1) << SCMI_APCORE_RESET_ADDRESS_SET_LOCK_POS)

struct __attribute((packed)) scmi_apcore_reset_address_set_a2p {
    uint32_t reset_address_low;
    uint32_t reset_address_high;
    uint32_t attributes;
};

struct __attribute((packed)) scmi_apcore_reset_address_set_p2a {
    int32_t status;
};

/*
 * Reset Address Get
 */

#define SCMI_APCORE_RESET_ADDRESS_GET_LOCK_POS 0

#define SCMI_APCORE_RESET_ADDRESS_GET_LOCK_MASK \
    (UINT32_C(0x1) << SCMI_APCORE_RESET_ADDRESS_GET_LOCK_POS)

struct __attribute((packed)) scmi_apcore_reset_address_get_p2a {
    int32_t status;
    uint32_t reset_address_low;
    uint32_t reset_address_high;
    uint32_t attributes;
};

#endif /* INTERNAL_SCMI_APCORE_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Core Configuration Protocol Support
 */

#ifndef INTERNAL_SCMI_APCORE_H
#define INTERNAL_SCMI_APCORE_H

#include <stdint.h>

/*
 * Protocol Attributes
 */

#define MOD_SCMI_APCORE_PROTOCOL_ATTRIBUTES_64BIT_POS 0

#define MOD_SCMI_APCORE_PROTOCOL_ATTRIBUTES_64BIT_MASK \
    (UINT32_C(0x1) << MOD_SCMI_APCORE_PROTOCOL_ATTRIBUTES_64BIT_POS)

/*
 * Reset Address Set
 */

#define MOD_SCMI_APCORE_RESET_ADDRESS_SET_LOCK_POS 0

#define MOD_SCMI_APCORE_RESET_ADDRESS_SET_LOCK_MASK \
    (UINT32_C(0x1) << MOD_SCMI_APCORE_RESET_ADDRESS_SET_LOCK_POS)

struct scmi_apcore_reset_address_set_a2p {
    uint32_t reset_address_low;
    uint32_t reset_address_high;
    uint32_t attributes;
};

struct scmi_apcore_reset_address_set_p2a {
    int32_t status;
};

/*
 * Reset Address Get
 */

#define MOD_SCMI_APCORE_RESET_ADDRESS_GET_LOCK_POS 0

#define MOD_SCMI_APCORE_RESET_ADDRESS_GET_LOCK_MASK \
    (UINT32_C(0x1) << MOD_SCMI_APCORE_RESET_ADDRESS_GET_LOCK_POS)

struct scmi_apcore_reset_address_get_p2a {
    int32_t status;
    uint32_t reset_address_low;
    uint32_t reset_address_high;
    uint32_t attributes;
};

#endif /* INTERNAL_SCMI_APCORE_H */

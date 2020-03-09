/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RDN1E1_SDS_H
#define RDN1E1_SDS_H

#include <mod_sds.h>

#include <stdint.h>

/*
 * Structure identifiers.
 */
enum rdn1e1_sds_struct_id {
    RDN1E1_SDS_CPU_INFO = 1 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    RDN1E1_SDS_FIRMWARE_VERSION = 2 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    RDN1E1_SDS_PLATFORM_ID = 3 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    RDN1E1_SDS_RESET_SYNDROME = 4 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    RDN1E1_SDS_FEATURE_AVAILABILITY =
        5 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    RDN1E1_SDS_CPU_BOOTCTR = 6 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    RDN1E1_SDS_CPU_FLAGS = 7 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
};

/*
 * Structure sizes.
 */
#define RDN1E1_SDS_CPU_INFO_SIZE 4
#define RDN1E1_SDS_FIRMWARE_VERSION_SIZE 4
#define RDN1E1_SDS_PLATFORM_ID_SIZE 8
#define RDN1E1_SDS_RESET_SYNDROME_SIZE 4
#define RDN1E1_SDS_FEATURE_AVAILABILITY_SIZE 4
#define RDN1E1_SDS_CPU_BOOTCTR_SIZE 256
#define RDN1E1_SDS_CPU_FLAGS_SIZE 256

/*
 * Field masks and offsets for the RDN1E1_SDS_AP_CPU_INFO structure.
 */
#define RDN1E1_SDS_CPU_INFO_PRIMARY_MASK 0xFFFFFFFF
#define RDN1E1_SDS_CPU_INFO_PRIMARY_POS 0

/*
 * Platform information
 */
struct rdn1e1_sds_platid {
    /* Subsystem part number */
    uint32_t platform_identifier;
    /* Platform type information */
    uint32_t platform_type_identifier;
};

/*
 * Field masks and offsets for the RDN1E1_SDS_FEATURE_AVAILABILITY structure.
 */
#define RDN1E1_SDS_FEATURE_FIRMWARE_MASK 0x1
#define RDN1E1_SDS_FEATURE_DMC_MASK 0x2
#define RDN1E1_SDS_FEATURE_MESSAGING_MASK 0x4

#define RDN1E1_SDS_FEATURE_FIRMWARE_POS 0
#define RDN1E1_SDS_FEATURE_DMC_POS 1
#define RDN1E1_SDS_FEATURE_MESSAGING_POS 2

#endif /* RDN1E1_SDS_H */

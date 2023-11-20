/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_SDS_H
#define PLATFORM_SDS_H

#include <mod_sds.h>

/*
 * Structure identifiers.
 */
enum platform_sds_struct_id {
    PLATFORM_SDS_CPU_INFO = 1 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    PLATFORM_SDS_FIRMWARE_VERSION = 2 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    PLATFORM_SDS_PLATFORM_ID = 3 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    PLATFORM_SDS_RESET_SYNDROME = 4 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    PLATFORM_SDS_FEATURE_AVAILABILITY = 5 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    PLATFORM_SDS_CPU_BOOTCTR = 6 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    PLATFORM_SDS_CPU_FLAGS = 7 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
};

enum platform_sds_region_idx {
    PLATFORM_SDS_REGION_SECURE,
#ifdef BUILD_MODE_DEBUG
    PLATFORM_SDS_REGION_NONSECURE,
#endif
    PLATFORM_SDS_REGION_COUNT,
};

/*
 * Structure sizes.
 */
#define PLATFORM_SDS_CPU_INFO_SIZE             4
#define PLATFORM_SDS_FIRMWARE_VERSION_SIZE     4
#define PLATFORM_SDS_PLATFORM_ID_SIZE          8
#define PLATFORM_SDS_RESET_SYNDROME_SIZE       4
#define PLATFORM_SDS_FEATURE_AVAILABILITY_SIZE 4
#define PLATFORM_SDS_CPU_BOOTCTR_SIZE          256
#define PLATFORM_SDS_CPU_FLAGS_SIZE            256

/*
 * Field masks and offsets for the PLATFORM_SDS_AP_CPU_INFO structure.
 */
#define PLATFORM_SDS_CPU_INFO_PRIMARY_MASK 0xFFFFFFFF
#define PLATFORM_SDS_CPU_INFO_PRIMARY_POS  0

/*
 * Platform information
 */
struct platform_sds_platid {
    /* Subsystem part number */
    uint32_t platform_identifier;
    /* Platform type information */
    uint32_t platform_type_identifier;
};

/*
 * Field masks and offsets for the PLATFORM_SDS_FEATURE_AVAILABILITY
 * structure.
 */
#define PLATFORM_SDS_FEATURE_FIRMWARE_MASK  0x1
#define PLATFORM_SDS_FEATURE_DMC_MASK       0x2
#define PLATFORM_SDS_FEATURE_MESSAGING_MASK 0x4

#define PLATFORM_SDS_FEATURE_FIRMWARE_POS  0
#define PLATFORM_SDS_FEATURE_DMC_POS       1
#define PLATFORM_SDS_FEATURE_MESSAGING_POS 2

#endif /* PLATFORM_SDS_H */

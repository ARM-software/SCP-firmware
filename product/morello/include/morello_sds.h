/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MORELLO_SDS_H
#define MORELLO_SDS_H

#include <mod_sds.h>

#include <stdint.h>

/*
 * Structure identifiers.
 */
enum morello_sds_struct_id {
    MORELLO_SDS_CPU_INFO = 1 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    MORELLO_SDS_FIRMWARE_VERSION = 2 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    MORELLO_SDS_PLATFORM_ID = 3 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    MORELLO_SDS_RESET_SYNDROME = 4 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    MORELLO_SDS_FEATURE_AVAILABILITY = 5 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    MORELLO_SDS_CPU_BOOTCTR = 6 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    MORELLO_SDS_CPU_FLAGS = 7 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    MORELLO_SDS_PLATFORM_INFO = 8 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
};

enum morello_sds_region_idx {
    MORELLO_SDS_REGION_SECURE,
#ifdef BUILD_MODE_DEBUG
    MORELLO_SDS_REGION_NONSECURE,
#endif
    MORELLO_SDS_REGION_COUNT,
};

/*
 * Structure sizes.
 */
#define MORELLO_SDS_CPU_INFO_SIZE 4
#if defined(PLAT_FVP)
#    define MORELLO_SDS_FIRMWARE_VERSION_SIZE 8
#else
#    define MORELLO_SDS_FIRMWARE_VERSION_SIZE 16
#endif
#define MORELLO_SDS_PLATFORM_ID_SIZE 8
#define MORELLO_SDS_RESET_SYNDROME_SIZE 4
#define MORELLO_SDS_FEATURE_AVAILABILITY_SIZE 4
#define MORELLO_SDS_CPU_BOOTCTR_SIZE 256
#define MORELLO_SDS_CPU_FLAGS_SIZE 256
#if defined(PLAT_FVP)
#    define MORELLO_SDS_PLATFORM_INFO_SIZE 8
#else
#    define MORELLO_SDS_PLATFORM_INFO_SIZE 26
#endif

/*
 * Field offsets for morello silicon revision.
 */
#define MORELLO_SILICON_REVISION_R_POS 16
#define MORELLO_SILICON_REVISION_P_POS 0

/*
 * Field offsets for PCC and MCC firmware versions
 */
#define MORELLO_PCC_FIRMWARE_VERSION_UPPER_POS 16
#define MORELLO_PCC_FIRMWARE_VERSION_MID_POS   8
#define MORELLO_PCC_FIRMWARE_VERSION_LOWER_POS 0

#define MORELLO_MCC_FIRMWARE_VERSION_UPPER_POS 16
#define MORELLO_MCC_FIRMWARE_VERSION_MID_POS   8
#define MORELLO_MCC_FIRMWARE_VERSION_LOWER_POS 0

/*
 * Field masks and offsets for the MORELLO_SDS_AP_CPU_INFO structure.
 */
#define MORELLO_SDS_CPU_INFO_PRIMARY_MASK 0xFFFFFFFF
#define MORELLO_SDS_CPU_INFO_PRIMARY_POS 0

/*
 * Field sizes for SCP firmware version information.
 */

#define MORELLO_SDS_FIRMWARE_COMMIT_ID_LEN 8

/*
 * Platform information
 */
struct morello_sds_platid {
    /* Subsystem part number */
    uint32_t platform_identifier;
    /* Platform type information */
    uint32_t platform_type_identifier;
};

/*
 * Field masks and offsets for the MORELLO_SDS_FEATURE_AVAILABILITY structure.
 */
#define MORELLO_SDS_FEATURE_FIRMWARE_MASK 0x1
#define MORELLO_SDS_FEATURE_DMC_MASK 0x2
#define MORELLO_SDS_FEATURE_MESSAGING_MASK 0x4

#define MORELLO_SDS_FEATURE_FIRMWARE_POS 0
#define MORELLO_SDS_FEATURE_DMC_POS 1
#define MORELLO_SDS_FEATURE_MESSAGING_POS 2

/*
 * Element identifiers for SDS structures
 */
#define SDS_ELEMENT_IDX_FIRMWARE_VERSION     1
#define SDS_ELEMENT_IDX_FEATURE_AVAILABILITY 3
#define SDS_ELEMENT_IDX_PLATFORM_INFO 4
#endif /* MORELLO_SDS_H */

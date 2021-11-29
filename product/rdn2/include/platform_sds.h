/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PLATFORM_SDS_H
#define PLATFORM_SDS_H

#include <platform_core.h>

#include <mod_sds.h>

#include <stdint.h>

#define SDS_STRUCT_ID(idx) (idx | 1 << MOD_SDS_ID_VERSION_MAJOR_POS)

/*
 * Structure identifiers.
 */
enum platform_sds_struct_id {
    PLATFORM_SDS_CPU_INFO = SDS_STRUCT_ID(1),
    PLATFORM_SDS_ROM_VERSION = SDS_STRUCT_ID(2),
    PLATFORM_SDS_RAM_VERSION = SDS_STRUCT_ID(3),
    PLATFORM_SDS_PLATFORM_ID = SDS_STRUCT_ID(4),
    PLATFORM_SDS_RESET_SYNDROME = SDS_STRUCT_ID(5),
    PLATFORM_SDS_FEATURE_AVAILABILITY = SDS_STRUCT_ID(6),
    PLATFORM_SDS_ISOLATED_CPU_MPID_LIST = SDS_STRUCT_ID(128),
};

enum platform_sds_region_idx {
    PLATFORM_SDS_REGION_SECURE,
    PLATFORM_SDS_REGION_COUNT,
};

enum platform_sds_ram_idx {
    PLATFORM_SDS_CPU_INFO_IDX,
    PLATFORM_SDS_ROM_VERSION_IDX,
    PLATFORM_SDS_RAM_VERSION_IDX,
    PLATFORM_SDS_RESET_SYNDROME_IDX,
    PLATFORM_SDS_FEATURE_AVAILABILITY_IDX,
    PLATFORM_SDS_ISOLATED_CPU_MPID_LIST_IDX,
    PLATFORM_SDS_IDX_COUNT
};

/*
 * Structure sizes.
 */
#define PLATFORM_SDS_CPU_INFO_SIZE             4
#define PLATFORM_SDS_ROM_VERSION_SIZE          4
#define PLATFORM_SDS_RAM_VERSION_SIZE          4
#define PLATFORM_SDS_PLATFORM_ID_SIZE          8
#define PLATFORM_SDS_RESET_SYNDROME_SIZE       4
#define PLATFORM_SDS_FEATURE_AVAILABILITY_SIZE 4

/*
 * Max size of structure listing the MPID of the isolated CPUs.
 * size = (Number of CPUs * sizeof MPID register) + sizeof CPU count variable
 */
#define PLATFORM_SDS_ISOLATED_CPU_MPID_LIST_SIZE \
    ((NUMBER_OF_CLUSTERS * sizeof(uint64_t)) + sizeof(uint64_t))

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
 * Field masks and offsets for the PLATFORM_SDS_FEATURE_AVAILABILITY structure.
 */
#define PLATFORM_SDS_FEATURE_FIRMWARE_MASK  0x1
#define PLATFORM_SDS_FEATURE_DMC_MASK       0x2
#define PLATFORM_SDS_FEATURE_MESSAGING_MASK 0x4

#define PLATFORM_SDS_FEATURE_FIRMWARE_POS  0
#define PLATFORM_SDS_FEATURE_DMC_POS       1
#define PLATFORM_SDS_FEATURE_MESSAGING_POS 2

#endif /* PLATFORM_SDS_H */

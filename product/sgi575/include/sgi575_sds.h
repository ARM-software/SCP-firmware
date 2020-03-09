/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGI575_SDS_H
#define SGI575_SDS_H

#include <mod_sds.h>

#include <stdint.h>

/*
 * Structure identifiers.
 */
enum sgi575_sds_struct_id {
    SGI575_SDS_CPU_INFO = 1 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    SGI575_SDS_FIRMWARE_VERSION = 2 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    SGI575_SDS_PLATFORM_ID = 3 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    SGI575_SDS_RESET_SYNDROME = 4 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    SGI575_SDS_FEATURE_AVAILABILITY =
        5 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    SGI575_SDS_CPU_BOOTCTR = 6 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    SGI575_SDS_CPU_FLAGS = 7 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
};

/*
 * Structure sizes.
 */
#define SGI575_SDS_CPU_INFO_SIZE 4
#define SGI575_SDS_FIRMWARE_VERSION_SIZE 4
#define SGI575_SDS_PLATFORM_ID_SIZE 8
#define SGI575_SDS_RESET_SYNDROME_SIZE 4
#define SGI575_SDS_FEATURE_AVAILABILITY_SIZE 4
#define SGI575_SDS_CPU_BOOTCTR_SIZE 256
#define SGI575_SDS_CPU_FLAGS_SIZE 256

/*
 * Field masks and offsets for the SGI575_SDS_AP_CPU_INFO structure.
 */
#define SGI575_SDS_CPU_INFO_PRIMARY_MASK 0xFFFFFFFF
#define SGI575_SDS_CPU_INFO_PRIMARY_POS 0

/*
 * Platform information:
 * Structure, field masks and offsets for the SGI575_SDS_PLATFORM_ID
 * structure.
 */
struct sgi575_sds_platid {
    /* Subsystem part number */
    uint32_t platform_identifier;
    /* Platform type information */
    uint32_t platform_type_identifier;
};

#define SGI575_SDS_PLATID_PARTNO_MASK 0xFFF
#define SGI575_SDS_PLATID_DESIGNER_MASK 0xFF000
#define SGI575_SDS_PLATID_REV_MINOR_MASK 0xF00000
#define SGI575_SDS_PLATID_REV_MAJOR_MASK 0xF000000
#define SGI575_SDS_PLATID_CONFIG_MASK 0xF0000000
#define SGI575_SDS_PLATID_TYPE_MASK 0xF

#define SGI575_SDS_PLATID_PARTNO_POS 0
#define SGI575_SDS_PLATID_DESIGNER_POS 12
#define SGI575_SDS_PLATID_REV_MINOR_POS 20
#define SGI575_SDS_PLATID_REV_MAJOR_POS 24
#define SGI575_SDS_PLATID_CONFIG_POS 28

#define SGI575_SDS_PLATID_TYPE_POS 0

/*
 * Field masks and offsets for the SGI575_SDS_FEATURE_AVAILABILITY structure.
 */
#define SGI575_SDS_FEATURE_FIRMWARE_MASK 0x1
#define SGI575_SDS_FEATURE_DMC_MASK 0x2
#define SGI575_SDS_FEATURE_MESSAGING_MASK 0x4

#define SGI575_SDS_FEATURE_FIRMWARE_POS 0
#define SGI575_SDS_FEATURE_DMC_POS 1
#define SGI575_SDS_FEATURE_MESSAGING_POS 2

#endif /* SGI575_SDS_H */

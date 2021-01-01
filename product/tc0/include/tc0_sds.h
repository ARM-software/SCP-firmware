/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TC0_SDS_H
#define TC0_SDS_H

#include <mod_sds.h>

/*
 * Structure identifiers.
 */
enum tc0_sds_struct_id {
    TC0_SDS_CPU_INFO = 1 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    TC0_SDS_FEATURE_AVAILABILITY = 6 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    TC0_SDS_BOOTLOADER = 9 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
};

enum tc0_sds_region_idx { TC0_SDS_REGION_SECURE, TC0_SDS_REGION_COUNT };

/*
 * Structure sizes.
 */
#define TC0_SDS_CPU_INFO_SIZE 4
#define TC0_SDS_FEATURE_AVAILABILITY_SIZE 4
#define TC0_SDS_BOOTLOADER_SIZE 12

/*
 * Field masks and offsets for TC0_SDS_AP_CPU_INFO structure.
 */
#define TC0_SDS_CPU_INFO_PRIMARY_MASK 0xFFFFFFFF
#define TC0_SDS_CPU_INFO_PRIMARY_POS 0

/*
 * Field masks and offsets for TC0_SDS_FEATURE_AVAILABILITY structure.
 */
#define TC0_SDS_FEATURE_FIRMWARE_MASK 0x1
#define TC0_SDS_FEATURE_DMC_MASK 0x2
#define TC0_SDS_FEATURE_MESSAGING_MASK 0x4

#define TC0_SDS_FEATURE_FIRMWARE_POS 0
#define TC0_SDS_FEATURE_DMC_POS 1
#define TC0_SDS_FEATURE_MESSAGING_POS 2

/*
 * Field masks and offsets for the TC0_SDS_BOOTLOADER structure.
 */
#define TC0_SDS_BOOTLOADER_VALID_MASK 0x1
#define TC0_SDS_BOOTLOADER_OFFSET_MASK 0xFFFFFFFF
#define TC0_SDS_BOOTLOADER_SIZE_MASK 0xFFFFFFFF

#define TC0_SDS_BOOTLOADER_VALID_POS 0
#define TC0_SDS_BOOTLOADER_OFFSET_POS 0
#define TC0_SDS_BOOTLOADER_SIZE_POS 0

#endif /* TC0_SDS_H */

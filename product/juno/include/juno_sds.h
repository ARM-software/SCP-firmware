/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno Shared Data Structures (SDS) definitions.
 */

#ifndef JUNO_SDS_H
#define JUNO_SDS_H

#include <mod_sds.h>

/*
 * Structure identifiers.
 */
enum juno_sds_struct_id {
    JUNO_SDS_CPU_INFO =             1 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_ROM_VERSION =          2 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_RAM_VERSION =          3 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_PLATFORM_ID =          4 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_RESET_SYNDROME =       5 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_FEATURE_AVAILABILITY = 6 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_CPU_BOOTCTR =          7 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_CPU_FLAGS =            8 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
    JUNO_SDS_BOOTLOADER =           9 | (1 << MOD_SDS_ID_VERSION_MAJOR_POS),
};

/*
 * Structure sizes.
 */
#define JUNO_SDS_CPU_INFO_SIZE             4
#define JUNO_SDS_ROM_VERSION_SIZE          4
#define JUNO_SDS_RAM_VERSION_SIZE          4
#define JUNO_SDS_PLATFORM_ID_SIZE          8
#define JUNO_SDS_RESET_SYNDROME_SIZE       4
#define JUNO_SDS_FEATURE_AVAILABILITY_SIZE 4
#define JUNO_SDS_CPU_BOOTCTR_SIZE          8
#define JUNO_SDS_CPU_FLAGS_SIZE            8
#define JUNO_SDS_BOOTLOADER_SIZE           12

/*
 * Field masks and offsets for the JUNO_SDS_AP_CPU_INFO structure.
 */
#define JUNO_SDS_CPU_INFO_PRIMARY_MASK 0xFFFFFFFF
#define JUNO_SDS_CPU_INFO_PRIMARY_POS 0

/*
 * Structure, field masks and offsets for the JUNO_SDS_PLATFORM_ID structure.
 */
struct juno_sds_platid {
    uint32_t platform_identifier;
    uint32_t platform_type_identifier;
};

#define JUNO_SDS_PLATID_PARTNO_MASK 0xFFF
#define JUNO_SDS_PLATID_DESIGNER_MASK 0xFF000
#define JUNO_SDS_PLATID_REV_MINOR_MASK 0xF00000
#define JUNO_SDS_PLATID_REV_MAJOR_MASK 0xF000000
#define JUNO_SDS_PLATID_CONFIG_MASK 0xF0000000
#define JUNO_SDS_PLATID_TYPE_MASK 0xF

#define JUNO_SDS_PLATID_PARTNO_POS 0
#define JUNO_SDS_PLATID_DESIGNER_POS 12
#define JUNO_SDS_PLATID_REV_MINOR_POS 20
#define JUNO_SDS_PLATID_REV_MAJOR_POS 24
#define JUNO_SDS_PLATID_CONFIG_POS 28

#define JUNO_SDS_PLATID_TYPE_POS 0

/*
 * Field masks and offsets for the JUNO_SDS_RESET_SYNDROME structure.
 */
#define JUNO_SDS_RESET_SYNDROME_POR_MASK 0x1
#define JUNO_SDS_RESET_SYNDROME_WDOGSCP_MASK 0x2
#define JUNO_SDS_RESET_SYNDROME_WDOGAP_MASK 0x4
#define JUNO_SDS_RESET_SYNDROME_SYSRESET_MASK 0x8
#define JUNO_SDS_RESET_SYNDROME_M3LOCKUP_MASK 0x10

#define JUNO_SDS_RESET_SYNDROME_POR_POS 0
#define JUNO_SDS_RESET_SYNDROME_WDOGSCP_POS 1
#define JUNO_SDS_RESET_SYNDROME_WDOGAP_POS 2
#define JUNO_SDS_RESET_SYNDROME_SYSRESET_POS 3
#define JUNO_SDS_RESET_SYNDROME_M3LOCKUP_POS 4

/*
 * Field masks and offsets for the JUNO_SDS_FEATURE_AVAILABILITY structure.
 */
#define JUNO_SDS_FEATURE_FIRMWARE_MASK 0x1
#define JUNO_SDS_FEATURE_DMC_MASK 0x2
#define JUNO_SDS_FEATURE_MESSAGING_MASK 0x4

#define JUNO_SDS_FEATURE_FIRMWARE_POS 0
#define JUNO_SDS_FEATURE_DMC_POS 1
#define JUNO_SDS_FEATURE_MESSAGING_POS 2

/*
 * Field masks and offsets for the JUNO_SDS_CPU_BOOTCTR structure.
 */
#define JUNO_SDS_CPU_BOOTCTR_CPU0_MASK 0xFF
#define JUNO_SDS_CPU_BOOTCTR_CPU1_MASK 0xFF00
#define JUNO_SDS_CPU_BOOTCTR_CPU2_MASK 0xFF0000
#define JUNO_SDS_CPU_BOOTCTR_CPU3_MASK 0xFF000000
#define JUNO_SDS_CPU_BOOTCTR_CPU4_MASK 0xFF
#define JUNO_SDS_CPU_BOOTCTR_CPU5_MASK 0xFF00
#define JUNO_SDS_CPU_BOOTCTR_CPU6_MASK 0xFF0000
#define JUNO_SDS_CPU_BOOTCTR_CPU7_MASK 0xFF000000

#define JUNO_SDS_CPU_BOOTCTR_CPU0_POS 0
#define JUNO_SDS_CPU_BOOTCTR_CPU1_POS 8
#define JUNO_SDS_CPU_BOOTCTR_CPU2_POS 16
#define JUNO_SDS_CPU_BOOTCTR_CPU3_POS 24
#define JUNO_SDS_CPU_BOOTCTR_CPU4_POS 0
#define JUNO_SDS_CPU_BOOTCTR_CPU5_POS 8
#define JUNO_SDS_CPU_BOOTCTR_CPU6_POS 16
#define JUNO_SDS_CPU_BOOTCTR_CPU7_POS 24

/*
 * Field masks and offsets for the JUNO_SDS_CPU_FLAGS structure.
 */
#define JUNO_SDS_CPU_FLAGS_CPU0_WFI_MASK 0x1
#define JUNO_SDS_CPU_FLAGS_CPU1_WFI_MASK 0x100
#define JUNO_SDS_CPU_FLAGS_CPU2_WFI_MASK 0x10000
#define JUNO_SDS_CPU_FLAGS_CPU3_WFI_MASK 0x1000000
#define JUNO_SDS_CPU_FLAGS_CPU4_WFI_MASK 0x1
#define JUNO_SDS_CPU_FLAGS_CPU5_WFI_MASK 0x100
#define JUNO_SDS_CPU_FLAGS_CPU6_WFI_MASK 0x10000
#define JUNO_SDS_CPU_FLAGS_CPU7_WFI_MASK 0x1000000

#define JUNO_SDS_CPU_FLAGS_CPU0_WFI_POS 0
#define JUNO_SDS_CPU_FLAGS_CPU1_WFI_POS 8
#define JUNO_SDS_CPU_FLAGS_CPU2_WFI_POS 16
#define JUNO_SDS_CPU_FLAGS_CPU3_WFI_POS 24
#define JUNO_SDS_CPU_FLAGS_CPU4_WFI_POS 0
#define JUNO_SDS_CPU_FLAGS_CPU5_WFI_POS 8
#define JUNO_SDS_CPU_FLAGS_CPU6_WFI_POS 16
#define JUNO_SDS_CPU_FLAGS_CPU7_WFI_POS 24

/*
 * Field masks and offsets for the JUNO_SDS_BOOTLOADER structure.
 */
#define JUNO_SDS_BOOTLOADER_VALID_MASK 0x1
#define JUNO_SDS_BOOTLOADER_OFFSET_MASK 0xFFFFFFFF
#define JUNO_SDS_BOOTLOADER_SIZE_MASK 0xFFFFFFFF

#define JUNO_SDS_BOOTLOADER_VALID_POS 0
#define JUNO_SDS_BOOTLOADER_OFFSET_POS 0
#define JUNO_SDS_BOOTLOADER_SIZE_POS 0

#endif /* JUNO_SDS_H */

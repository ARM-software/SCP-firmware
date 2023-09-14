/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Base address definitions for the SCP's sub-system and access extending
 *     into the rest of the CSS.
 */

#ifndef SCP_CSS_MMAP_H
#define SCP_CSS_MMAP_H

/* Base address and size of SCP's ITCM */
#define SCP_ITC_RAM_BASE (0x00000000)
#define SCP_ITC_RAM_SIZE (256 * 1024)

/* Base address and size of SCP's DTCM */
#define SCP_DTC_RAM_BASE (0x20000000)
#define SCP_DTC_RAM_SIZE (256 * 1024)

/* SCP sub-system peripherals */
#define SCP_SID_BASE (0x2A4A0000)
#define SCP_UART_BASE (0x44002000)
#define SCP_POWER_CONTROL_BASE (0x50000000)
#define SCP_SYSTEM_PIK_BASE    (0x50040000)

/* Base address of SCP expansion memory regions */
#define SCP_SOC_EXPANSION3_BASE (0x40000000) /* 64MB size */

/* SCP addresses mapped via ATU into address translation windows */
#define SCP_ADDRESS_TRANSLATION_WINDOW0_BASE (0x60000000)

/* Offsets within SCP's Address Translation Window0 */
#define SCP_ATW0_LCP_AND_CLUSTER_UTILITY_OFFSET (0x0)
#define SCP_ATW0_AP_PERIPHERAL_SRAM_OFFSET (0x10000000)

/*
 * LCP subsystem and Cluster Utility memory region that is addressable in the AP
 * memory map between 0x2_0000_0000 - 0x2_0FFF_FFFF is mapped in the SCP address
 * translation window 0 from 0x6000_0000 to 0x6FFF_FFFF via ATU configuration.
 */

/*
 * Size of SCP's view of per-cluster LCP subsystem and utility memory region.
 */
#define SCP_LCP_AND_CLUSTER_UTILITY_SIZE (0x200000)

/*
 * Offsets of various blocks within LCP subsystem and cluster utility that is
 * mapped into SCP's address translation window 0. These offsets are applicable
 * to each cluster in the system.
 */
#define SCP_CLUSTER_UTILITY_CORE_MANAGER_OFFSET (0x80000)

/* Core Manager base address for a cluster 'n' */
#define SCP_CLUSTER_UTILITY_CORE_MANAGER_BASE(n) \
    (SCP_ADDRESS_TRANSLATION_WINDOW0_BASE + \
     SCP_ATW0_LCP_AND_CLUSTER_UTILITY_OFFSET + \
     (n * SCP_LCP_AND_CLUSTER_UTILITY_SIZE) + \
     SCP_CLUSTER_UTILITY_CORE_MANAGER_OFFSET)

#endif /* SCP_CSS_MMAP_H */

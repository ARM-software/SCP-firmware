/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_SGI575_MMAP_H
#define SCP_SGI575_MMAP_H

#include <stdint.h>

/*
 * Top-level base addresses
 */
#define SCP_EXPANSION1_BASE           UINT32_C(0x01000000)
#define SCP_EXPANSION2_BASE           UINT32_C(0x21000000)
#define SCP_EXPANSION3_BASE           UINT32_C(0x40000000)
#define SCP_PERIPHERAL_BASE           UINT32_C(0x44000000)
#define SCP_MEMORY_CONTROLLER         UINT32_C(0x4E000000)
#define SCP_POWER_PERIPHERAL_BASE     UINT32_C(0x50000000)
#define SCP_SYS0_BASE                 UINT32_C(0x60000000)
#define SCP_SYS1_BASE                 UINT32_C(0xA0000000)
#define SCP_PPB_BASE_INTERNAL         UINT32_C(0xE0000000)
#define SCP_PPB_BASE_EXTERNAL         UINT32_C(0xE0040000)

/*
 * Peripherals
 */
#define SCP_REFCLK_CNTCTL_BASE        (SCP_PERIPHERAL_BASE + 0x0000)
#define SCP_REFCLK_CNTBASE0_BASE      (SCP_PERIPHERAL_BASE + 0x1000)
#define SCP_UART_BASE                 (SCP_PERIPHERAL_BASE + 0x2000)
#define SCP_WDOG_BASE                 (SCP_PERIPHERAL_BASE + 0x6000)
#define SCP_CS_CNTCONTROL_BASE        (SCP_PERIPHERAL_BASE + 0xA000)
#define SCP_MHU_AP_BASE               (SCP_PERIPHERAL_BASE + 0x1000000)
#define SCP_MHU_MCP_BASE              (SCP_PERIPHERAL_BASE + 0x1600000)

/*
 * Power control peripherals
 */
#define SCP_PIK_SCP_BASE              (SCP_POWER_PERIPHERAL_BASE + 0x00000)
#define SCP_PIK_DEBUG_BASE            (SCP_POWER_PERIPHERAL_BASE + 0x20000)
#define SCP_SCP_SENSOR_DEBUG_BASE     (SCP_POWER_PERIPHERAL_BASE + 0x30000)
#define SCP_PIK_SYSTEM_BASE           (SCP_POWER_PERIPHERAL_BASE + 0x40000)
#define SCP_SENSOR_SYSTEM_BASE        (SCP_POWER_PERIPHERAL_BASE + 0x50000)
#define SCP_SENSOR_CLUS0_BASE         (SCP_POWER_PERIPHERAL_BASE + 0x70000)
#define SCP_SENSOR_CLUS1_BASE         (SCP_POWER_PERIPHERAL_BASE + 0x90000)
#define SCP_PIK_CLUSTER_BASE(n)       ((SCP_POWER_PERIPHERAL_BASE + 0x60000) + \
                                      ((n) * 0x20000))

/*
 * PPU base address
 */
#define SCP_PPU_SCP_BASE              (SCP_PIK_SCP_BASE + 0x1000)
#define SCP_PPU_SYS0_BASE             (SCP_PIK_SYSTEM_BASE + 0x1000)
#define SCP_PPU_SYS1_BASE             (SCP_PIK_SYSTEM_BASE + 0x5000)
#define SCP_PPU_DEBUG_BASE            (SCP_PIK_DEBUG_BASE + 0x1000)
#define SCP_PPU_CLUSTER_BASE(n)       (SCP_PIK_CLUSTER_BASE((n)) + 0x1000)
#define SCP_PPU_CORE_BASE(n, m)       (SCP_PPU_CLUSTER_BASE((n)) + \
                                       ((m) + 1) * 0x1000)

/*
 * System access port 0
 */
#define SCP_CMN600_BASE               (SCP_SYS0_BASE + 0x10000000)

/*
 * System access port 1
 */
#define SCP_TRUSTED_RAM_BASE          (SCP_SYS1_BASE + 0x04000000)
#define SCP_NONTRUSTED_RAM_BASE       (SCP_SYS1_BASE + 0x06000000)
#define SSC_BASE                      (SCP_SYS1_BASE + 0x2A420000)
#define SCP_REFCLK_CNTCONTROL_BASE    (SCP_SYS1_BASE + 0x2A430000)

/*
 * Base addresses of MHU devices
 */
#define SCP_MHU_SCP_AP_NS(cluster)    (SCP_MHU_AP_BASE + \
                                      (0x10000 * cluster) + 0x020)
#define SCP_MHU_AP_SCP_NS(cluster)    (SCP_MHU_AP_BASE + \
                                      (0x10000 * cluster) + 0x120)
#define SCP_MHU_SCP_AP_S(cluster)     (SCP_MHU_AP_BASE + \
                                      (0x10000 * cluster) + 0x200)
#define SCP_MHU_AP_SCP_S(cluster)     (SCP_MHU_AP_BASE + \
                                      (0x10000 * cluster) + 0x300)

#define SCP_MHU_SCP_MCP_NS        (SCP_MHU_MCP_BASE + 0x020)
#define SCP_MHU_MCP_SCP_NS        (SCP_MHU_MCP_BASE + 0x120)
#define SCP_MHU_SCP_MCP_S         (SCP_MHU_MCP_BASE + 0x200)
#define SCP_MHU_MCP_SCP_S         (SCP_MHU_MCP_BASE + 0x300)

#endif /* SCP_SGI575_MMAP_H */

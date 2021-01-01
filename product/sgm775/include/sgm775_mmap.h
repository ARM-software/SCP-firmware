/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGM775_MMAP_H
#define SGM775_MMAP_H

#include <stdint.h>

/*
 * Top-level base addresses
 */
#define EXPANSION0_BASE           UINT32_C(0x40000000)
#define PERIPHERAL_BASE           UINT32_C(0x44000000)
#define POWER_PERIPHERAL_BASE     UINT32_C(0x50000000)
#define SYS0_BASE                 UINT32_C(0x60000000)
#define SYS1_BASE                 UINT32_C(0xA0000000)
#define PPB_INTERNAL_BASE         UINT32_C(0xE0000000)
#define PPB_EXTERNAL_BASE         UINT32_C(0xE0040000)
#define EXPANSION1_BASE           UINT32_C(0xE0100000)

/*
 * Peripherals
 */
#define REFCLK_CNTCTL_BASE        (PERIPHERAL_BASE + 0x0000)
#define REFCLK_CNTBASE0_BASE      (PERIPHERAL_BASE + 0x1000)
#define WDOG_BASE                 (PERIPHERAL_BASE + 0x6000)
#define S32K_CNTCONTROL_BASE      (PERIPHERAL_BASE + 0x7000)
#define S32K_CNTCTL_BASE          (PERIPHERAL_BASE + 0x8000)
#define S32K_CNTBASE0_BASE        (PERIPHERAL_BASE + 0x9000)
#define CS_CNTCONTROL_BASE        (PERIPHERAL_BASE + 0xA000)

/*
 * Power control peripherals
 */
#define PIK_SCP_BASE              (POWER_PERIPHERAL_BASE + 0x00000)
#define PIK_DEBUG_BASE            (POWER_PERIPHERAL_BASE + 0x20000)
#define SENSOR_DEBUG_BASE         (POWER_PERIPHERAL_BASE + 0x30000)
#define PIK_SYSTEM_BASE           (POWER_PERIPHERAL_BASE + 0x40000)
#define SENSOR_SYSTEM_BASE        (POWER_PERIPHERAL_BASE + 0x50000)
#define PIK_CLUS0_BASE            (POWER_PERIPHERAL_BASE + 0x60000)
#define SENSOR_CLUS0_BASE         (POWER_PERIPHERAL_BASE + 0x70000)
#define PIK_CLUS1_BASE            (POWER_PERIPHERAL_BASE + 0x80000)
#define SENSOR_CLUS1_BASE         (POWER_PERIPHERAL_BASE + 0x90000)
#define PIK_GPU_BASE              (POWER_PERIPHERAL_BASE + 0xA0000)
#define SENSOR_GPU_BASE           (POWER_PERIPHERAL_BASE + 0xB0000)
#define PIK_VPU_BASE              (POWER_PERIPHERAL_BASE + 0xC0000)
#define SENSOR_VPU_BASE           (POWER_PERIPHERAL_BASE + 0xD0000)
#define PIK_DPU_BASE              (POWER_PERIPHERAL_BASE + 0xE0000)
#define SENSOR_DPU_BASE           (POWER_PERIPHERAL_BASE + 0xF0000)

/*
 * PPU base address
 */
#define PPU_SCP_BASE              (PIK_SCP_BASE + 0x1000)
#define PPU_SYS0_BASE             (PIK_SYSTEM_BASE + 0x1000)
#define PPU_SYS1_BASE             (PIK_SYSTEM_BASE + 0x2000)
#define PPU_DEBUG_BASE            (PIK_DEBUG_BASE + 0x1000)
#define PPU_CLUS0CORE0_BASE       (PIK_CLUS0_BASE + 0x2000)
#define PPU_CLUS0CORE1_BASE       (PIK_CLUS0_BASE + 0x3000)
#define PPU_CLUS0CORE2_BASE       (PIK_CLUS0_BASE + 0x4000)
#define PPU_CLUS0CORE3_BASE       (PIK_CLUS0_BASE + 0x5000)
#define PPU_CLUS0CORE4_BASE       (PIK_CLUS0_BASE + 0x6000)
#define PPU_CLUS0CORE5_BASE       (PIK_CLUS0_BASE + 0x7000)
#define PPU_CLUS0CORE6_BASE       (PIK_CLUS0_BASE + 0x8000)
#define PPU_CLUS0CORE7_BASE       (PIK_CLUS0_BASE + 0x9000)
#define PPU_CLUS0_BASE            (PIK_CLUS0_BASE + 0x1000)
#define PPU_CLUS1CORE0_BASE       (PIK_CLUS1_BASE + 0x2000)
#define PPU_CLUS1CORE1_BASE       (PIK_CLUS1_BASE + 0x3000)
#define PPU_CLUS1CORE2_BASE       (PIK_CLUS1_BASE + 0x4000)
#define PPU_CLUS1CORE3_BASE       (PIK_CLUS1_BASE + 0x5000)
#define PPU_CLUS1_BASE            (PIK_CLUS1_BASE + 0x1000)
#define PPU_GPU_BASE              (PIK_GPU_BASE + 0x1000)
#define PPU_VPU_BASE              (PIK_VPU_BASE + 0x1000)
#define PPU_DPU0_BASE             (PIK_DPU_BASE + 0x2000)
#define PPU_DPU1_BASE             (PIK_DPU_BASE + 0x3000)

/*
 * System access port 1
 */
#define TRUSTED_RAM_BASE          (SYS1_BASE + 0x04000000)
#define NONTRUSTED_RAM_BASE       (SYS1_BASE + 0x06000000)
#define SSC_BASE                  (SYS1_BASE + 0x2A420000)
#define REFCLK_CNTCONTROL_BASE    (SYS1_BASE + 0x2A430000)
#define MHU_BASE                  (SYS1_BASE + 0x2B1F0000)

/*
 * Base addresses of MHU devices
 */

#define MHU_SCP_INTR_L_BASE (MHU_BASE)
#define MHU_SCP_INTR_H_BASE (MHU_BASE + 0x0020)
#define MHU_CPU_INTR_L_BASE (MHU_BASE + 0x0100)
#define MHU_CPU_INTR_H_BASE (MHU_BASE + 0x0120)
#define MHU_SCP_INTR_S_BASE (MHU_BASE + 0x0200)
#define MHU_CPU_INTR_S_BASE (MHU_BASE + 0x0300)

#endif /* SGM775_MMAP_H */

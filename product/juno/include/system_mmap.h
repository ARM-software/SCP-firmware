/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *    SoC-level memory map.
 */

#ifndef SYSTEM_MMAP_H
#define SYSTEM_MMAP_H

#include <stdint.h>

#define PERIPHERAL_BASE           UINT32_C(0x44000000)
#define EXTERNAL_RAM_BASE         UINT32_C(0x60000000)
#define EXTERNAL_DEV_BASE         UINT32_C(0xA0000000)
#define PPB_BASE_INTERNAL         UINT32_C(0xE0000000)
#define PPB_BASE_EXTERNAL         UINT32_C(0xE0040000)
#define VENDOR_EXTENSION_BASE     UINT32_C(0xE0100000)

#define TRUSTED_RAM_BASE          (EXTERNAL_DEV_BASE + 0x04000000)
#define SSC_BASE                  (EXTERNAL_DEV_BASE + 0x2A420000)
#define DMC400_BASE               (EXTERNAL_DEV_BASE + 0x2B0A0000)
#define NONTRUSTED_RAM_BASE       (EXTERNAL_DEV_BASE + 0x2E000000)
#define WDOG_BASE                 (PERIPHERAL_BASE + 0x6000)
#define CTI_BASE                  (PPB_BASE_EXTERNAL + 0x4000)
#define CONFIG_BASE               (PERIPHERAL_BASE + 0x10000)

#define REFCLK_CNTCONTROL_BASE    (EXTERNAL_DEV_BASE + 0x2A430000)
#define REFCLK_CNTCTL_BASE        (PERIPHERAL_BASE)
#define REFCLK_CNTBASE0_BASE      (PERIPHERAL_BASE + 0x1000)
#define S32K_CNTCONTROL_BASE      (PERIPHERAL_BASE + 0x7000)
#define S32K_CNTCTL_BASE          (PERIPHERAL_BASE + 0x8000)
#define S32K_CNTBASE0_BASE        (PERIPHERAL_BASE + 0x9000)
#define CS_CNTCONTROL_BASE        (PERIPHERAL_BASE + 0xA000)

#define PPU_BASE                  (PERIPHERAL_BASE + 0x20000)
#define PPU_BIG_CPU0_BASE         (PPU_BASE + 0x0000)
#define PPU_BIG_CPU1_BASE         (PPU_BASE + 0x0020)
#define PPU_BIG_SSTOP_BASE        (PPU_BASE + 0x0080)
#define PPU_LITTLE_CPU0_BASE      (PPU_BASE + 0x0100)
#define PPU_LITTLE_CPU1_BASE      (PPU_BASE + 0x0120)
#define PPU_LITTLE_CPU2_BASE      (PPU_BASE + 0x0140)
#define PPU_LITTLE_CPU3_BASE      (PPU_BASE + 0x0160)
#define PPU_LITTLE_SSTOP_BASE     (PPU_BASE + 0x0180)
#define PPU_GPUTOP_BASE           (PPU_BASE + 0x0200)
#define PPU_SYSTOP_BASE           (PPU_BASE + 0x0300)
#define PPU_DBGSYS_BASE           (PPU_BASE + 0x0320)
#define PPU_ID_BASE               (PPU_BASE + 0x0FD0)

#define MHU_BASE                  (EXTERNAL_DEV_BASE + 0x2B1F0000)
#define MHU_SCP_INTR_L_BASE       (MHU_BASE + 0x0000)
#define MHU_SCP_INTR_H_BASE       (MHU_BASE + 0x0020)
#define MHU_CPU_INTR_L_BASE       (MHU_BASE + 0x0100)
#define MHU_CPU_INTR_H_BASE       (MHU_BASE + 0x0120)
#define MHU_SCP_INTR_S_BASE       (MHU_BASE + 0x0200)
#define MHU_CPU_INTR_S_BASE       (MHU_BASE + 0x0300)

#define SCS_DEMCR_ADDR UINT32_C(0xE000EDFC)
#define DWT_CTRL_ADDR  UINT32_C(0xE0001000)
#define DWT_CYCCNT     UINT32_C(0xE0001004)

#endif /* SYSTEM_MMAP_H */

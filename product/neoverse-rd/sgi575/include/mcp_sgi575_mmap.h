/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MCP_SGI575_MMAP_H
#define MCP_SGI575_MMAP_H

#include <stdint.h>

/*
 * Top-level base addresses
 */
#define MCP_SOC_EXPANSION1_BASE UINT32_C(0x01000000)
#define MCP_SOC_EXPANSION2_BASE UINT32_C(0x21000000)
#define MCP_SOC_EXPANSION3_BASE UINT32_C(0x40000000)
#define MCP_MHU_SCP_BASE        UINT32_C(0x45600000)
#define MCP_SOC_EXPANSION4_BASE UINT32_C(0x48000000)
#define MCP_PERIPH_BASE         UINT32_C(0x4C000000)
#define MCP_MEMORY_CONTROLLER   UINT32_C(0x4E000000)
#define MCP_POWER_PERIPH_BASE   UINT32_C(0x50000000)
#define MCP_SYS0_BASE           UINT32_C(0x60000000)
#define MCP_SYS1_BASE           UINT32_C(0xA0000000)
#define MCP_PPB_BASE_INTERNAL   UINT32_C(0xE0000000)
#define MCP_PPB_BASE_EXTERNAL   UINT32_C(0xE0040000)

/*
 * Peripherals
 */
#define MCP_REFCLK_CNTCTL_BASE   (MCP_PERIPH_BASE)
#define MCP_REFCLK_CNTBASE0_BASE (MCP_PERIPH_BASE + 0x1000)
#define MCP_UART0_BASE           (MCP_PERIPH_BASE + 0x2000)
#define MCP_UART1_BASE           (MCP_PERIPH_BASE + 0x3000)
#define MCP_WDOG_BASE            (MCP_PERIPH_BASE + 0x6000)
#define MCP_MHU_AP_BASE          (MCP_PERIPH_BASE + 0x400000)

/*
 * Power control peripherals
 */
#define MCP_PIK_BASE (MCP_POWER_PERIPH_BASE)

/*
 * System access port 1
 */
#define MCP_REFCLK_CNTCONTROL_BASE (MCP_SYS1_BASE + 0x2A430000)

/*
 * Base addresses of MHU devices
 */
#define MCP_MHU_SCP_MCP_NS (MCP_MHU_SCP_BASE + 0x020)
#define MCP_MHU_MCP_SCP_NS (MCP_MHU_SCP_BASE + 0x120)
#define MCP_MHU_SCP_MCP_S  (MCP_MHU_SCP_BASE + 0x200)
#define MCP_MHU_MCP_SCP_S  (MCP_MHU_SCP_BASE + 0x300)

#endif /* MCP_SGI575_MMAP_H */

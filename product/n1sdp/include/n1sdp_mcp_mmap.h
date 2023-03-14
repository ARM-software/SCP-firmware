/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef N1SDP_MCP_MMAP_H
#define N1SDP_MCP_MMAP_H

#include <stdint.h>

/*
 * Top-level base addresses
 */
#define MCP_SOC_EXPANSION1_BASE       UINT32_C(0x01000000)
#define MCP_SOC_EXPANSION2_BASE       UINT32_C(0x21000000)
#define MCP_SOC_EXPANSION3_BASE       UINT32_C(0x40000000)
#define MCP_SOC_EXPANSION4_BASE       UINT32_C(0x48000000)
#define MCP_PERIPH_BASE               UINT32_C(0x4C000000)
#define MCP_MEMORY_CONTROLLER         UINT32_C(0x4E000000)
#define MCP_POWER_PERIPH_BASE         UINT32_C(0x50000000)
#define MCP_SYS0_BASE                 UINT32_C(0x60000000)
#define MCP_SYS1_BASE                 UINT32_C(0xA0000000)
#define MCP_PPB_BASE_INTERNAL         UINT32_C(0xE0000000)
#define MCP_PPB_BASE_EXTERNAL         UINT32_C(0xE0040000)

/*
 * Peripherals
 */
#define MCP_REFCLK_CNTCTL_BASE        (MCP_PERIPH_BASE)
#define MCP_REFCLK_CNTBASE0_BASE      (MCP_PERIPH_BASE + 0x1000)
#define MCP_UART0_BASE                (MCP_PERIPH_BASE + 0x2000)
#define MCP_WDOG_BASE                 (MCP_PERIPH_BASE + 0x6000)

#define MCP_I2C0_BASE                 (0x3FFFE000)
#define MCP_I2C1_BASE                 (0x3FFFF000)

/*
 * Power control peripherals
 */
#define MCP_PIK_BASE                  (MCP_POWER_PERIPH_BASE)

/*
 * Base addresses of MHUv1 devices
 */
#define MCP_MHU_AP_BASE               (MCP_PERIPH_BASE + 0x400000)
#define MCP_MHU_SCP_BASE              UINT32_C(0x45600000)

#define MHU_MCP_TO_AP_NS              (MCP_MHU_AP_BASE + 0x0020)
#define MHU_AP_TO_MCP_NS              (MCP_MHU_AP_BASE + 0x0120)
#define MHU_MCP_TO_AP_S               (MCP_MHU_AP_BASE + 0x0200)
#define MHU_AP_TO_MCP_S               (MCP_MHU_AP_BASE + 0x0300)

#define MHU_SCP_TO_MCP_NS             (MCP_MHU_SCP_BASE + 0x0020)
#define MHU_MCP_TO_SCP_NS             (MCP_MHU_SCP_BASE + 0x0120)
#define MHU_SCP_TO_MCP_S              (MCP_MHU_SCP_BASE + 0x0200)
#define MHU_MCP_TO_SCP_S              (MCP_MHU_SCP_BASE + 0x0300)

/*
 * Shared memory regions
 */
#define MCP_AP_SHARED_SECURE_RAM      (MCP_PERIPH_BASE + 0x420000)
#define MCP_AP_SHARED_NONSECURE_RAM   (MCP_PERIPH_BASE + 0x410000)
#define MCP_SCP_SHARED_SECURE_RAM     (0x45620000)
#define MCP_SCP_SHARED_NONSECURE_RAM  (0x45610000)

/*
 * System access port 1
 */
#define MCP_REFCLK_CNTCONTROL_BASE (MCP_SYS1_BASE + 0x2A430000)
#define MCP_NONTRUSTED_RAM_BASE    (MCP_SYS1_BASE + 0x06000000)
/* SCP MCP SCMI payload area at 31KB offset from base */
#define MCP_SCP_NS_MAILBOX_SRAM (MCP_NONTRUSTED_RAM_BASE + 0x7C00)

#endif /* N1SDP_MCP_MMAP_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_SOC_MMAP_H
#define SCP_SOC_MMAP_H

#include "scp_mmap.h"

#define SCP_NOR0_FLASH_BASE (SCP_SOC_EXPANSION1_BASE + 0x07000000)
#define SCP_PLL_BASE        (SCP_SOC_EXPANSION3_BASE + 0x03000000)

#define SCP_PLL_SYSPLL       (SCP_PLL_BASE + 0x00000000)
#define SCP_PLL_DMC          (SCP_PLL_BASE + 0x00000010)
#define SCP_PLL_INTERCONNECT (SCP_PLL_BASE + 0x00000020)

#define SCP_PLL_CPU0 (SCP_PLL_BASE + 0x00000100)
#define SCP_PLL_CPU1 (SCP_PLL_BASE + 0x00000104)
#define SCP_PLL_CPU2 (SCP_PLL_BASE + 0x00000108)
#define SCP_PLL_CPU3 (SCP_PLL_BASE + 0x0000010C)

#endif /* SCP_SOC_MMAP_H */

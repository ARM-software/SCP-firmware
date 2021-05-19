/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_SOC_MMAP_H
#define SCP_SOC_MMAP_H

#include "scp_mmap.h"

#define SCP_PLL_BASE (SCP_SOC_EXPANSION3_BASE + 0x03000000)

#define SCP_PLL_SYSPLL (SCP_PLL_BASE + 0x00000000)
#define SCP_PLL_DISPLAY (SCP_PLL_BASE + 0x00000014)
#define SCP_PLL_PIX0 (SCP_PLL_BASE + 0x00000018)
#define SCP_PLL_PIX1 (SCP_PLL_BASE + 0x0000001C)
#define SCP_PLL_INTERCONNECT (SCP_PLL_BASE + 0x00000020)

#define SCP_PLL_CPU_TYPE0 (SCP_PLL_BASE + 0x00000100)
#define SCP_PLL_CPU_TYPE1 (SCP_PLL_BASE + 0x00000104)
#define SCP_PLL_CPU_TYPE2 (SCP_PLL_BASE + 0x00000108)

#endif /* SCP_SOC_MMAP_H */

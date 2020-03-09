/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCP_SYSTEM_MMAP_H
#define SCP_SYSTEM_MMAP_H

#include "scp_rdn1e1_mmap.h"

#define SCP_PLAT_BASE           (SCP_SYS0_BASE + 0x3FFE0000)

#define SCP_PLL_BASE            (SCP_EXPANSION2_BASE + 0x01000000)

#define SCP_PLL_SYSPLL          (SCP_PLL_BASE)
#define SCP_PLL_CLUS0           (SCP_PLL_BASE + 0x00000004)
#define SCP_PLL_CLUS1           (SCP_PLL_BASE + 0x00000008)
#define SCP_PLL_DMC             (SCP_PLL_BASE + 0x00000010)
#define SCP_PLL_INTERCONNECT    (SCP_PLL_BASE + 0x00000020)

#define SCP_PLL_CPU0            (SCP_PLL_BASE + 0x00000100)
#define SCP_PLL_CPU1            (SCP_PLL_BASE + 0x00000104)
#define SCP_PLL_CPU2            (SCP_PLL_BASE + 0x00000108)
#define SCP_PLL_CPU3            (SCP_PLL_BASE + 0x0000010C)
#define SCP_PLL_CPU4            (SCP_PLL_BASE + 0x00000110)
#define SCP_PLL_CPU5            (SCP_PLL_BASE + 0x00000114)
#define SCP_PLL_CPU6            (SCP_PLL_BASE + 0x00000118)
#define SCP_PLL_CPU7            (SCP_PLL_BASE + 0x0000011C)

#define SCP_DDR_PHY0            (SCP_SYS0_BASE + 0x3FB60000)
#define SCP_DDR_PHY1            (SCP_SYS0_BASE + 0x3FB70000)

#define SCP_DMC0                (SCP_MEMORY_CONTROLLER + 0x00000000)
#define SCP_DMC1                (SCP_MEMORY_CONTROLLER + 0x00100000)

#define SCP_SENSOR_SOC_TEMP     (SCP_PLAT_BASE + 0x00000080)
#define SCP_PLATFORM_ID         (SCP_PLAT_BASE + 0x000000E0)

#endif /* SCP_SYSTEM_MMAP_H*/

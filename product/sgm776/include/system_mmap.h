/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSTEM_MMAP_H
#define SYSTEM_MMAP_H

#include "sgm776_mmap.h"

#define DMC_EXTERNAL0       (SYS0_BASE + 0x3FBE0000)
#define DMC_EXTERNAL1       (SYS0_BASE + 0x3FBF0000)
#define DMC_EXTERNAL2       (SYS0_BASE + 0x3FC00000)
#define DMC_EXTERNAL3       (SYS0_BASE + 0x3FC10000)

#define BOARD_UART1_BASE    (SYS0_BASE + 0x3FF70000)
#define PLAT_BASE           (SYS0_BASE + 0x3FFE0000)

#define PLL_GPU             (PLAT_BASE + 0x00000008)
#define PLL_SYSTEM          (PLAT_BASE + 0x0000000C)
#define PLL_VIDEO           (PLAT_BASE + 0x00000010)
#define PLL_DISPLAY         (PLAT_BASE + 0x00000014)

#define PIX0_CONTROL        (PLAT_BASE + 0x00000018)
#define PIX1_CONTROL        (PLAT_BASE + 0x0000001C)

#define SWCLKTCK_CONTROL    (PLAT_BASE + 0x00000020)
#define SENSOR_SOC_TEMP     (PLAT_BASE + 0x00000080)
#define PLATFORM_ID         (PLAT_BASE + 0x000000E0)

#define PLL_CLUS0_0         (PLAT_BASE + 0x00000100)
#define PLL_CLUS0_1         (PLAT_BASE + 0x00000104)
#define PLL_CLUS0_2         (PLAT_BASE + 0x00000108)
#define PLL_CLUS0_3         (PLAT_BASE + 0x0000010C)
#define PLL_CLUS0_4         (PLAT_BASE + 0x00000110)
#define PLL_CLUS0_5         (PLAT_BASE + 0x00000114)
#define PLL_CLUS0_6         (PLAT_BASE + 0x00000118)
#define PLL_CLUS0_7         (PLAT_BASE + 0x0000011C)

#define DDR_PHY0            (SYS0_BASE + 0x3FB60000)
#define DDR_PHY1            (SYS0_BASE + 0x3FB70000)
#define DDR_PHY2            (SYS0_BASE + 0x3FB80000)
#define DDR_PHY3            (SYS0_BASE + 0x3FB90000)

#define GPV_CCI_GPU1        (SYS1_BASE + 0x2A004000)
#define GPV_CCI_GPU0        (SYS1_BASE + 0x2A005000)
#define GPV_CCI_LITTLE      (SYS1_BASE + 0x2A006000)
#define GPV_CCI_BIG         (SYS1_BASE + 0x2A007000)
#define GPV_VPU             (SYS1_BASE + 0x2A243000)
#define GPV_DPU0            (SYS1_BASE + 0x2A244000)
#define GPV_DPU1            (SYS1_BASE + 0x2A245000)

#define DMC_INTERNAL0       (SYS1_BASE + 0x2A500000)
#define DMC_INTERNAL1       (SYS1_BASE + 0x2A540000)
#define DMC_INTERNAL2       (SYS1_BASE + 0x2A580000)
#define DMC_INTERNAL3       (SYS1_BASE + 0x2A5C0000)


#endif /* SYSTEM_MMAP_H */

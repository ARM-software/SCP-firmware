/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Core Manager and clock control registers
 */

#ifndef CORE_MANAGER_H
#define CORE_MANAGER_H

#include "scp_css_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

// clang-format off
struct core_manager_reg {
           uint8_t   RESERVED0[0x10 - 0x00];
   FWK_RW  uint32_t  PE_STATIC_CONFIG;
           uint8_t   RESERVED1[0x18 - 0x14];
   FWK_RW  uint32_t  PE_RVBARADDR_LW;
   FWK_RW  uint32_t  PE_RVBARADDR_UP;
           uint8_t   RESERVED2[0x030-0x020];
   FWK_R   uint32_t  PE_STATUS;
           uint8_t   RESERVED3[0x800-0x034];
   FWK_RW  uint32_t  CLUS_MGRCLK_CTRL;
   FWK_RW  uint32_t  CLUS_MGRCLK_DIV1;
           uint8_t   RESERVED4[0x820 - 0x808];
   FWK_RW  uint32_t  CLUS_EXP1CLK_CTRL;
   FWK_RW  uint32_t  CLUS_EXP1CLK_DIV;
           uint8_t   RESERVED5[0x830 - 0x828];
   FWK_RW  uint32_t  CLUS_EXP2CLK_CTRL;
   FWK_RW  uint32_t  CLUS_EXP2CLK_DIV;
           uint8_t   RESERVED6[0x840 - 0x838];
   FWK_RW  uint32_t  CLUS_GICCLK_CTRL;
   FWK_RW  uint32_t  CLUS_GICCLK_DIV1;
           uint8_t   RESERVED7[0x850 -0x848];
   FWK_RW  uint32_t  CLUS_PERIPHCLK_CTRL;
   FWK_RW  uint32_t  CLUS_PERIPHCLK_DIV1;
           uint8_t   RESERVED8[0x860 - 0x858];
   FWK_RW  uint32_t  CORECLK_CTRL;
   FWK_RW  uint32_t  CORECLK_DIV1;
   FWK_RW  uint32_t  CORECLK_MOD1;
           uint8_t   RESERVED9[0xA00 - 0x086C];
   FWK_R   uint32_t  CLKFORCE_STATUS;
   FWK_W   uint32_t  CLKFORCE_SET;
   FWK_W   uint32_t  CLKFORCE_CLR;
           uint8_t   RESERVED10[0x0FB4 - 0x0A0C];
   FWK_R   uint32_t  CAP3;
   FWK_R   uint32_t  CAP2;
   FWK_R   uint32_t  CAP1;
   FWK_R   uint32_t  PWR_CTRL_CONFIG;
           uint8_t   RESERVED11[0xFD0 - 0xFC4];
   FWK_R   uint32_t  PID4;
   FWK_R   uint32_t  PID5;
   FWK_R   uint32_t  PID6;
   FWK_R   uint32_t  PID7;
   FWK_R   uint32_t  PID0;
   FWK_R   uint32_t  PID1;
   FWK_R   uint32_t  PID2;
   FWK_R   uint32_t  PID3;
   FWK_R   uint32_t  ID0;
   FWK_R   uint32_t  ID1;
   FWK_R   uint32_t  ID2;
   FWK_R   uint32_t  ID3;
};
// clang-format on

#define SCP_CLUSTER_UTILITY_CORE_MANAGER_PTR(IDX) \
    ((struct core_manager_reg *)SCP_CLUSTER_UTILITY_CORE_MANAGER_BASE(IDX))

#endif /* CORE_MANAGER_H */

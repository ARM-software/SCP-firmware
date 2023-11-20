/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CPU_PIK_H
#define CPU_PIK_H

#include "scp_css_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

// clang-format off

struct pik_cpu_reg {
           uint8_t   RESERVED0[0x10 - 0x00];
   FWK_RW  uint32_t  PE_STATIC_CONFIG;
           uint32_t  RESERVED1;
   FWK_RW  uint32_t  PE_RVBARADDR_LW;
   FWK_RW  uint32_t  PE_RVBARADDR_UP;
           uint8_t   RESERVED2[0x030-0x020];
   FWK_R   uint32_t  PE_STATUS;
           uint8_t   RESERVED3[0x800-0x034];
   FWK_RW  uint32_t  CLUS_PPUCLK_CTRL;
   FWK_RW  uint32_t  CLUS_PPUCLK_DIV1;
           uint8_t   RESERVED4[0x840 - 0x808];
   FWK_RW  uint32_t  CLUS_GICCLK_CTRL;
   FWK_RW  uint32_t  CLUS_GICCLK_DIV1;
           uint8_t   RESERVED5[0x850 -0x848];
   FWK_RW  uint32_t  CLUS_PERIPHCLK_CTRL;
   FWK_RW  uint32_t  CLUS_PERIPHCLK_DIV1;
           uint8_t   RESERVED6[0x860 - 0x858];
   FWK_RW  uint32_t  CORECLK_CTRL;
   FWK_RW  uint32_t  CORECLK_DIV1;
   FWK_RW  uint32_t  CORECLK_MOD1;
           uint8_t   RESERVED7[0xA00 - 0x086C];
   FWK_RW  uint32_t  CLKFORCE_STATUS;
   FWK_RW  uint32_t  CLKFORCE_SET;
   FWK_RW  uint32_t  CLKFORCE_CLR;
           uint8_t   RESERVED8[0x0FB4 - 0x0A0C];
   FWK_R   uint32_t  CAP3;
   FWK_R   uint32_t  CAP2;
   FWK_R   uint32_t  CAP1;
   FWK_R   uint32_t  PWR_CTRL_CONFIG;
           uint8_t   RESERVED9[0xFD0 - 0xFC4];
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

#define CLUSTER_PIK_PTR(IDX) ((struct pik_cpu_reg *)SCP_PIK_CLUSTER_BASE(IDX))
// clang-format on

#endif /* CPU_PIK_H */

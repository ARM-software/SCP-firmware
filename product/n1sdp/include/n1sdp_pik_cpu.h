/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef N1SDP_PIK_CPU_H
#define N1SDP_PIK_CPU_H

#include <stdint.h>
#include <fwk_macros.h>

#define PE_COUNT_MAX 16

/*!
 * \brief PE Static Configuration register definitions
 */
struct static_config_reg {
    FWK_RW  uint32_t STATIC_CONFIG;
    FWK_RW  uint32_t RVBARADDR_LW;
    FWK_RW  uint32_t RVBARADDR_UP;
            uint32_t RESERVED;
};

/*!
 * \brief AP cores clock control register definitions
 */
struct coreclk_reg {
    FWK_RW  uint32_t CTRL;
    FWK_RW  uint32_t DIV;
            uint32_t RESERVED;
    FWK_RW  uint32_t MOD;
};

/*!
 * \brief CPU (V8.2) PIK register definitions
 */
struct pik_cpu_reg {
  FWK_RW  uint32_t   CLUSTER_CONFIG;
          uint8_t    RESERVED0[0x10 - 0x4];
  struct static_config_reg  STATIC_CONFIG[PE_COUNT_MAX];
          uint8_t    RESERVED1[0x800 - 0x110];
  FWK_RW  uint32_t   PPUCLK_CTRL;
  FWK_RW  uint32_t   PPUCLK_DIV1;
          uint8_t    RESERVED2[0x810 - 0x808];
  FWK_RW  uint32_t   PCLK_CTRL;
          uint8_t    RESERVED3[0x820 - 0x814];
  FWK_RW  uint32_t   ATCLK_CTRL;
          uint8_t    RESERVED4[0x830 - 0x824];
  FWK_RW  uint32_t   GICCLK_CTRL;
          uint8_t    RESERVED5[0x840 - 0x834];
  FWK_RW  uint32_t   AMBACLK_CTRL;
          uint8_t    RESERVED6[0x850 - 0x844];
  FWK_RW  uint32_t   CLUSCLK_CTRL;
  FWK_RW  uint32_t   CLUSCLK_DIV1;
          uint8_t    RESERVED7[0x860 - 0x858];
  struct coreclk_reg CORECLK[8];
          uint8_t    RESERVED8[0xA00 - 0x8E0];
  FWK_R   uint32_t   CLKFORCE_STATUS;
  FWK_RW  uint32_t   CLKFORCE_SET;
  FWK_RW  uint32_t   CLKFORCE_CLR;
          uint8_t    RESERVED9[0xFB8 - 0xA0C];
  FWK_R   uint32_t   CAP2;
  FWK_R   uint32_t   CAP;
  FWK_R   uint32_t   PIK_CONFIG;
  FWK_R   uint8_t    RESERVED10[0xFD0 - 0xFC4];
  FWK_R   uint32_t   PID4;
  FWK_R   uint32_t   PID5;
  FWK_R   uint32_t   PID6;
  FWK_R   uint32_t   PID7;
  FWK_R   uint32_t   PID0;
  FWK_R   uint32_t   PID1;
  FWK_R   uint32_t   PID2;
  FWK_R   uint32_t   PID3;
  FWK_R   uint32_t   ID0;
  FWK_R   uint32_t   ID1;
  FWK_R   uint32_t   ID2;
  FWK_R   uint32_t   ID3;
};

#define PIK_CPU_CAP_CLUSSYNC       UINT32_C(0x00000001)
#define PIK_CPU_CAP_CORESYNC(CORE) ((uint32_t)(1 << ((CORE) + 1)))
#define PIK_CPU_CAP_PE_MASK        UINT32_C(0xF0000000)
#define PIK_CPU_CAP_PE_POS         28

#endif  /* N1SDP_PIK_CPU_H */

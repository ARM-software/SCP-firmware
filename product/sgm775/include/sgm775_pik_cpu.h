/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGM775_PIK_CPU_H
#define SGM775_PIK_CPU_H

#include <fwk_macros.h>

#include <stdint.h>

#define PE_COUNT_MAX  16

/*!
 * \brief CPU PIK V8 register definitions
 */
struct pik_cpu_reg_v8 {
  FWK_RW  uint32_t STATIC_CONFIG;
          uint8_t  RESERVED0[0x10 - 0x4];
  FWK_RW  uint32_t RVBARADDR0_LW;
  FWK_RW  uint32_t RVBARADDR0_UP;
  FWK_RW  uint32_t RVBARADDR1_LW;
  FWK_RW  uint32_t RVBARADDR1_UP;
  FWK_RW  uint32_t RVBARADDR2_LW;
  FWK_RW  uint32_t RVBARADDR2_UP;
  FWK_RW  uint32_t RVBARADDR3_LW;
  FWK_RW  uint32_t RVBARADDR3_UP;
  FWK_RW  uint32_t CLUSTER_CONFIG;
          uint8_t  RESERVED1[0x200 - 0x34];
  FWK_R   uint32_t DBG_RST_STATU;
  FWK_RW  uint32_t DBG_RST_SET;
  FWK_RW  uint32_t DBG_RST_CLR;
          uint8_t  RESERVED2[0x400 - 0x20C];
  FWK_RW  uint32_t CPUACTIVE_CTRL;
          uint8_t  RESERVED3[0x800 - 0x404];
  FWK_RW  uint32_t PPUCLK_CTRL;
  FWK_RW  uint32_t PPUCLK_DIV1;
  FWK_RW  uint32_t PPUCLK_DIV2;
  FWK_R   uint32_t RESERVED4;
  FWK_RW  uint32_t CPUCLK_CTRL;
  FWK_RW  uint32_t CPUCLK_DIV1;
  FWK_RW  uint32_t CPUCLK_DIV2;
  FWK_R   uint32_t RESERVED5;
  FWK_RW  uint32_t PCLKDBG_CTRL;
          uint8_t  RESERVED6[0x830 - 0x824];
  FWK_RW  uint32_t ATCLKDBG_CTRL;
          uint8_t  RESERVED7[0x840 - 0x834];
  FWK_RW  uint32_t ACLKCPU_CTRL;
          uint8_t  RESERVED8[0xA00 - 0x844];
  FWK_R   uint32_t CLKFORCE_STATUS;
  FWK_RW  uint32_t CLKFORCE_SET;
  FWK_RW  uint32_t CLKFORCE_CLR;
          uint8_t  RESERVED9[0xFC0 - 0xA0C];
  FWK_R   uint32_t PIK_CONFIG;
          uint8_t  RESERVED10[0xFD0 - 0xFC4];
  FWK_R   uint32_t PID4;
  FWK_R   uint32_t PID5;
  FWK_R   uint32_t PID6;
  FWK_R   uint32_t PID7;
  FWK_R   uint32_t PID0;
  FWK_R   uint32_t PID1;
  FWK_R   uint32_t PID2;
  FWK_R   uint32_t PID3;
  FWK_R   uint32_t ID0;
  FWK_R   uint32_t ID1;
  FWK_R   uint32_t ID2;
  FWK_R   uint32_t ID3;
};

/*!
 * \brief PE Static Configuration register definitions
 */
struct static_config {
  FWK_RW  uint32_t STATIC_CONFIG;
  FWK_RW  uint32_t RVBARADDR_LW;
  FWK_RW  uint32_t RVBARADDR_UP;
          uint32_t RESERVED;
};

/*!
 * \brief AP cores clock control register definitions
 */
struct ap_clk_ctrl {
  FWK_RW  uint32_t CORECLK_CTRL;
  FWK_RW  uint32_t CORECLK_DIV;
          uint32_t RESERVED;
  FWK_RW  uint32_t CORECLK_MOD;
};

/*!
 * \brief CPU PIK V8.2 register definitions
 */
struct pik_cpu_reg_v8_2 {
  FWK_RW  uint32_t     CLUSTER_CONFIG;
          uint8_t      RESERVED0[0x10 - 0x4];
  struct static_config STATIC_CONFIG[PE_COUNT_MAX];
          uint8_t      RESERVED1[0x800 - 0x110];
  FWK_RW  uint32_t     PPUCLK_CTRL;
  FWK_RW  uint32_t     PPUCLK_DIV1;
          uint8_t      RESERVED2[0x810 - 0x808];
  FWK_RW  uint32_t     PCLK_CTRL;
          uint8_t      RESERVED3[0x820 - 0x814];
  FWK_RW  uint32_t     ATCLK_CTRL;
          uint8_t      RESERVED4[0x830 - 0x824];
  FWK_RW  uint32_t     GICCLK_CTRL;
          uint8_t      RESERVED5[0x840 - 0x834];
  FWK_RW  uint32_t     AMBACLK_CTRL;
          uint8_t      RESERVED6[0x850 - 0x844];
  FWK_RW  uint32_t     CLUSCLK_CTRL;
  FWK_RW  uint32_t     CLUSCLK_DIV1;
          uint8_t      RESERVED7[0x860 - 0x858];
  struct ap_clk_ctrl   AP_CLK_CTRL[8];
          uint8_t      RESERVED8[0xA00 - 0x8E0];
  FWK_R   uint32_t     CLKFORCE_STATUS;
  FWK_RW  uint32_t     CLKFORCE_SET;
  FWK_RW  uint32_t     CLKFORCE_CLR;
          uint8_t      RESERVED9[0xFB8 - 0xA0C];
  FWK_R   uint32_t     CAP2;
  FWK_R   uint32_t     CAP;
  FWK_R   uint32_t     PIK_CONFIG;
          uint8_t      RESERVED10[0xFD0 - 0xFC4];
  FWK_R   uint32_t     PID4;
  FWK_R   uint32_t     PID5;
  FWK_R   uint32_t     PID6;
  FWK_R   uint32_t     PID7;
  FWK_R   uint32_t     PID0;
  FWK_R   uint32_t     PID1;
  FWK_R   uint32_t     PID2;
  FWK_R   uint32_t     PID3;
  FWK_R   uint32_t     ID0;
  FWK_R   uint32_t     ID1;
  FWK_R   uint32_t     ID2;
  FWK_R   uint32_t     ID3;
};

#define PIK_CPU_V8_2_CAP_CLUSSYNC          UINT32_C(0x00000001)
#define PIK_CPU_V8_2_CAP_CORESYNC(CORE)    ((uint32_t)(1 << ((CORE) + 1)))
#define PIK_CPU_V8_2_CAP_PE_MASK           UINT32_C(0xF0000000)
#define PIK_CPU_V8_2_CAP_PE_POS            28

#define PIK_CPU_V8_2_PIK_CONFIG_NO_OF_PPU  UINT32_C(0x0000000F)

#endif  /* SGM775_PIK_CPU_H */

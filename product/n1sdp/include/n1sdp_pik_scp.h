/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP PIK registers
 */

#ifndef N1SDP_PIK_SCP_H
#define N1SDP_PIK_SCP_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief SCP PIK register definitions
 */
struct pik_scp_reg {
            uint8_t   RESERVED0[0x10 - 0x0];
    FWK_RW  uint32_t  RESET_SYNDROME;
            uint8_t   RESERVED1[0x20 - 0x14];
    FWK_RW  uint32_t  SURVIVAL_RESET_STATUS;
            uint8_t   RESERVED2[0x30 - 0x24];
    FWK_RW  uint32_t  MCP_SEC_CTRL;
    FWK_RW  uint32_t  ADDR_TRANS;
    FWK_RW  uint32_t  DBG_ADDR_TRANS;
            uint8_t   RESERVED3[0x40 - 0x3C];
    FWK_RW  uint32_t  WS1_TIMER_MATCH;
    FWK_RW  uint32_t  WS1_TIMER_EN;
            uint8_t   RESERVED4[0x200 - 0x48];
    FWK_R   uint32_t  SS_RESET_STATUS;
    FWK_W   uint32_t  SS_RESET_SET;
    FWK_W   uint32_t  SS_RESET_CLR;
            uint8_t   RESERVED5[0x810 - 0x20C];
    FWK_RW  uint32_t  CORECLK_CTRL;
    FWK_RW  uint32_t  CORECLK_DIV1;
            uint8_t   RESERVED6[0x820 - 0x818];
    FWK_RW  uint32_t  ACLK_CTRL;
    FWK_RW  uint32_t  ACLK_DIV1;
            uint8_t   RESERVED7[0x830 - 0x828];
    FWK_RW  uint32_t  SYNCCLK_CTRL;
    FWK_RW  uint32_t  SYNCCLK_DIV1;
            uint8_t   RESERVED8[0xA10 - 0x838];
    FWK_R   uint32_t  PLL_STATUS[17];
            uint8_t   RESERVED9[0xA60 - 0xA54];
    FWK_R   uint32_t  CONS_MMUTCU_INT_STATUS;
    FWK_R   uint32_t  CONS_MMUTBU_INT_STATUS0;
    FWK_R   uint32_t  CONS_MMUTBU_INT_STATUS1;
            uint8_t   RESERVED10[0xB00 - 0xA6C];
    FWK_R   uint32_t  MHU_NS_INT_STATUS;
    FWK_R   uint32_t  MHU_S_INT_STATUS;
            uint8_t   RESERVED11[0xB20 - 0xB08];
    FWK_R   uint32_t  CPU_PPU_INT_STATUS[8];
    FWK_R   uint32_t  CLUS_PPU_INT_STATUS;
            uint8_t   RESERVED12[0xB60 - 0xB44];
    FWK_R   uint32_t  TIMER_INT_STATUS[8];
    FWK_R   uint32_t  CPU_PLL_LOCK_STATUS[8];
            uint8_t   RESERVED13[0xBC0 - 0xBA0];
    FWK_R   uint32_t  CPU_PLL_UNLOCK_STATUS[8];
            uint8_t   RESERVED14[0xBF0 - 0xBE0];
    FWK_R   uint32_t  CLUSTER_PLL_LOCK_STATUS;
    FWK_R   uint32_t  CLUSTER_PLL_UNLOCK_STATUS;
            uint8_t   RESERVED15[0xC00 - 0xBF8];
    FWK_R   uint32_t  CLUS_FAULT_INT_STATUS;
            uint8_t   RESERVED16[0xC30 - 0xC04];
    FWK_R   uint32_t  CLUSTER_ECCERR_INT_STATUS;
            uint8_t   RESERVED17[0xD00 - 0xC34];
    FWK_R   uint32_t  DMC0_4_INT_STATUS;
    FWK_R   uint32_t  DMC1_5_INT_STATUS;
    FWK_R   uint32_t  DMC2_6_INT_STATUS;
    FWK_R   uint32_t  DMC3_7_INT_STATUS;
            uint8_t   RESERVED18[0xFC0 - 0xD10];
    FWK_R   uint32_t  PCL_CFG;
            uint8_t   RESERVED19[0xFD0 - 0xFC4];
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

#endif  /* N1SDP_PIK_SCP_H */

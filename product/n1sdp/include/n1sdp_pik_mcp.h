/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     MCP PIK registers
 */

#ifndef N1SDP_PIK_MCP_H
#define N1SDP_PIK_MCP_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief MCP PIK register definitions
 */
struct pik_mcp_reg {
            uint8_t   RESERVED0[0x10 - 0x0];
    FWK_RW  uint32_t  RESET_SYNDROME;
            uint8_t   RESERVED1[0x20 - 0x14];
    FWK_RW  uint32_t  SURVIVAL_RESET_STATUS;
            uint8_t   RESERVED2[0x34 - 0x24];
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
            uint8_t   RESERVED7[0xA10 - 0x828];
    FWK_R   uint32_t  PLL_STATUS0;
            uint8_t   RESERVED8[0xFC0 - 0xA14];
    FWK_R   uint32_t  PWR_CTRL_CONFIG;
            uint8_t   RESERVED18[0xFD0 - 0xFC4];
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

#endif  /* N1SDP_PIK_MCP_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGM776_PIK_DEBUG_H
#define SGM776_PIK_DEBUG_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief Debug register definitions
 */
struct pik_debug_reg {
    FWK_R   uint32_t DBG_PWR_REQ_ST[4];
    FWK_R   uint32_t DBG_PWR_REQ_ACK[4];
    FWK_R   uint32_t DBG_RST_REQ_ST;
    FWK_R   uint32_t DBG_RST_ACK;
            uint8_t  RESERVED0[0x30 - 0x28];
    FWK_R   uint32_t SYS_PWR_REQ_ST[4];
    FWK_R   uint32_t SYS_PWR_REQ_ACK[4];
    FWK_R   uint32_t SYS_RST_REQ_ST;
            uint8_t  RESERVED1[0x60 - 0x54];
    FWK_RW  uint32_t DEBUG_CONFIG;
            uint8_t  RESERVED2[0x810 - 0x64];
    FWK_RW  uint32_t TRACECLK_CTRL;
    FWK_RW  uint32_t TRACECLK_DIV1;
            uint8_t  RESERVED3[0x820 - 0x818];
    FWK_RW  uint32_t PCLKDBG_CTRL;
    FWK_RW  uint32_t PCLKDBG_DIV1;
            uint8_t  RESERVED4[0x830 - 0x828];
    FWK_RW  uint32_t DBGCLK_CTRL;
    FWK_RW  uint32_t DBGCLK_DIV1;
            uint8_t  RESERVED5[0xA00 - 0x838];
    FWK_R   uint32_t CLKFORCE_STATUS;
    FWK_RW  uint32_t CLKFORCE_SET;
    FWK_RW  uint32_t CLKFORCE_CLR;
            uint8_t  RESERVED6[0xB00 - 0xA0C];
    FWK_R   uint32_t DBG_PWR_REQ_INT_ST[4];
    FWK_R   uint32_t DBG_RST_REQ_INT_ST;
            uint8_t  RESERVED7[0xB20 - 0xB14];
    FWK_R   uint32_t SYS_PWR_REQ_INT_ST[4];
    FWK_R   uint32_t SYS_RST_REQ_INT_ST;
            uint8_t  RESERVED8[0xFBC - 0xB34];
    FWK_R   uint32_t CAP;
    FWK_R   uint32_t PCL_CONFIG;
            uint8_t  RESERVED9[0xFD0 - 0xFC4];
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

#endif  /* SGM776_PIK_DEBUG_H */

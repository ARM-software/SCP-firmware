/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGM776_PIK_SYSTEM_H
#define SGM776_PIK_SYSTEM_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief System PIK register definitions
 */
struct pik_system_reg {
            uint8_t  RESERVED0[0x800];
    FWK_RW  uint32_t PPUCLK_CTRL;
    FWK_RW  uint32_t PPUCLK_DIV1;
            uint8_t  RESERVED1[0x810 - 0x808];
    FWK_RW  uint32_t NOCMEMCLK_CTRL;
    FWK_RW  uint32_t NOCMEMCLK_DIV1;
            uint8_t  RESERVED2[0x820 - 0x818];
    FWK_RW  uint32_t ACLKCCI_CTRL;
    FWK_RW  uint32_t ACLKCCI_DIV1;
            uint8_t  RESERVED3[0x840 - 0x828];
    FWK_RW  uint32_t TCUCLK_CTRL;
    FWK_RW  uint32_t TCUCLK_DIV1;
            uint8_t  RESERVED4[0x850 - 0x848];
    FWK_RW  uint32_t GICCLK_CTRL;
    FWK_RW  uint32_t GICCLK_DIV1;
            uint8_t  RESERVED5[0x860 - 0x858];
    FWK_RW  uint32_t PCLKSCP_CTRL;
    FWK_RW  uint32_t PCLKSCP_DIV1;
            uint8_t  RESERVED6[0x870 - 0x868];
    FWK_RW  uint32_t SYSPERCLK_CTRL;
    FWK_RW  uint32_t SYSPERCLK_DIV1;
            uint8_t  RESERVED7[0x880 - 0x878];
    FWK_RW  uint32_t FCMCLK_CTRL;
    FWK_RW  uint32_t FCMCLK_DIV1;
            uint8_t  RESERVED8[0xA00 - 0x888];
    FWK_R   uint32_t CLKFORCE_STATUS;
    FWK_RW  uint32_t CLKFORCE_SET;
    FWK_RW  uint32_t CLKFORCE_CLR;
            uint8_t  RESERVED9[0xFBC - 0xA0C];
    FWK_R   uint32_t CAP;
    FWK_R   uint32_t PCL_CONFIG;
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

#define CAP_GICCLK_GATING_SUPPORT  UINT32_C(0x00000001)
#define CAP_TCUCLK_SUPPORT         UINT32_C(0x00000002)
#define CAP_ELA_SUPPORT            UINT32_C(0x00000004)
#define CAP_FCMCLK_SUPPORT         UINT32_C(0x00000008)

#endif  /* SGM776_PIK_SYSTEM_H */

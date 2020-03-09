/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGI575_PIK_SYSTEM_H
#define SGI575_PIK_SYSTEM_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief TCU clock register definitions
 */
struct tcuclk_ctrl_reg {
    FWK_RW uint32_t   TCUCLK_CTRL;
    FWK_RW uint32_t   TCUCLK_DIV1;
};

/*!
 * \brief System PIK register definitions
 */
struct pik_system_reg {
            uint8_t         RESERVED0[0x800 - 0x0];
    FWK_RW  uint32_t        PPUCLK_CTRL;
    FWK_RW  uint32_t        PPUCLK_DIV1;
            uint8_t         RESERVED1[0x820 - 0x808];
    FWK_RW  uint32_t        INTCLK_CTRL;
    FWK_RW  uint32_t        INTCLK_DIV1;
            uint8_t         RESERVED2[0x830 - 0x828];
    struct tcuclk_ctrl_reg  TCUCLK[4];
    FWK_RW  uint32_t        GICCLK_CTRL;
    FWK_RW  uint32_t        GICCLK_DIV1;
            uint8_t         RESERVED3[0x860 - 0x858];
    FWK_RW  uint32_t        PCLKSCP_CTRL;
    FWK_RW  uint32_t        PCLKSCP_DIV1;
            uint8_t         RESERVED4[0x870 - 0x868];
    FWK_RW  uint32_t        SYSPERCLK_CTRL;
    FWK_RW  uint32_t        SYSPERCLK_DIV1;
            uint8_t         RESERVED5[0x880 - 0x878];
    FWK_RW  uint32_t        DMCCLK_CTRL;
    FWK_RW  uint32_t        DMCCLK_DIV1;
            uint8_t         RESERVED6[0x890 - 0x888];
    FWK_RW  uint32_t        SYSPCLKDBG_CTRL;
    FWK_RW  uint32_t        SYSPCLKDBG_DIV1;
            uint8_t         RESERVED7[0x8A0 - 0x898];
    FWK_RW  uint32_t        UARTCLK_CTRL;
    FWK_RW  uint32_t        UARTCLK_DIV1;
            uint8_t         RESERVED8[0xA00 - 0x8A8];
    FWK_R   uint32_t        CLKFORCE_STATUS;
    FWK_W   uint32_t        CLKFORCE_SET;
    FWK_W   uint32_t        CLKFORCE_CLR;
            uint8_t         RESERVED9[0xB0C - 0xA0C];
    FWK_RW  uint32_t        SYSTOP_RST_DLY;
            uint8_t         RESERVED10[0xFC0 - 0xB10];
    FWK_R   uint32_t        PCL_CONFIG;
            uint8_t         RESERVED11[0xFD0 - 0xFC4];
    FWK_R   uint32_t        PID4;
    FWK_R   uint32_t        PID5;
    FWK_R   uint32_t        PID6;
    FWK_R   uint32_t        PID7;
    FWK_R   uint32_t        PID0;
    FWK_R   uint32_t        PID1;
    FWK_R   uint32_t        PID2;
    FWK_R   uint32_t        PID3;
    FWK_R   uint32_t        ID0;
    FWK_R   uint32_t        ID1;
    FWK_R   uint32_t        ID2;
    FWK_R   uint32_t        ID3;
};

#endif  /* SGI575_PIK_SYSTEM_H */

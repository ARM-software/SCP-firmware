/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP System PIK registers
 */

#ifndef SYSTEM_PIK_H
#define SYSTEM_PIK_H

#include "scp_css_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief System PIK register definitions
 */

// clang-format off
struct system_pik_reg {
            uint8_t   RESERVED0[0x820 - 0x0];
    FWK_RW  uint32_t  INTCLK_CTRL;
    FWK_RW  uint32_t  INTCLK_DIV1;
            uint8_t   RESERVED1[0x850 - 0x828];
    FWK_RW  uint32_t  GICCLK_CTRL;
    FWK_RW  uint32_t  GICCLK_DIV1;
            uint8_t   RESERVED2[0x860 - 0x858];
    FWK_RW  uint32_t  SCPPIKCLK_CTRL;
    FWK_RW  uint32_t  SCPPIKCLK_DIV1;
            uint8_t   RESERVED3[0x870 - 0x868];
    FWK_RW  uint32_t  SYSPERCLK_CTRL;
    FWK_RW  uint32_t  SYSPERCLK_DIV1;
            uint8_t   RESERVED4[0x8A0 - 0x878];
    FWK_RW  uint32_t  APUARTCLK_CTRL;
    FWK_RW  uint32_t  APUARTCLK_DIV1;
            uint8_t   RESERVED5[0x8B0 - 0x8A8];
    FWK_RW  uint32_t  IONCICLK_CTRL;
    FWK_RW  uint32_t  IONCICLK_DIV1;
            uint8_t   RESERVED6[0x900 - 0x8B8];
    FWK_RW  uint32_t  TCUCLK_CTRL;
    FWK_RW  uint32_t  TCUCLK_DIV1;
            uint8_t   RESERVED7[0x940 - 0x908];
    FWK_RW  uint32_t  TCU_CLK_ENABLE;
    FWK_RW  uint32_t  NCI_CLK_ENABLE;
            uint8_t   RESERVED8[0xA00 - 0x948];
    FWK_R   uint32_t  CLKFORCE_STATUS;
    FWK_W   uint32_t  CLKFORCE_SET;
    FWK_W   uint32_t  CLKFORCE_CLR;
            uint8_t   RESERVED9[0xB10 - 0xA0C];
    FWK_RW  uint32_t  IOMACRO_OVERRIDE;
    FWK_RW  uint32_t  RSSPSI_STATUS;
    FWK_RW  uint32_t  RSSSAM_STATUS0;
    FWK_RW  uint32_t  RSSSAM_STATUS1;
    FWK_RW  uint32_t  RSSLCM_STATUS;
            uint8_t   RESERVED10[0xFC0 - 0xB24];
    FWK_R   uint32_t  PIK_CONFIG;
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

#define SYSTEM_PIK_PTR ((struct system_pik_reg *)SCP_SYSTEM_PIK_BASE)

#endif /* SYSTEM_PIK_H */

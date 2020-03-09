/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGI575_PIK_DEBUG_H
#define SGI575_PIK_DEBUG_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief Debug register definitions
 */
struct pik_debug_reg {
    FWK_RW  uint32_t DEBUG_CTRL;
    FWK_R   uint32_t DEBUG_STATUS;
    FWK_RW  uint32_t DEBUG_CONFIG;
            uint8_t  RESERVED0[0x10 - 0xC];
    FWK_R   uint32_t APP_DAP_TARGET_ID;
    FWK_R   uint32_t SCP_DAP_TARGET_ID;
    FWK_R   uint32_t DAP_INSTANCE_ID;
            uint8_t  RESERVED1[0x810 - 0x1C];
    FWK_RW  uint32_t TRACECLK_CTRL;
    FWK_RW  uint32_t TRACECLK_DIV1;
            uint8_t  RESERVED2[0x820 - 0x818];
    FWK_RW  uint32_t PCLKDBG_CTRL;
            uint8_t  RESERVED3[0x830 - 0x824];
    FWK_RW  uint32_t ATCLKDBG_CTRL;
    FWK_RW  uint32_t ATCLKDBG_DIV1;
    FWK_R   uint8_t  RESERVED4[0xFC0 - 0x838];
    FWK_R   uint32_t PCL_CONFIG;
            uint8_t  RESERVED5[0xFD0 - 0xFC4];
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

#endif  /* SGI575_PIK_DEBUG_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGM775_PIK_DEBUG_H
#define SGM775_PIK_DEBUG_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief Debug register definitions
 */
struct pik_debug_reg {
  FWK_RW  uint32_t DEBUG_CONTROL;
  FWK_R   uint32_t DEBUG_STATUS;
          uint32_t RESERVED0[2];
  FWK_R   uint32_t APP_DAP_TARGET_ID;
  FWK_R   uint32_t SCP_DAP_TARGET_ID;
  FWK_R   uint32_t DAP_INSTANCE_ID;
          uint8_t  RESERVED1[0x810 - 0x1C];
  FWK_RW  uint32_t TRACECLK_CTRL;
  FWK_RW  uint32_t TRACECLK_DIV1;
          uint32_t RESERVED2[2];
  FWK_RW  uint32_t PCLKDBG_CTRL;
          uint32_t RESERVED3[3];
  FWK_RW  uint32_t ATCLK_CTRL;
  FWK_RW  uint32_t ATCLK_DIV1;
  FWK_R   uint8_t  RESERVED4[0xFC0 - 0x838];
  FWK_R   uint32_t PIK_CONFIG;
          uint32_t RESERVED5[3];
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

#endif  /* SGM775_PIK_DEBUG_H */

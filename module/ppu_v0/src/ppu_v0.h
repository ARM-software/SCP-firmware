/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PPU_V0_H
#define PPU_V0_H

/*!
 * \cond
 * @{
 */

#include <fwk_macros.h>

#include <stdbool.h>
#include <stdint.h>

struct ppu_v0_reg {
  FWK_RW  uint32_t POWER_POLICY;
  FWK_R   uint32_t POWER_STATUS;
  FWK_R   uint32_t STATIC_CFG_STATUS;
  FWK_R   uint32_t DEV_IF_IP_CUR_STATUS;
  FWK_R   uint32_t MISC_IP_CUR_STATUS;
  FWK_R   uint32_t STORED_STATUS;
  FWK_W   uint32_t OFF_MEM_RET_UNLOCK;
          uint32_t RESERVED0;
  FWK_RW  uint32_t POWER_CFG;
          uint32_t RESERVED1[3];
  FWK_RW  uint32_t IMR;
  FWK_RW  uint32_t ISR;
  FWK_RW  uint32_t IESR;
          uint32_t RESERVED2[5];
  FWK_RW  uint32_t FUNC_RET_RAM_CFG;
  FWK_RW  uint32_t FULL_RET_RAM_CFG;
  FWK_RW  uint32_t MEM_RET_RAM_CFG;
          uint32_t RESERVED3;
  FWK_RW  uint32_t MODE_ENTRY_DELAY_TIME_0;
  FWK_RW  uint32_t MODE_ENTRY_DELAY_TIME_1;
          uint32_t RESERVED4[2];
  FWK_RW  uint32_t DEV_CONTROL_DELAY_CFG_0;
  FWK_RW  uint32_t DEV_CONTROL_DELAY_CFG_1;
          uint8_t  RESERVED5[0xFC8 - 0x78];
  FWK_R   uint32_t IMPLEMENTATION_ID;
  FWK_R   uint32_t ARCHITECTURE_ID;
  FWK_R   uint32_t PID4;
  FWK_R   uint32_t PID5;
  FWK_R   uint32_t PID6;
  FWK_R   uint32_t PID7;
  FWK_R   uint32_t PID0;
  FWK_R   uint32_t PID1;
  FWK_R   uint32_t PID2;
  FWK_R   uint32_t PID3;
  FWK_R   uint32_t CID0;
  FWK_R   uint32_t CID1;
  FWK_R   uint32_t CID2;
  FWK_R   uint32_t CID3;
};

enum ppu_v0_mode {
    PPU_V0_MODE_OFF        = 0,
    PPU_V0_MODE_MEM_RET    = 1,
    PPU_V0_MODE_LOGIC_RET  = 2,
    PPU_V0_MODE_FULL_RET   = 3,
    PPU_V0_MODE_MEM_OFF    = 4,
    PPU_V0_MODE_FUNC_RET   = 5,
    PPU_V0_MODE_ON         = 6,
    PPU_V0_MODE_WARM_RESET = 7,
    PPU_V0_MODE_COUNT,
};

/*
 * Bit definitions for PPR
 */
#define PPU_V0_PPR_POLICY       UINT32_C(0x00000007)
#define PPU_V0_PPR_DYNAMIC_EN   UINT32_C(0x00000100)
#define PPU_V0_PPR_EMULATED_EN  UINT32_C(0x00000200)
#define PPU_V0_PPR_OFF_LOCK_EN  UINT32_C(0x00001000)

/*
 * Bit definitions for PSR
 */
#define PPU_V0_PSR_EMULATED  UINT32_C(0x00000200)
#define PPU_V0_PSR_DYNAMIC   UINT32_C(0x00000100)
#define PPU_V0_PSR_POWSTAT   UINT32_C(0x00000007)

/*
 * Bit definitions for IMR
 */
#define PPU_V0_IMR_MASK            UINT32_C(0x010000FF)
#define PPU_V0_IMR_STA_POLICY_TRN  UINT32_C(0x00000001)
#define PPU_V0_IMR_STA_ACCEPT      UINT32_C(0x00000002)
#define PPU_V0_IMR_STA_DENY        UINT32_C(0x00000004)
#define PPU_V0_IMR_DYN_ACCEPT      UINT32_C(0x00000008)
#define PPU_V0_IMR_DYN_DENY        UINT32_C(0x00000010)
#define PPU_V0_IMR_EMU_ACCEPT      UINT32_C(0x00000020)
#define PPU_V0_IMR_EMU_DENY        UINT32_C(0x00000040)
#define PPU_V0_IMR_UNSPT_POLICY    UINT32_C(0x00000080)
#define PPU_V0_IMR_DYN_POLICY_MIN  UINT32_C(0x01000000)

/*
 * Bit definitions for ISR
 */
#define PPU_V0_ISR_MASK                 UINT32_C(0x01FF01FF)
#define PPU_V0_ISR_STA_POLICY_TRN       UINT32_C(0x00000001)
#define PPU_V0_ISR_STA_ACCEPT           UINT32_C(0x00000002)
#define PPU_V0_ISR_STA_DENY             UINT32_C(0x00000004)
#define PPU_V0_ISR_DYN_ACCEPT           UINT32_C(0x00000008)
#define PPU_V0_ISR_DYN_DENY             UINT32_C(0x00000010)
#define PPU_V0_ISR_EMU_ACCEPT           UINT32_C(0x00000020)
#define PPU_V0_ISR_EMU_DENY             UINT32_C(0x00000040)
#define PPU_V0_ISR_UNSPT_POLICY         UINT32_C(0x00000080)
#define PPU_V0_ISR_DBGEMUPWRDWN_EDGE    UINT32_C(0x00000100)
#define PPU_V0_ISR_ACTIVE_EDGE          UINT32_C(0x00FF0000)
#define PPU_V0_ISR_ACTIVE_EDGE_ACTIVE0  UINT32_C(0x00010000)
#define PPU_V0_ISR_ACTIVE_EDGE_ACTIVE1  UINT32_C(0x00020000)
#define PPU_V0_ISR_ACTIVE_EDGE_ACTIVE2  UINT32_C(0x00040000)
#define PPU_V0_ISR_ACTIVE_EDGE_ACTIVE3  UINT32_C(0x00080000)
#define PPU_V0_ISR_ACTIVE_EDGE_ACTIVE4  UINT32_C(0x00100000)
#define PPU_V0_ISR_ACTIVE_EDGE_ACTIVE5  UINT32_C(0x00200000)
#define PPU_V0_ISR_ACTIVE_EDGE_ACTIVE6  UINT32_C(0x00400000)
#define PPU_V0_ISR_ACTIVE_EDGE_ACTIVE7  UINT32_C(0x00800000)
#define PPU_V0_ISR_DYN_POLICY_MIN       UINT32_C(0x01000000)

/*
 * Bit definitions for ARCHITECTURE_ID
 */
#define PPU_V0_ARCHITECTURE_ID UINT32_C(0x00000000)

/*
 * Interface
 */
void ppu_v0_init(struct ppu_v0_reg *ppu);
int ppu_v0_request_power_mode(struct ppu_v0_reg *ppu, enum ppu_v0_mode mode);
int ppu_v0_set_power_mode(struct ppu_v0_reg *ppu, enum ppu_v0_mode mode);
int ppu_v0_get_power_mode(struct ppu_v0_reg *ppu, enum ppu_v0_mode *mode);

/*!
 * \endcond
 * @}
 */

#endif  /* PPU_V0_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP PIK registers
 */

#ifndef SCP_PIK_H
#define SCP_PIK_H

#include "scp_css_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

// clang-format off
/*!
 * \brief SCP PIK register definitions
 */
struct pik_scp_reg {
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
            uint8_t   RESERVED5[0x810 - 0x204];
    FWK_RW  uint32_t  CORECLK_CTRL;
    FWK_RW  uint32_t  CORECLK_DIV1;
            uint8_t   RESERVED6[0x820 - 0x818];
    FWK_RW  uint32_t  ACLK_CTRL;
    FWK_RW  uint32_t  ACLK_DIV1;
            uint8_t   RESERVED7[0x850 - 0x828];
    FWK_RW  uint32_t  REFCLK_CTRL;
            uint8_t   RESERVED8[0xA60 - 0x854];
    FWK_R   uint32_t  CONS_MMUTCU_INT_STATUS;
    FWK_R   uint32_t  CONS_MMUTCU_INT_CLR;
    FWK_R   uint32_t  CONS_MMUTBU_INT_STATUS0;
    FWK_R   uint32_t  CONS_MMUTBU_INT_CLR0;
    FWK_R   uint32_t  CONS_MMUTBU_INT_STATUS1;
    FWK_R   uint32_t  CONS_MMUTBU_INT_CLR1;
    FWK_RW  uint32_t  CONS_MMUTBU_INT_STATUS2;
    FWK_RW  uint32_t  CONS_MMUTBU_INT_CLR2;
    FWK_RW  uint32_t  CONS_MMUTBU_INT_STATUS3;
    FWK_RW  uint32_t  CONS_MMUTBU_INT_CLR3;
    FWK_RW  uint32_t  CONS_MMUTBU_INT_STATUS4;
    FWK_RW  uint32_t  CONS_MMUTBU_INT_CLR4;
    FWK_RW  uint32_t  CONS_MMUTBU_INT_STATUS5;
    FWK_RW  uint32_t  CONS_MMUTBU_INT_CLR5;
            uint8_t   RESERVED9[0xB20 - 0xA98];
    FWK_R   uint32_t  CPU_PPU_INT_STATUS[4];
            uint8_t   RESERVED10[0xB40 - 0xB30];
    FWK_R   uint32_t  CLUS_PPU_INT_STATUS[4];
            uint8_t   RESERVED11[0xB80 - 0xB50];
    FWK_R   uint32_t  CPU_PLL_LOCK_STATUS[4];
            uint8_t   RESERVED12[0xBC0 - 0xB90];
    FWK_R   uint32_t  CPU_PLL_UNLOCK_STATUS[4];
            uint8_t   RESERVED13[0xC00 - 0xBD0];
    FWK_R   uint32_t  CONS_CLUS_SCF_INT_STATUSx[4];
            uint8_t   RESERVED14[0xD00 - 0xC10];
    FWK_RW  uint32_t  TCMECC_ERRSTATUS;
    FWK_RW  uint32_t  TCMECC_ERRCTRL;
    FWK_RW  uint32_t  TCMECC_ERRCODE;
    FWK_RW  uint32_t  TCMECC_ERRADDR;
            uint8_t   RESERVED15[0xFC0-0xD10];
    FWK_R   uint32_t  PWR_CTRL_CONFIG;
            uint8_t   RESERVED16[0xFD0 - 0xFC4];
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

#define PLL_STATUS_0_REFCLK     UINT32_C(0x00000001)
#define PLL_STATUS_0_SYSPLLLOCK UINT32_C(0x00000002)
#define PLL_STATUS_0_DDRPLLLOCK UINT32_C(0x00000004)
#define PLL_STATUS_0_INTPLLLOCK UINT32_C(0x00000008)

#define PLL_STATUS_CPUPLLLOCK(CPU) ((uint32_t)(1 << (CPU % 32)))

/* Pointer to SCP PIK */
#define SCP_PIK_PTR ((struct pik_scp_reg *)SCP_PIK_SCP_BASE)
// clang-format on

#endif /* SCP_PIK_H */

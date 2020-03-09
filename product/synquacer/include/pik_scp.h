/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PIK_SCP_H
#define PIK_SCP_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief SCP PIK register definitions
 */
typedef struct {
    FWK_R uint32_t RESERVED0[4];
    FWK_RW uint32_t RESET_SYNDROME;
    FWK_R uint32_t RESERVED1[3];
    FWK_RW uint32_t SURVIVAL;
    FWK_R uint32_t RESERVED2[4];
    FWK_RW uint32_t ADDR_TRANS;
    FWK_R uint32_t RESERVED3[2];
    FWK_RW uint32_t WS1_TIMER_MATCH;
    FWK_RW uint32_t WS1_TIMER_EN;
    FWK_R uint32_t RESERVED4[110];
    FWK_R uint32_t SS_RESET_STATUS;
    FWK_W uint32_t SS_RESET_SET;
    FWK_W uint32_t SS_RESET_CLR;
    FWK_R uint32_t RESERVED5[385];
    FWK_RW uint32_t CORECLK_CNTRL;
    FWK_RW uint32_t CORECLK_DIV1;
    FWK_R uint32_t RESERVED6[2];
    FWK_RW uint32_t ACLK_CNTRL;
    FWK_RW uint32_t ACLK_DIV1;
    FWK_R uint32_t RESERVED7[118];
    FWK_R uint32_t CLKFORCE_STATUS;
    FWK_R uint32_t CLKFORCE_SET;
    FWK_R uint32_t CLKFORCE_CLEAR;
    FWK_R uint32_t RESERVED8[1];
    FWK_R uint32_t PLL_STATUS[17];
    FWK_R uint32_t RESERVED9[43];
    FWK_R uint32_t MHU_NS_INT_STATUS;
    FWK_R uint32_t MHU_S_INT_STATUS;
    FWK_R uint32_t RESERVED10[6];
    FWK_R uint32_t CPU_PPU_INT_STATUS[8];
    FWK_R uint32_t CLUS_INT_STATUS;
    FWK_R uint32_t RESERVED11[7];
    FWK_R uint32_t TIMER_INT_STATUS[8];
    FWK_R uint32_t CPU_PLLA_LOCK_STATUS[8];
    FWK_R uint32_t CPU_PLLB_LOCK_STATUS[8];
    FWK_R uint32_t CPU_PLLA_UNLOCK_STATUS[8];
    FWK_R uint32_t CPU_PLLB_UNLOCK_STATUS[8];
    FWK_R uint32_t RESERVED12[240];
    FWK_R uint32_t PWR_CTRL_CFG;
    FWK_R uint32_t RESERVED13[3];
    FWK_R uint32_t PID4;
    FWK_R uint32_t PID5;
    FWK_R uint32_t PID6;
    FWK_R uint32_t PID7;
    FWK_R uint32_t PID0;
    FWK_R uint32_t PID1;
    FWK_R uint32_t PID2;
    FWK_R uint32_t PID3;
    FWK_R uint32_t ID0;
    FWK_R uint32_t ID1;
    FWK_R uint32_t ID2;
    FWK_R uint32_t ID3;
} pik_scp_reg_t;

#define PLL_STATUS_0_REFCLK UINT32_C(0x00000001)
#define PLL_STATUS_0_SYSPLLLOCK UINT32_C(0x00000002)

#endif /* PIK_SCP_H */

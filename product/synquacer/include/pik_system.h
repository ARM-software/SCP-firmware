/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PIK_SYSTEM_H
#define PIK_SYSTEM_H

#include <fwk_macros.h>

#include <stdint.h>

typedef struct {
    FWK_R uint32_t RESERVED0[512];
    FWK_RW uint32_t PPUCLK_CTRL;
    FWK_RW uint32_t PPUCLK_DIV1;
    FWK_R uint32_t RESERVED1[6];
    FWK_RW uint32_t CCNCLK_CTRL;
    FWK_RW uint32_t CCNCLK_DIV1;
    FWK_R uint32_t RESERVED2[10];
    FWK_RW uint32_t GICCLK_CTRL;
    FWK_RW uint32_t GICCLK_DIV1;
    FWK_R uint32_t RESERVED3[2];
    FWK_RW uint32_t PCLKSCP_CTRL;
    FWK_RW uint32_t PCLKSCP_DIV1;
    FWK_R uint32_t RESERVED4[2];
    FWK_RW uint32_t SYSPERCLK_CTRL;
    FWK_RW uint32_t SYSPERCLK_DIV1;
    FWK_R uint32_t RESERVED5[2];
    FWK_RW uint32_t DMCCLK_CTRL;
    FWK_RW uint32_t DMCCLK_DIV1;
    FWK_R uint32_t RESERVED6[2];
    FWK_RW uint32_t SYSPCLKDBG_CTRL;
    FWK_RW uint32_t SYSPCLKDBG_DIV1;
    FWK_R uint32_t RESERVED7[2];
    FWK_RW uint32_t UARTCLK_CTRL;
    FWK_RW uint32_t UARTCLK_DIV1;
    FWK_R uint32_t RESERVED8[86];
    FWK_R uint32_t CLKFORCE_STATUS;
    FWK_RW uint32_t CLKFORCE_SET;
    FWK_RW uint32_t CLKFORCE_CLR;
    FWK_R uint32_t RESERVED9[63];
    FWK_RW uint32_t DMC_CONTROL;
    FWK_RW uint32_t SYSTOP_RST_DLY;
    FWK_R uint32_t RESERVED10[300];
    FWK_R uint32_t PIK_CONFIG;
    FWK_R uint32_t RESERVED11[3];
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
} pik_system_reg_t;

#endif /* PIK_SYSTEM_H */

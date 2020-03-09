/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef PIK_CPU_H
#define PIK_CPU_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief CPU PIK register definitions
 */
typedef struct {
    FWK_RW uint32_t STATIC_CONFIG;
    FWK_R uint32_t RESERVED0[3];
    FWK_RW uint32_t RVBARADDR0_LW;
    FWK_RW uint32_t RVBARADDR0_UP;
    FWK_RW uint32_t RVBARADDR1_LW;
    FWK_RW uint32_t RVBARADDR1_UP;
    FWK_RW uint32_t RVBARADDR2_LW;
    FWK_RW uint32_t RVBARADDR2_UP;
    FWK_RW uint32_t RVBARADDR3_LW;
    FWK_RW uint32_t RVBARADDR3_UP;
    FWK_RW uint32_t CLUSTER_CONFIG;
    FWK_R uint8_t RESERVED1[0x200 - 0x34];
    FWK_R uint32_t DBG_RST_STATUS;
    FWK_RW uint32_t DBG_RST_SET;
    FWK_RW uint32_t DBG_RST_CLR;
    FWK_R uint8_t RESERVED2[0x400 - 0x20c];
    FWK_RW uint32_t CPUACTIVE_CTRL;
    FWK_R uint8_t RESERVED3[0x800 - 0x404];
    FWK_RW uint32_t PPUCLK_CTRL;
    FWK_RW uint32_t PPUCLK_DIV1;
    FWK_RW uint32_t PPUCLK_DIV2;
    FWK_R uint32_t RESERVED4;
    FWK_RW uint32_t CPUCLK_CTRL;
    FWK_RW uint32_t CPUCLK_DIV1;
    FWK_RW uint32_t CPUCLK_DIV2;
    FWK_R uint32_t RESERVED5;
    FWK_RW uint32_t PCLKDBG_CTRL;
    FWK_R uint32_t RESERVED6[3];
    FWK_RW uint32_t ATCLKDBG_CTRL;
    FWK_R uint32_t RESERVED7[3];
    FWK_RW uint32_t ACLKCPU_CTRL;
    FWK_R uint8_t RESERVED8[0xA00 - 0x844];
    FWK_R uint32_t CLKFORCE_STATUS;
    FWK_RW uint32_t CLKFORCE_SET;
    FWK_RW uint32_t CLKFORCE_CLR;
    FWK_R uint8_t RESERVED9[0xFC0 - 0xA0C];
    FWK_R uint32_t CONFIG;
    FWK_R uint32_t RESERVED10[3];
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
} pik_cpu_reg_t;

#endif /* PIK_CPU_H */

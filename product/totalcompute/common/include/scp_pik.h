/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP PIK registers
 */

#ifndef SCP_PIK_H
#define SCP_PIK_H

#include "scp_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

struct pik_scp_reg {
    uint32_t RESERVED0[4];
    FWK_RW uint32_t RESET_SYNDROME;
    FWK_RW uint32_t WIC_CTRL;
    FWK_R uint32_t WIC_STATUS;
    uint8_t RESERVED1[0xA00 - 0x1C];
    FWK_R uint32_t CLKFORCE_STATUS;
    FWK_RW uint32_t CLKFORCE_SET;
    FWK_RW uint32_t CLKFORCE_CLR;
    uint32_t RESERVED2;
    FWK_R uint32_t PLL_STATUS[2];
    uint8_t RESERVED3[0xFC0 - 0xA18];
    FWK_R uint32_t PIK_CONFIG;
    uint32_t RESERVED4[3];
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
};

#define PLL_STATUS_0_SYSPLL_LOCK     UINT32_C(1 << 5)
#define PLL_STATUS_0_DISPLAYPLL_LOCK UINT32_C(1 << 6)

#define PLL_STATUS_CPUPLL_LOCK(CPU) ((uint32_t)(1 << (CPU % 32)))

/* Pointer to SCP PIK */
#define SCP_PIK_PTR ((struct pik_scp_reg *)SCP_PIK_SCP_BASE)

#endif /* SCP_PIK_H */

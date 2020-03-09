/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGI575_SSC_H
#define SGI575_SSC_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief System Security Control (SSC) register definitions
 */
struct ssc_reg {
           uint8_t  RESERVED1[0x10 - 0x0];
    FWK_R  uint32_t SSC_DBGCFG_STAT;
    FWK_W  uint32_t SSC_DBGCFG_SET;
    FWK_W  uint32_t SSC_DBGCFG_CLR;
           uint8_t  RESERVED2[0x28 - 0x1C];
    FWK_RW uint32_t SSC_AUXDBGCFG;
           uint8_t  RESERVED3[0x30 - 0x2C];
    FWK_RW uint32_t SSC_GPRETN;
           uint8_t  RESERVED4[0x40 - 0x34];
    FWK_R  uint32_t SSC_VERSION;
           uint8_t  RESERVED5[0xFD0 - 0x44];
    FWK_R  uint32_t PID4;
           uint8_t  RESERVED6[0xFE0 - 0xFD4];
    FWK_R  uint32_t PID0;
    FWK_R  uint32_t PID1;
    FWK_R  uint32_t PID2;
    FWK_R  uint32_t PID3;
    FWK_R  uint32_t COMPID0;
    FWK_R  uint32_t COMPID1;
    FWK_R  uint32_t COMPID2;
    FWK_R  uint32_t COMPID3;
};

#endif /* SGI575_SSC_H */

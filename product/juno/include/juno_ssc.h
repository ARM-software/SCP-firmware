/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SoC-level System Security Control (SSC) register definitions.
 */

#ifndef JUNO_SSC_H
#define JUNO_SSC_H

#include "system_mmap.h"

#include <fwk_macros.h>

#include <stdint.h>

struct ssc_reg {
           uint8_t   RESERVED1[0x10 - 0];
    FWK_R  uint32_t  SSC_DBGCFG_STAT;
    FWK_W  uint32_t  SSC_DBGCFG_SET;
    FWK_W  uint32_t  SSC_DBGCFG_CLR;
           uint8_t   RESERVED2[0x24 - 0x1C];
    FWK_RW uint32_t  SSC_SWDHOD;
    FWK_RW uint32_t  SSC_AUXDBGCFG;
           uint8_t   RESERVED3[0x30 - 0x2C];
    FWK_RW uint32_t  SSC_GPRETN;
           uint8_t   RESERVED4[0x40 - 0x34];
    FWK_R  uint32_t  SSC_VERSION;
           uint8_t   RESERVED5[0xFD0 - 0x44];
    FWK_R  uint32_t  PID4;
           uint8_t   RESERVED6[0xFE0 - 0xFD4];
    FWK_R  uint32_t  PID0;
    FWK_R  uint32_t  PID1;
    FWK_R  uint32_t  PID2;
    FWK_R  uint32_t  PID3;
    FWK_R  uint32_t  COMPID0;
    FWK_R  uint32_t  COMPID1;
    FWK_R  uint32_t  COMPID2;
    FWK_R  uint32_t  COMPID3;
};

#define SSC ((struct ssc_reg *) SSC_BASE)

#define JUNO_SSC_SWDHOD_OFF                 UINT32_C(0x00000000)
#define JUNO_SSC_SWDHOD_AP                  UINT32_C(0x00000001)
#define JUNO_SSC_SWDHOD_SCP                 UINT32_C(0x00000002)
#define JUNO_SSC_SWDHOD_ALL                 UINT32_C(0x00000003)

#define JUNO_SSC_VERSION_CONFIGURATION      UINT32_C(0xF0000000)
#define JUNO_SSC_VERSION_CONFIGURATION_POS  28
#define JUNO_SSC_VERSION_DESIGNER_ID        UINT32_C(0x000FF000)
#define JUNO_SSC_VERSION_PART_NUMBER        UINT32_C(0x00000FFF)

#define JUNO_SSC_GPRETN_VALID_BITS          UINT32_C(0x0000FFFF)

#endif /* JUNO_SSC_H */

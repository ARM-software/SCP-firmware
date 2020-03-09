/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGM775_SSC_H
#define SGM775_SSC_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief System Security Control (SSC) register definitions
 */
struct ssc_reg {
    FWK_R  uint32_t RESERVED1[4];
    FWK_R  uint32_t SSC_DBGCFG_STAT;
    FWK_W  uint32_t SSC_DBGCFG_SET;
    FWK_W  uint32_t SSC_DBGCFG_CLR;
    FWK_R  uint32_t RESERVED2[2];
    FWK_RW uint32_t SSC_SWDHOD;
    FWK_RW uint32_t SSC_AUXDBGCFG;
    FWK_R  uint32_t RESERVED3;
    FWK_RW uint32_t SSC_GPRETN;
    FWK_R  uint32_t RESERVED4[3];
    FWK_R  uint32_t SSC_VERSION;
    FWK_R  uint32_t RESERVED5[995];
    FWK_R  uint32_t PID4;
    FWK_R  uint32_t RESERVED6[3];
    FWK_R  uint32_t PID0;
    FWK_R  uint32_t PID1;
    FWK_R  uint32_t PID2;
    FWK_R  uint32_t PID3;
    FWK_R  uint32_t COMPID0;
    FWK_R  uint32_t COMPID1;
    FWK_R  uint32_t COMPID2;
    FWK_R  uint32_t COMPID3;
};

enum ssc_part {
    SSC_PART_SGM_775 = 0x790,
};

#endif /* SGM775_SSC_H */

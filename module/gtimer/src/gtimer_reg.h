/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GTIMER_REG_H
#define GTIMER_REG_H

#include <fwk_macros.h>

#include <stdint.h>

/*!
 * \brief Counter registers (CNTCONTROL)
 */
struct cntcontrol_reg {
    FWK_RW  uint32_t CR;
    FWK_R   uint32_t SR;
    FWK_RW  uint32_t CVL;
    FWK_RW  uint32_t CVH;
            uint32_t RESERVED0[4];
    FWK_RW  uint32_t FID0;
            uint8_t  RESERVED1[0xC0 - 0x24];
    FWK_RW  uint32_t SCR; /* CSS: Synchronization Control Register */
    FWK_R   uint32_t SVL; /* CSS: Synchronized Counter Lower Value Register */
    FWK_R   uint32_t SVU; /* CSS: Synchronized Counter Upper Value Register */
            uint8_t  RESERVED2[0xFD0 - 0xCC];
    FWK_R   uint32_t PID[11];
};

#define CNTCONTROL_CR_EN              UINT32_C(0x00000001)
#define CNTCONTROL_CR_HDBG            UINT32_C(0x00000002)
#define CNTCONTROL_CR_FCREQ           UINT32_C(0x00000100)

#define CNTCONTROL_SCR_ENSYNC         UINT32_C(0x00000001)
#define CNTCONTROL_SCR_ENSYNC_DIRECT  UINT32_C(0x00000000)
#define CNTCONTROL_SCR_ENSYNC_DELAY   UINT32_C(0x00000001)

/*!
 * \brief Counter registers (CNTCTL)
 */
struct cntctl_reg {
    FWK_RW  uint32_t FRQ;
    FWK_RW  uint32_t NSAR;
    FWK_R   uint32_t TTIDR;
            uint8_t  RESERVED0[0x40 - 0x0C];
    FWK_RW  uint32_t ACR;
            uint8_t  RESERVED1[0xFD0 - 0x44];
    FWK_R   uint32_t PID[11];
};

#define CNTCTL_ACR        UINT32_C(0x0000003f)
#define CNTCTL_ACR_RPCT   UINT32_C(0x00000001)
#define CNTCTL_ACR_RVCT   UINT32_C(0x00000002)
#define CNTCTL_ACR_RFRQ   UINT32_C(0x00000004)
#define CNTCTL_ACR_RVOFF  UINT32_C(0x00000008)
#define CNTCTL_ACR_RWVT   UINT32_C(0x00000010)
#define CNTCTL_ACR_RWPT   UINT32_C(0x00000020)

/*!
 * \brief Counter registers (CNTBASE)
 */
struct cntbase_reg {
    FWK_R   uint32_t PCTL;
    FWK_R   uint32_t PCTH;
    FWK_R   uint32_t VCTL;
    FWK_R   uint32_t VCTH;
    FWK_R   uint32_t FRQ;
    FWK_RW  uint32_t PL0ACR;
    FWK_R   uint32_t OFFL;
    FWK_R   uint32_t OFFH;
    FWK_RW  uint32_t P_CVALL;
    FWK_RW  uint32_t P_CVALH;
    FWK_RW  uint32_t P_TVAL;
    FWK_RW  uint32_t P_CTL;
    FWK_RW  uint32_t V_CVALL;
    FWK_RW  uint32_t V_CVALH;
    FWK_RW  uint32_t V_TVAL;
    FWK_RW  uint32_t V_CTL;
            uint8_t  RESERVED0[0xFD0 - 0x40];
    FWK_R   uint32_t PID[11];
};

#define CNTBASE_P_CTL          UINT32_C(0x00000007)
#define CNTBASE_P_CTL_ENABLE   UINT32_C(0x00000001)
#define CNTBASE_P_CTL_IMASK    UINT32_C(0x00000002)
#define CNTBASE_P_CTL_ISTATUS  UINT32_C(0x00000004)

#endif /* GTIMER_REG_H */

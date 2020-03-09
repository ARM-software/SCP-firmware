/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      PL011 (UART) register definitions
 */

#ifndef PL011_H
#define PL011_H

#include <fwk_macros.h>

#include <stdint.h>

struct pl011_reg {
    FWK_RW  uint16_t DR;
            uint16_t RESERVED0;
    union {
        FWK_RW  uint8_t     RSR;
        FWK_RW  uint8_t     ECR;
    };
            uint8_t  RESERVED1[0x18 - 0x05];
    FWK_R   uint16_t FR;
            uint16_t RESERVED2[3];
    FWK_RW  uint8_t  ILPR;
            uint8_t  RESERVED3[3];
    FWK_RW  uint16_t IBRD;
            uint16_t RESERVED4;
    FWK_RW  uint32_t FBRD;
    FWK_RW  uint16_t LCR_H;
            uint16_t RESERVED5;
    FWK_RW  uint16_t CR;
            uint16_t RESERVED6;
    FWK_RW  uint16_t IFLS;
            uint16_t RESERVED7;
    FWK_RW  uint16_t IMSC;
            uint16_t RESERVED8;
    FWK_R   uint16_t RIS;
            uint16_t RESERVED9;
    FWK_R   uint16_t MIS;
            uint16_t RESERVED10;
    FWK_W   uint16_t ICR;
            uint16_t RESERVED11;
    FWK_RW  uint16_t DMACR;
            uint8_t  RESERVED12[0xFE0 - 0x4C];
    FWK_R   uint32_t PID0;
    FWK_R   uint32_t PID1;
    FWK_R   uint32_t PID2;
    FWK_R   uint32_t PID3;
    FWK_R   uint32_t CID0;
    FWK_R   uint32_t CID1;
    FWK_R   uint32_t CID2;
    FWK_R   uint32_t CID3;
};

#define PL011_DR_DATA                UINT16_C(0x00FF)
#define PL011_DR_FE                  UINT16_C(0x0100)
#define PL011_DR_PE                  UINT16_C(0x0200)
#define PL011_DR_BE                  UINT16_C(0x0400)
#define PL011_DR_OE                  UINT16_C(0x0800)

#define PL011_RSR_FE                 UINT8_C(0x01)
#define PL011_RSR_PE                 UINT8_C(0x02)
#define PL011_RSR_BE                 UINT8_C(0x04)
#define PL011_RSR_OE                 UINT8_C(0x08)
#define PL011_ECR_CLR                UINT8_C(0xFF)

#define PL011_FR_CTS                 UINT16_C(0x0001)
#define PL011_FR_DSR                 UINT16_C(0x0002)
#define PL011_FR_DCD                 UINT16_C(0x0004)
#define PL011_FR_BUSY                UINT16_C(0x0008)
#define PL011_FR_RXFE                UINT16_C(0x0010)
#define PL011_FR_TXFF                UINT16_C(0x0020)
#define PL011_FR_RXFF                UINT16_C(0x0040)
#define PL011_FR_TXFE                UINT16_C(0x0080)
#define PL011_FR_RI                  UINT16_C(0x0100)

#define PL011_LCR_H_BRK              UINT16_C(0x0001)
#define PL011_LCR_H_PEN              UINT16_C(0x0002)
#define PL011_LCR_H_EPS              UINT16_C(0x0004)
#define PL011_LCR_H_STP2             UINT16_C(0x0008)
#define PL011_LCR_H_FEN              UINT16_C(0x0010)
#define PL011_LCR_H_WLEN             UINT16_C(0x0060)
#define PL011_LCR_H_WLEN_5BITS       UINT16_C(0x0000)
#define PL011_LCR_H_WLEN_6BITS       UINT16_C(0x0020)
#define PL011_LCR_H_WLEN_7BITS       UINT16_C(0x0040)
#define PL011_LCR_H_WLEN_8BITS       UINT16_C(0x0060)
#define PL011_LCR_H_SPS              UINT16_C(0x0080)

#define PL011_CR_UARTEN              UINT16_C(0x0001)
#define PL011_CR_SIREN               UINT16_C(0x0002)
#define PL011_CR_SIRLP               UINT16_C(0x0004)
#define PL011_CR_LBE                 UINT16_C(0x0080)
#define PL011_CR_TXE                 UINT16_C(0x0100)
#define PL011_CR_RXE                 UINT16_C(0x0200)
#define PL011_CR_DTR                 UINT16_C(0x0400)
#define PL011_CR_RTS                 UINT16_C(0x0800)
#define PL011_CR_OUT1                UINT16_C(0x1000)
#define PL011_CR_OUT2                UINT16_C(0x2000)
#define PL011_CR_RTSEN               UINT16_C(0x4000)
#define PL011_CR_CTSEN               UINT16_C(0x8000)

#define PL011_IFLS_TXIFLSEL          UINT16_C(0x0007)
#define PL011_IFLS_RXIFLSEL          UINT16_C(0x0038)

#define PL011_IMSC_RIMIM             UINT16_C(0x0001)
#define PL011_IMSC_CTSMIM            UINT16_C(0x0002)
#define PL011_IMSC_DCDMIM            UINT16_C(0x0004)
#define PL011_IMSC_DSRMIM            UINT16_C(0x0008)
#define PL011_IMSC_RXIM              UINT16_C(0x0010)
#define PL011_IMSC_TXIM              UINT16_C(0x0020)
#define PL011_IMSC_RTIM              UINT16_C(0x0040)
#define PL011_IMSC_FEIM              UINT16_C(0x0080)
#define PL011_IMSC_PEIM              UINT16_C(0x0100)
#define PL011_IMSC_BEIM              UINT16_C(0x0200)
#define PL011_IMSC_OEIM              UINT16_C(0x0400)

#define PL011_RIS_RIRMIS             UINT16_C(0x0001)
#define PL011_RIS_CTSRMIS            UINT16_C(0x0002)
#define PL011_RIS_DCDRMIS            UINT16_C(0x0004)
#define PL011_RIS_DSRRMIS            UINT16_C(0x0008)
#define PL011_RIS_RXRIS              UINT16_C(0x0010)
#define PL011_RIS_TXRIS              UINT16_C(0x0020)
#define PL011_RIS_RTRIS              UINT16_C(0x0040)
#define PL011_RIS_FERIS              UINT16_C(0x0080)
#define PL011_RIS_PERIS              UINT16_C(0x0100)
#define PL011_RIS_BERIS              UINT16_C(0x0200)
#define PL011_RIS_OERIS              UINT16_C(0x0400)

#define PL011_MIS_RIMMIS             UINT16_C(0x0001)
#define PL011_MIS_CTSMMIS            UINT16_C(0x0002)
#define PL011_MIS_DCDMMIS            UINT16_C(0x0004)
#define PL011_MIS_DSRMMIS            UINT16_C(0x0008)
#define PL011_MIS_RXMIS              UINT16_C(0x0010)
#define PL011_MIS_TXMIS              UINT16_C(0x0020)
#define PL011_MIS_RTMIS              UINT16_C(0x0040)
#define PL011_MIS_FEMIS              UINT16_C(0x0080)
#define PL011_MIS_PEMIS              UINT16_C(0x0100)
#define PL011_MIS_BEMIS              UINT16_C(0x0200)
#define PL011_MIS_OEMIS              UINT16_C(0x0400)

#define PL011_ICR_RIMIC              UINT16_C(0x0001)
#define PL011_ICR_CTSMIC             UINT16_C(0x0002)
#define PL011_ICR_DCDMIC             UINT16_C(0x0004)
#define PL011_ICR_DSRMIC             UINT16_C(0x0008)
#define PL011_ICR_RXIC               UINT16_C(0x0010)
#define PL011_ICR_TXIC               UINT16_C(0x0020)
#define PL011_ICR_RTIC               UINT16_C(0x0040)
#define PL011_ICR_FEIC               UINT16_C(0x0080)
#define PL011_ICR_PEIC               UINT16_C(0x0100)
#define PL011_ICR_BEIC               UINT16_C(0x0200)
#define PL011_ICR_OEIC               UINT16_C(0x0400)

#define PL011_DMACR_RXDMAE           UINT16_C(0x0001)
#define PL011_DMACR_TXDMAE           UINT16_C(0x0002)
#define PL011_DMACR_DMAAONERR        UINT16_C(0x0004)

#define PL011_UARTCLK_MIN (1420 * FWK_KHZ)
#define PL011_UARTCLK_MAX (542720 * FWK_KHZ)

#endif /* PL011 */

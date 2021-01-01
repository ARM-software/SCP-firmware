/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SCIF_H
#define SCIF_H

#include <rcar_mmap.h>

#include <fwk_macros.h>

#define SCIF_INTERNAL_CLK 0 /* Internal clock(S3D4:66.66MHz) */
#define SCIF_EXTARNAL_CLK 1 /* External clock(SCK2:14.7456MHz) */
#define SCIF_CLK SCIF_INTERNAL_CLK

struct scif_reg {
    FWK_RW uint16_t SCSMR; /* H'00 */
    uint16_t RESERVED1; /* H'02 */
    FWK_RW uint8_t SCBRR; /* H'04 */
    uint8_t RESERVED2; /* H'05 */
    uint16_t RESERVED3; /* H'06 */
    FWK_RW uint16_t SCSCR; /* H'08 */
    uint16_t RESERVED4; /* H'0A */
    FWK_W uint8_t SCFTDR; /* H'0C */
    uint8_t RESERVED5; /* H'0D */
    uint16_t RESERVED6; /* H'0E */
    FWK_RW uint16_t SCFSR; /* H'10 */
    uint16_t RESERVED7; /* H'12 */
    FWK_R uint8_t SCFRDR; /* H'14 */
    uint8_t RESERVED8; /* H'15 */
    uint16_t RESERVED9; /* H'16 */
    FWK_RW uint16_t SCFCR; /* H'18 */
    uint16_t RESERVED10; /* H'1A */
    FWK_R uint16_t SCFDR; /* H'1C */
    uint16_t RESERVED11; /* H'1E */
    FWK_RW uint16_t SCSPTR; /* H'20 */
    uint16_t RESERVED12; /* H'22 */
    FWK_RW uint16_t SCLSR; /* H'24 */
    uint16_t RESERVED13; /* H'26 */
    uint32_t RESERVED14; /* H'28 */
    uint32_t RESERVED15; /* H'2C */
    FWK_RW uint16_t DL; /* H'30 */
    uint16_t RESERVED16; /* H'32 */
    FWK_RW uint16_t CKS; /* H'34 */
};

#define CPG_SMSTPCR2 (CPG_BASE + 0x0138)
#define CPG_SMSTPCR3 (CPG_BASE + 0x013C)
#define CPG_MSTPSR2 (CPG_BASE + 0x0040)
#define CPG_MSTPSR3 (CPG_BASE + 0x0048)
#define CPG_CPGWPR (CPG_BASE + 0x0900)

#define PRR (0xFFF00044)
#define PRR_PRODUCT_MASK (0x00007F00)
#define PRR_CUT_MASK (0x000000FF)
#define PRR_PRODUCT_H3_VER_10 (0x00004F00) /* R-Car H3 Ver.1.0 */

#define SCSMR_CA_MASK (1 << 7)
#define SCSMR_CA_ASYNC (0x0000)
#define SCSMR_CHR_MASK (1 << 6)
#define SCSMR_CHR_8 (0x0000)
#define SCSMR_PE_MASK (1 << 5)
#define SCSMR_PE_DIS (0x0000)
#define SCSMR_STOP_MASK (1 << 3)
#define SCSMR_STOP_1 (0x0000)
#define SCSMR_CKS_MASK (3 << 0)
#define SCSMR_CKS_DIV1 (0x0000)
#define SCSMR_INIT_DATA \
    (SCSMR_CA_ASYNC + SCSMR_CHR_8 + SCSMR_PE_DIS + SCSMR_STOP_1 + \
     SCSMR_CKS_DIV1)

#define MSTP310 (1 << 10)
#define MSTP26 (1 << 6)

#define SCSCR_TE_MASK (1 << 5)
#define SCSCR_TE_DIS (0x0000)
#define SCSCR_TE_EN (0x0020)
#define SCSCR_RE_MASK (1 << 4)
#define SCSCR_RE_DIS (0x0000)
#define SCSCR_RE_EN (0x0010)
#define SCSCR_CKE_MASK (3 << 0)
#define SCSCR_CKE_INT (0x0000)
#define SCSCR_CKE_BRG (0x0002)
#if SCIF_CLK == SCIF_EXTARNAL_CLK
#    define SCSCR_CKE_INT_CLK (SCSCR_CKE_BRG)
#else
#    define SCSCR_CKE_INT_CLK (SCSCR_CKE_INT)
#endif

#define SCFSR_INIT_DATA (0x0000)

#define SCFCR_TFRST_EN (0x0004)
#define SCFCR_RFRS_EN (0x0002)

#define FIFO_SIZE_BIT (5)
#define FIFO_MASK ((1 << FIFO_SIZE_BIT) - 1)
#define FIFO_R_SHIFT (0)
#define FIFO_T_SHIFT (8)
#define FIFO_FULL (16)
#define GET_SCFDR_T(reg) ((reg->SCFDR >> FIFO_T_SHIFT) & FIFO_MASK)

#define SCFCR_TTRG_8 (0x0000)
#define SCFCR_INIT_DATA (SCFCR_TTRG_8)

#define SCBRR_115200BPS (17) /* 115200bps@66MHz */
#define SCBRR_230400BPS (8) /* 230400bps@66MHz */

#endif /* SCIF_H */

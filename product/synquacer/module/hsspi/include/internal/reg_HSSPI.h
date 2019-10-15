/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_REG_HSSPI_H
#define INTERNAL_REG_HSSPI_H

#ifdef RESERVED_AREA_BYTE
#undef RESERVED_AREA_BYTE
#endif
#define RESERVED_AREA_BYTE(var1, var2)   \
    unsigned char reserved_##var1_##var2 \
        [(((var2) + 1) - (var1)) / sizeof(unsigned char)]

#ifdef RESERVED_AREA_HALF
#undef RESERVED_AREA_HALF
#endif
#define RESERVED_AREA_HALF(var1, var2)    \
    unsigned short reserved_##var1_##var2 \
        [(((var2) + 1) - (var1)) / sizeof(unsigned short)]

#ifdef RESERVED_AREA_WORD
#undef RESERVED_AREA_WORD
#endif
#define RESERVED_AREA_WORD(var1, var2)   \
    unsigned long reserved_##var1_##var2 \
        [(((var2) + 1) - (var1)) / sizeof(unsigned long)]

typedef enum {
    HSSPI_EN_TRP_MBM = 0,
    HSSPI_EN_TRP_DUAL = 1,
    HSSPI_EN_TRP_QUAD = 2,
    HSSPI_EN_TRP_SINGLE = 3
} HSSPI_EN_TRP_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int MEN : 1;
        unsigned int CSEN : 1;
        unsigned int reserved1 : 1;
        unsigned int CDSS : 1;
        unsigned int MES : 1;
        unsigned int SYNCON : 1;
        unsigned int reserved2 : 2;
        unsigned int reserved3 : 24;
    } bit;
} HSSPI_UN_MCTRL_t;

typedef enum {
    HSSPI_EN_MCTRL_MEM_DISABLE = 0,
    HSSPI_EN_MCTRL_MEM_ENABLE = 1
} HSSPI_EN_MCTRL_MEM_t;

typedef enum {
    HSSPI_EN_MCTRL_CSEN_DIRECT = 0,
    HSSPI_EN_MCTRL_CSEN_CMDSEQ = 1
} HSSPI_EN_MCTRL_CSEN_t;

typedef enum {
    HSSPI_EN_MCTRL_CDSS_iHCLK = 0,
    HSSPI_EN_MCTRL_CDSS_iPCLK = 1
} HSSPI_EN_MCTRL_CDSS_t;

typedef enum {
    HSSPI_EN_MCTRL_SYNCON_ASYNC = 0,
    HSSPI_EN_MCTRL_SYNCON_SYNC = 1
} HSSPI_EN_MCTRL_SYNCON_t;

#define HSSPI_MAKE_MCTRL(SYNCON, CDSS, CSEN, MEM) \
    (((SYNCON) << 5) | ((CDSS) << 3) | ((CSEN) << 1) | ((MEM) << 0))

typedef union {
    unsigned int DATA;
    struct {
        unsigned int CPHA : 1;
        unsigned int CPOL : 1;
        unsigned int ACES : 1;
        unsigned int RTM : 1;
        unsigned int SSPOL : 1;
        unsigned int SS2CD : 2;
        unsigned int SDIR : 1;
        unsigned int SENDIAN : 1;
        unsigned int CDRS : 7;
        unsigned int SAFESYNC : 1;
        unsigned int WRDSEL : 4;
        unsigned int RDDSEL : 2;
        unsigned int reserved1 : 1;
        unsigned int reserved2 : 8;
    } bit;
} HSSPI_UN_PCC_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int UMAFS : 1;
        unsigned int WAFS : 1;
        unsigned int PVFS : 1;
        unsigned int DWCBSFS : 1;
        unsigned int DRCBSFS : 1;
        unsigned int reserved1 : 3;
        unsigned int reserved2 : 24;
    } bit;
} HSSPI_UN_FAULTF_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int UMAFC : 1;
        unsigned int WAFC : 1;
        unsigned int PVFC : 1;
        unsigned int DWCBSFC : 1;
        unsigned int DRCBSFC : 1;
        unsigned int reserved1 : 3;
        unsigned int reserved2 : 24;
    } bit;
} HSSPI_UN_FAULTC_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int RXFTH : 4;
        unsigned int TXFTH : 4;
        unsigned int FWIDTH : 2;
        unsigned int TXCTRL : 1;
        unsigned int RXFLSH : 1;
        unsigned int TXFLSH : 1;
        unsigned int reserved1 : 3;
        unsigned int reserved2 : 16;
    } bit;
} HSSPI_UN_FIFOCFG_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int TFFS : 1;
        unsigned int TFES : 1;
        unsigned int TFOS : 1;
        unsigned int TFUS : 1;
        unsigned int TFLETS : 1;
        unsigned int TFMTS : 1;
        unsigned int TSSRS : 1;
        unsigned int reserved1 : 1;
        unsigned int reserved2 : 24;
    } bit;
} HSSPI_UN_TXF_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int TFFE : 1;
        unsigned int TFEE : 1;
        unsigned int TFOE : 1;
        unsigned int TFUE : 1;
        unsigned int TFLETE : 1;
        unsigned int TFMTE : 1;
        unsigned int TSSRE : 1;
        unsigned int reserved1 : 1;
        unsigned int reserved2 : 24;
    } bit;
} HSSPI_UN_TXE_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int TFFC : 1;
        unsigned int TFEC : 1;
        unsigned int TFOC : 1;
        unsigned int TFUC : 1;
        unsigned int TFLETC : 1;
        unsigned int TFMTC : 1;
        unsigned int TSSRC : 1;
        unsigned int reserved1 : 1;
        unsigned int reserved2 : 24;
    } bit;
} HSSPI_UN_TXC_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int RFFS : 1;
        unsigned int RFES : 1;
        unsigned int RFOS : 1;
        unsigned int RFUS : 1;
        unsigned int RFLETS : 1;
        unsigned int RFMTS : 1;
        unsigned int RSSRS : 1;
        unsigned int reserved1 : 1;
        unsigned int reserved2 : 24;
    } bit;
} HSSPI_UN_RXF;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int RFFE : 1;
        unsigned int RFEE : 1;
        unsigned int RFOE : 1;
        unsigned int RFUE : 1;
        unsigned int RFLETE : 1;
        unsigned int RFMTE : 1;
        unsigned int RSSRE : 1;
        unsigned int reserved1 : 1;
        unsigned int reserved2 : 24;
    } bit;
} HSSPI_UN_RXE_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int RFFC : 1;
        unsigned int RFEC : 1;
        unsigned int RFOC : 1;
        unsigned int RFUC : 1;
        unsigned int RFLETC : 1;
        unsigned int RFMTC : 1;
        unsigned int RSSRC : 1;
        unsigned int reserved1 : 1;
        unsigned int reserved2 : 24;
    } bit;
} HSSPI_UN_RXC_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int RXDMAEN : 1;
        unsigned int TXDMAEN : 1;
        unsigned int reserved1 : 6;
        unsigned int MUSTBE1 : 1;
        unsigned int SSDC : 1;
        unsigned int MSTARTEN : 1;
        unsigned int reserved2 : 5;
        unsigned int reserved3 : 16;
    } bit;
} HSSPI_UN_DMDMAENCFG_t;

typedef enum {
    HSSPI_EN_DMTRP_TRPW_SINGLE = 0,
    HSSPI_EN_DMTRP_TRPW_DUAL = 1,
    HSSPI_EN_DMTRP_TRPW_QUAD = 2
} HSSPI_EN_DMTRP_TRPW_t;

typedef enum {
    HSSPI_EN_DMTRP_TRPM_TXandRX = 0,
    HSSPI_EN_DMTRP_TRPM_RXonly = 1,
    HSSPI_EN_DMTRP_TRPM_TXonly = 2
} HSSPI_EN_DMTRP_TRPM_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int START : 1;
        unsigned int reserved1: 7;
        unsigned int STOP : 1;
        unsigned int reserved2 : 7;
        unsigned int PSEL : 2;
        unsigned int reserved3 : 6;
        unsigned int TRPW : 2;
        unsigned int TRPM : 2;
        unsigned int reserved4 : 4;
    } bit;
} HSSPI_UN_DMx_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int BCC : 16;
        unsigned int BCS : 16;
    } bit;
} HSSPI_UN_DMBCx_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int RXACTIVE : 1;
        unsigned int TXACTIVE : 1;
        unsigned int reserved1 : 6;
        unsigned int RXFLEVEL : 5;
        unsigned int reserved2 : 3;
        unsigned int TXFLEVEL : 5;
        unsigned int reserved3 : 3;
        unsigned int reserved4 : 8;
    } bit;
} HSSPI_UN_DMSTATUS_t;

typedef enum {
    HSSPI_EN_CSCFG_MSEL_8K = 0x0,
    HSSPI_EN_CSCFG_MSEL_16K = 0x1,
    HSSPI_EN_CSCFG_MSEL_32K = 0x2,
    HSSPI_EN_CSCFG_MSEL_64K = 0x3,
    HSSPI_EN_CSCFG_MSEL_128K = 0x4,
    HSSPI_EN_CSCFG_MSEL_256K = 0x5,
    HSSPI_EN_CSCFG_MSEL_512K = 0x6,
    HSSPI_EN_CSCFG_MSEL_1M = 0x7,
    HSSPI_EN_CSCFG_MSEL_2M = 0x8,
    HSSPI_EN_CSCFG_MSEL_4M = 0x9,
    HSSPI_EN_CSCFG_MSEL_8M = 0xA,
    HSSPI_EN_CSCFG_MSEL_16M = 0xB,
    HSSPI_EN_CSCFG_MSEL_32M = 0xC,
    HSSPI_EN_CSCFG_MSEL_64M = 0xD,
    HSSPI_EN_CSCFG_MSEL_128M = 0xE,
    HSSPI_EN_CSCFG_MSEL_256M = 0xF
} HSSPI_EN_CSCFG_MSEL_t;

typedef enum {
    HSSPI_EN_CSCFG_SRAM_RO = 0,
    HSSPI_EN_CSCFG_SRAM_RW = 1
} HSSPI_EN_CSCFG_SRAM_t;

#define HSSPI_SHIFT_CSCFG_SRAM (0)

typedef enum {
    HSSPI_EN_CSCFG_MBM_SINGLE = 0,
    HSSPI_EN_CSCFG_MBM_DUAL = 1,
    HSSPI_EN_CSCFG_MBM_QUAD = 2
} HSSPI_EN_CSCFG_MBM_t;

#define HSSPI_SHIFT_CSCFG_MBM (1)

typedef union {
    unsigned int DATA;
    struct {
        unsigned int SRAM : 1; /* B[00] */
        unsigned int MBM : 2; /* B[02:01] */
        unsigned int SPICHNG : 1; /* B[03] */
        unsigned int BOOTEN : 1; /* B[04] */
        unsigned int BSEL : 1; /* B[05] */
        unsigned int reserved1 : 2; /* B[07:06] */
        unsigned int SSEL0EN : 1; /* B[08] */
        unsigned int SSEL1EN : 1; /* B[09] */
        unsigned int SSEL2EN : 1; /* B[10] */
        unsigned int SSEL3EN : 1; /* B[11] */
        unsigned int reserved2 : 4; /* B[15:12] */
        unsigned int MSEL : 4; /* B[19:16] */
        unsigned int reserved3 : 4; /* B[23:20] */
        unsigned int reserved4 : 8; /* B[31:24] */
    } bit;
} HSSPI_UN_CSCFG_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int ITIME : 16; /* B[15:00] */
        unsigned int reserved1 : 16; /* B[31:16] */
    } bit;
} HSSPI_UN_CSITIME_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int reserved1 : 13; /* B[12:00] */
        unsigned int AEXT : 19; /* B[31:13] */
    } bit;
} HSSPI_UN_CSAEXT_t;

#define MAKE_CSDC(DATA, CONT, TRP, DEC) \
    (((DATA) << 8) | ((CONT) << 3) | ((TRP) << 1) | ((DEC) << 0))

typedef union {
    unsigned int DATA;
    struct {
        unsigned int DEC : 1; /* B[00] */
        unsigned int TRP : 2; /* B[02:01] */
        unsigned int CONT : 1; /* B[03] */
        unsigned int reserved1 : 4; /* B[07:04] */
        unsigned int DATA : 8; /* B[15:08] */
        unsigned int reserved2 : 16; /* B[31:16] */
    } bit;
} HSSPI_UN_CSDC_LO_t;

typedef union {
    unsigned int DATA;
    struct {
        unsigned int reserved1 : 16; /* B[15:00] */
        unsigned int DEC : 1; /* B[16] */
        unsigned int TRP : 2; /* B[18:17] */
        unsigned int CONT : 1; /* B[19] */
        unsigned int reserved2 : 4; /* B[23:20] */
        unsigned int DATA : 8; /* B[31:24] */
    } bit;
} HSSPI_UN_CSDC_HI_t;

typedef union {
    unsigned int DATA;
    HSSPI_UN_CSDC_LO_t CSDC_LO;
    HSSPI_UN_CSDC_HI_t CSDC_HI;
    struct {
        unsigned int BIT1500 : 16;
        unsigned int BIT3116 : 16;
    } half;
} HSSPI_UN_CSDC_ITEM_t;

typedef union {
    unsigned short HALF[8];
    HSSPI_UN_CSDC_ITEM_t CSDC[4];
} HSSPI_UN_CSDC_t;

typedef struct {
    HSSPI_UN_MCTRL_t MCTRL;
    HSSPI_UN_PCC_t PCC[4];
    HSSPI_UN_TXF_t TXF;
    HSSPI_UN_TXE_t TXE;
    HSSPI_UN_TXC_t TXC;
    HSSPI_UN_RXF RXF;
    HSSPI_UN_RXE_t RXE;
    HSSPI_UN_RXC_t RXC;
    HSSPI_UN_FAULTF_t FAULTF;
    HSSPI_UN_FAULTC_t FAULTC;
    HSSPI_UN_DMDMAENCFG_t DMDMAEN_DMCFG;
    HSSPI_UN_DMx_t DMx;
    HSSPI_UN_DMBCx_t DMBCx;
    HSSPI_UN_DMSTATUS_t DMSTATUS;
    RESERVED_AREA_WORD(0x044, 0x04B);
    HSSPI_UN_FIFOCFG_t FIFOCFG;
    unsigned int TXFIFO[16];
    unsigned int RXFIFO[16];
    HSSPI_UN_CSCFG_t CSCFG;
    HSSPI_UN_CSITIME_t CSITIME;
    HSSPI_UN_CSAEXT_t CSAEXT;
    HSSPI_UN_CSDC_t RDCSDC;
    HSSPI_UN_CSDC_t WRCSDC;
    unsigned int MID;
} REG_ST_HSSPI_t;

#endif /* INTERNAL_REG_HSSPI_H */

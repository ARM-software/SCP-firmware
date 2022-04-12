/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef HSSPI_REG_H
#define HSSPI_REG_H

#include <fwk_macros.h>

#include <stdint.h>

struct hsspi_reg {
    FWK_RW uint32_t MCTRL;
    FWK_RW uint32_t PCC[4];
    FWK_R uint32_t TXF;
    FWK_RW uint32_t TXE;
    FWK_W uint32_t TXC;
    FWK_R uint32_t RXF;
    FWK_RW uint32_t RXE;
    FWK_W uint32_t RXC;
    FWK_R uint32_t FAULTF;
    FWK_W uint32_t FAULTC;
    FWK_RW uint32_t DMCFG;
    FWK_RW uint32_t DMCTRL;
    FWK_RW uint32_t DMBCCS;
    FWK_R uint32_t DMSTATUS;
    uint32_t RESERVED[2];
    FWK_RW uint32_t FIFOCFG;
    FWK_W uint32_t TXFIFO[16];
    FWK_R uint32_t RXFIFO[16];
    FWK_RW uint32_t CSCFG;
    FWK_RW uint32_t CSITIME;
    FWK_RW uint32_t CSAEXT;
    FWK_RW uint32_t RDCSDC[4];
    FWK_RW uint32_t WRCSDC[4];
    FWK_R uint32_t MID;
};

#define MAX_SLAVE_NUM (4)

#define DISABLE (0)
#define ENABLE  (1)

/* MCTRL reigster field */
#define MCTRL_MES(val)    (((val)&0x1) << 4)
#define MCTRL_MES_MASK    MCTRL_MES(1)
#define MCTRL_CDSS(val)   (((val)&0x1) << 3)
#define MCTRL_CDSS_MASK   MCTRL_CDSS(1)
#define MCTRL_CSEN(val)   (((val)&0x1) << 1)
#define MCTRL_CSEN_MASK   MCTRL_CSEN(1)
#define MCTRL_MEN(val)    ((val)&0x1)
#define MCTRL_MEN_MASK    MCTRL_MEN(1)

/* PCC reigster field */
#define PCC_RD_DESEL_TIME(val) (((val)&0xF) << 21)
#define PCC_RD_DESEL_TIME_MASK PCC_RD_DESEL_TIME(0xF)

#define PCC_WR_DESEL_TIME(val) (((val)&0xF) << 17)
#define PCC_WR_DESEL_TIME_MASK PCC_WR_DESEL_TIME(0xF)

#define PCC_CLOCK_DIV(val) (((val)&0x7F) << 9)
#define PCC_CLOCK_DIV_MASK PCC_CLOCK_DIV(0x7F)

/* CSCFG reigster field */
#define CSCFG_MSEL(val)        (((val)&0xF) << 16)
#define CSCFG_MSEL_MASK        CSCFG_MSEL(0xF)
#define MEMORY_BANK_SIZE(msel) (0x2000UL << (msel))
#define MEMORY_BANK_MASK(msel) ((0x2000UL << (msel)) - 1)

#define CSCFG_SSELEN(s0, s1, s2, s3) \
    ((((s0)&0x1) << 8) | (((s1)&0x1) << 9) | (((s2)&0x1) << 10) | \
     (((s3)&0x1) << 11))
#define CSCFG_SSELEN_MASK CSCFG_SSELEN(1, 1, 1, 1)

#define CSCFG_BOOTEN(val) (((val)&0x1) << 4)
#define CSCFG_BOOTEN_MASK CSCFG_BOOTEN(0x1)

#define CSCFG_SPICHG(val) (((val)&0x1) << 3)
#define CSCFG_SPICHG_MASK CSCFG_SPICHG(0x1)

#define CSCFG_MBM(val) (((val)&0x3) << 1)
#define CSCFG_MBM_MASK CSCFG_MBM(0x3)
#define IO_SINGLE      (0x0)
#define IO_DUAL        (0x1)
#define IO_QUAD        (0x2)

#define CSCFG_SRAM(val) ((val)&0x1)
#define CSCFG_SRAM_MASK CSCFG_SRAM(0x1)
#define READ_ONLY       (0x0)
#define WRITABLE        (0x1)

/* CSITIME register field */
#define CSITIME_MASK (0xFFFF)
#define CSITIME_256  (0x100)

/* RDCSDC, WRCSDC register field */
#define DATA(val)    (((val)&0xFF) << 8)
#define TRP(val)     (((val)&0x3) << 1)
#define TRP_DEFAULT  (0x0)
#define TRP_DUAL     (0x1)
#define TRP_QUAD     (0x2)
#define TRP_SINGLE   (0x3)
#define DEC(val)     ((val)&0x1)
#define USE_RAW_DATA (0x0)
#define USE_DEC_DATA (0x1)

/* Decoded Data */
#define MEMORY_ADDR_07_00 (0x0)
#define MEMORY_ADDR_15_08 (0x1)
#define MEMORY_ADDR_23_16 (0x2)
#define MEMORY_ADDR_31_24 (0x3)
#define DUMMY_CYCLE_1BYTE (0x4)
#define ALT_NIBBLE_FORMAT (0x5)
#define DATA_PHASE_START  (0x7)

#define SET_RAW_DATA(data, trp) (DATA(data) | TRP(trp) | DEC(USE_RAW_DATA))
#define SET_DEC_DATA(data, trp) (DATA(data) | TRP(trp) | DEC(USE_DEC_DATA))
#define SET_ALT_NIBBLE(data, trp) \
    SET_DEC_DATA(((data & 0xF0) | ALT_NIBBLE_FORMAT), trp)
#define SET_ADDR_1BYTE(trp)  SET_DEC_DATA(MEMORY_ADDR_07_00, trp)
#define SET_ADDR_2BYTE(trp)  SET_DEC_DATA(MEMORY_ADDR_15_08, trp)
#define SET_ADDR_3BYTE(trp)  SET_DEC_DATA(MEMORY_ADDR_23_16, trp)
#define SET_ADDR_4BYTE(trp)  SET_DEC_DATA(MEMORY_ADDR_31_24, trp)
#define SET_DUMMY_1BYTE(trp) SET_DEC_DATA(DUMMY_CYCLE_1BYTE, trp)
#define SET_DUMMY_CYCLE_2    SET_DEC_DATA(DUMMY_CYCLE_1BYTE, TRP_QUAD)
#define SET_DUMMY_CYCLE_4    SET_DEC_DATA(DUMMY_CYCLE_1BYTE, TRP_DUAL)
#define SET_DUMMY_CYCLE_8    SET_DEC_DATA(DUMMY_CYCLE_1BYTE, TRP_SINGLE)
#define END_OF_COMMAND(trp)  SET_DEC_DATA(DATA_PHASE_START, trp)

#endif /* HSSPI_REG_H */

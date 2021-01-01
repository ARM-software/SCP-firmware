/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_REG_DMA330_H
#define INTERNAL_REG_DMA330_H

#include <stdbool.h>
#include <stdint.h>

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

typedef struct {
    uint32_t CSR; /* 0x100,...,0x138 Channel status for DMA channel 0~7 */
    uint32_t CPC; /* 0x104,...,0x13C Channel PC for DMA channel 0~7     */
} REG_ST_DMA330_CT_t;

typedef struct {
    uint32_t SAR; /* 0x400,...,0x4E0 Source address for DMA channel 0~7      */
    uint32_t DAR; /* 0x404,...,0x4E4 Destination address for DMA channel 0~7 */
    uint32_t CCR; /* 0x408,...,0x4E8 Channel control for DMA channel 0~7     */
    uint32_t LC0; /* 0x40C,...,0x4EC Loop counter 0 for DMA channel 0~7      */
    uint32_t LC1; /* 0x410,...,0x4F0 Loop counter 1 for DMA channel 0~7      */
    uint32_t reserved[3];
} REG_ST_DMA330_AS_t;

typedef union {
    uint32_t DATA;
    struct {
        uint32_t SRC_INC : 1; /* B00            */
        uint32_t SRC_BURST_SIZE : 3; /* B01-B03        */
        uint32_t SRC_BURST_LEN : 4; /* B04-B07        */
        uint32_t SRC_PROT_CTRL : 3; /* B08-B10        */
        uint32_t SRC_CACHE_CTRL : 3; /* B11-B13        */
        uint32_t DST_INC : 1; /* B14            */
        uint32_t DST_BURST_SIZE : 3; /* B15-B17        */
        uint32_t DST_BURST_LEN : 4; /* B18-B21        */
        uint32_t DST_PROT_CTRL : 3; /* B22-B24        */
        uint32_t DST_CACHE_CTRL : 3; /* B25-B27        */
        uint32_t ENDIAN_SWAP_SIZE : 3; /* B28-B30        */
        uint32_t reserved1 : 1; /* B31 Reserved   */
    } bit;
} DMA330_UN_CCR_t;

#define DMA330_MAKE_CCR(ES, DC, DP, DL, DS, DI, SC, SP, SL, SS, SI)           \
    (((ES) << 28) | ((DC) << 25) | ((DP) << 22) | ((DL) << 18) |              \
     ((DS) << 15) | ((DI) << 14) | ((SC) << 11) | ((SP) << 8) | ((SL) << 4) | \
     ((SS) << 1) | ((SI) << 0))

typedef union {
    uint32_t DATA;
    struct {
        uint32_t FS_MGR : 1; /* B00                */
        uint32_t reserved1 : 31; /* B01-B31 Reserved   */
    } bit;
} DMA330_UN_FSRD_t;

typedef union {
    uint32_t DATA;
    struct {
        uint32_t CH0 : 1; /* B00               */
        uint32_t CH1 : 1; /* B01               */
        uint32_t CH2 : 1; /* B02               */
        uint32_t CH3 : 1; /* B03               */
        uint32_t CH4 : 1; /* B04               */
        uint32_t CH5 : 1; /* B05               */
        uint32_t CH6 : 1; /* B06               */
        uint32_t CH7 : 1; /* B07               */
        uint32_t reserved1 : 24; /* B08-B31 Reserved  */
    } bit;
} DMA330_UN_FSRC_t;

// 3.3.9 Fault Type DMA Manager Register
typedef union {
    uint32_t DATA;
    struct {
        uint32_t UNDEF_INSTR : 1; /* B00                 */
        uint32_t OPERAND_INVALID : 1; /* B01                 */
        uint32_t reserved1 : 2; /* B02-B03    Reserved */
        uint32_t DMAGO_ERR : 1; /* B04                 */
        uint32_t MGR_EVNT_ERR : 1; /* B05                 */
        uint32_t reserved2 : 10; /* B06-B15    Reserved */
        uint32_t INSTR_FETCH_ERR : 1; /* B16                 */
        uint32_t reserved3 : 13; /* B17-B29    Reserved */
        uint32_t DBG_INSTR : 1; /* B30                 */
        uint32_t reserved4 : 1; /* B31        Reserved */
    } bit;
} DMA330_UN_FTRD_t;

// Fault Type DMA Channel Registers
typedef union {
    uint32_t DATA;
    struct {
        uint32_t UNDEF_INSTR : 1; /* B00              */
        uint32_t OPERAND_INVALID : 1; /* B01              */
        uint32_t reserved1 : 3; /* B02-B04 Reserved */
        uint32_t CH_EVNT_ERR : 1; /* B05              */
        uint32_t CH_PERIPH_ERR : 1; /* B06              */
        uint32_t CH_RDWR_ERR : 1; /* B07              */
        uint32_t reserved2 : 4; /* B08-B11 Reserved */
        uint32_t MFIFO_ERR : 1; /* B12              */
        uint32_t ST_DATA_UNAVAILABLE : 1; /* B13              */
        uint32_t reserved3 : 2; /* B14-B15 Reserved */
        uint32_t INSTR_FETCH_ERR : 1; /* B16              */
        uint32_t DATA_WRITE_ERR : 1; /* B17              */
        uint32_t DATA_READ_ERR : 1; /* B18              */
        uint32_t reserved4 : 11; /* B19-B29 Reserved */
        uint32_t DBG_INSTR : 1; /* B30              */
        uint32_t LOCKUP_ERR : 1; /* B31              */
    } bit;
} DMA330_UN_FTR_t;

typedef struct {
    uint32_t DSR; /* +0x000 DMA Manager Status Register         */
    uint32_t DPC; /* +0x004 DMA Program Counter Register        */
    RESERVED_AREA_WORD(0x008, 0x01F);
    uint32_t INTEN; /* +0x020 Interrupt Enable Register           */
    uint32_t INT_EVENT_RIS; /* +0x024 Event-Interrupt Raw Status Register */
    uint32_t INTMIS; /* +0x028 Interrupt Status Register           */
    uint32_t INTCLR; /* +0x02C Interrupt Clear Register            */
    uint32_t FSRD; /* +0x030 Fault Status DMA Manager Register   */
    uint32_t FSRC; /* +0x034 Fault Status DMA Channel Register   */
    uint32_t FTRD; /* +0x038 Fault Type DMA Manager Register     */
    RESERVED_AREA_WORD(0x03C, 0x03F);
    uint32_t FTR[8]; /* +0x040-0x05C Fault Type DMA Channel Registers */
    RESERVED_AREA_WORD(0x060, 0x0FF);
    REG_ST_DMA330_CT_t
        CT[8]; /* +0x100-0x13C DMA Channel Thread Status Register */
    RESERVED_AREA_WORD(0x140, 0x3FF);
    REG_ST_DMA330_AS_t
        AS[8]; /* +0x400-0x4FC AXI Status And Loop Counter Register */
    RESERVED_AREA_WORD(0x500, 0xCFF);
    uint32_t DBGSTATUS; /* +0xD00 Debug Status Register */
    uint32_t DBGCMD; /* +0xD04 Debug Command Register */
    uint32_t DBGINST[2]; /* +0xD08-0xD0C Debug Instruction-0,1 Register */
    RESERVED_AREA_WORD(0xD10, 0xDFF);
    uint32_t CR[5]; /* +0xE00-0xE10 Configuration Register 0~4 */
    uint32_t CRD; /* +0xE14 DMA Configuration Register */
    RESERVED_AREA_WORD(0xE18, 0xE7F);
    uint32_t WD; /* +0xE80 Watchdog Register */
    RESERVED_AREA_WORD(0xE84, 0xFDF);
    uint32_t
        PERIPH_ID[4]; /* +0xFE0-0xFEC Peripheral Identification Registers */
    uint32_t
        PCELL_ID[4]; /* +0xFF0-0xFFC Component Identification Registers 0-3 */
} REG_ST_DMA330_S_t;

typedef struct {
    uint32_t DSR; /* +0x000 */
    uint32_t DPC; /* +0x004 */
    RESERVED_AREA_WORD(0x008, 0x01F);
    uint32_t INTEN; /* +0x020 */
    uint32_t INT_EVENT_RIS; /* +0x024 */
    uint32_t INTMIS; /* +0x028 */
    uint32_t INTCLR; /* +0x02C */
    uint32_t FSRD; /* +0x030 */
    uint32_t FSRC; /* +0x034 */
    uint32_t FTRD; /* +0x038 */
    RESERVED_AREA_WORD(0x03C, 0x03F);
    uint32_t FTR[8]; /* +0x040-0x05C */
    RESERVED_AREA_WORD(0x060, 0x0FF);
    REG_ST_DMA330_CT_t CT[8]; /* +0x100-0x13C */
    RESERVED_AREA_WORD(0x140, 0x3FF);
    REG_ST_DMA330_AS_t AS[8]; /* +0x400-0x4FC */
    RESERVED_AREA_WORD(0x500, 0xCFF);
    uint32_t DBGSTATUS; /* +0xD00 */
    uint32_t DBGCMD; /* +0xD04 */
    uint32_t DBGINST[2]; /* +0xD08-0xD0C */
    RESERVED_AREA_WORD(0xD10, 0xDFF);
    uint32_t CR[5]; /* +0xE00-0xE10 */
    uint32_t CRD; /* +0xE14 */
    RESERVED_AREA_WORD(0xE18, 0xE7F);
    uint32_t WD; /* +0xE80 */
    RESERVED_AREA_WORD(0xE84, 0xFDF);
    uint32_t PERIPH_ID[4]; /* +0xFE0-0xFEC */
    uint32_t PCELL_ID[4]; /* +0xFF0-0xFFC */

} REG_ST_DMA330_NS_t;

#endif /* INTERNAL_REG_DMA330_H */

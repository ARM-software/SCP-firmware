/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "synquacer_mmap.h"

#include <cmsis_os2.h>

#include <internal/hsspi_driver.h>
#include <internal/reg_HSSPI.h>

#include <mod_synquacer_system.h>

#include <fwk_log.h>

#include <fmw_cmsis.h>

#include <stddef.h>
#include <stdint.h>

static const uint16_t m_awcCommandList_NULL[] = {
    /*    DATA[15:8]    CONT[3]        TRP[2:1]    DEC[0]    */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1) /* EndOfList */
};

static const uint16_t m_awcCommandList_CMD_S[] = {
    /*    DATA[15:8]    CONT[3] TRP[2:1]    DEC[0]    */
    MAKE_CSDC(0x00, 0, HSSPI_EN_TRP_SINGLE, 0), /* COMMAND   */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1) /* EndOfList */
};

#ifdef CONFIG_SCB_USE_4BYTE_MODE
static const uint16_t m_awcCommandList_CMD_S_ADDR4_S_D8_S[] = {
    /*    DATA[15:8]    CONT[3] TRP[2:1]    DEC[0]    */
    MAKE_CSDC(0x00, 0, HSSPI_EN_TRP_SINGLE, 0), /* COMMAND     */
    MAKE_CSDC(0x03, 0, HSSPI_EN_TRP_SINGLE, 1), /* ADDR[31:24] */
    MAKE_CSDC(0x02, 0, HSSPI_EN_TRP_SINGLE, 1), /* ADDR[23:16] */
    MAKE_CSDC(0x01, 0, HSSPI_EN_TRP_SINGLE, 1), /* ADDR[15:08] */
    MAKE_CSDC(0x00, 0, HSSPI_EN_TRP_SINGLE, 1), /* ADDR[07:00] */
    MAKE_CSDC(0x04, 0, HSSPI_EN_TRP_SINGLE, 1), /* DUMMY CYCLE x8 */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList   */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList   */
};
#else /* CONFIG_SCB_USE_4BYTE_MODE */
static const uint16_t m_awcCommandList_CMD_S_ADDR3_S_D8_S[] = {
    /*    DATA[15:8]    CONT[3] TRP[2:1]    DEC[0]    */
    MAKE_CSDC(0x00, 0, HSSPI_EN_TRP_SINGLE, 0), /* COMMAND     */
    MAKE_CSDC(0x02, 0, HSSPI_EN_TRP_SINGLE, 1), /* ADDR[23:16] */
    MAKE_CSDC(0x01, 0, HSSPI_EN_TRP_SINGLE, 1), /* ADDR[15:08] */
    MAKE_CSDC(0x00, 0, HSSPI_EN_TRP_SINGLE, 1), /* ADDR[07:00] */
    MAKE_CSDC(0x04, 0, HSSPI_EN_TRP_SINGLE, 1), /* DUMMY CYCLE x8 */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList   */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList   */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1) /* EndOfList   */
};
#endif /* CONFIG_SCB_USE_4BYTE_MODE */

static const uint16_t m_awcCommandList_SingleCMD[] = {
    /*    DATA[15:8]    CONT[3]        TRP[2:1]    DEC[0]    */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 0),
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1) /* EndOfList */
};

static const uint16_t m_awcCommandList_CMD_S_ADDR2_S[] = {
    /*    DATA[15:8]    CONT[3] TRP[2:1]    DEC[0]    */
    MAKE_CSDC(0x00, 0, HSSPI_EN_TRP_SINGLE, 0), /* COMMAND     */
    MAKE_CSDC(0x01, 0, HSSPI_EN_TRP_SINGLE, 1), /* ADDR[15:08] */
    MAKE_CSDC(0x00, 0, HSSPI_EN_TRP_SINGLE, 1), /* ADDR[07:00] */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList   */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList   */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList   */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1), /* EndOfList   */
    MAKE_CSDC(0x07, 0, HSSPI_EN_TRP_MBM, 1) /* EndOfList   */
};

static void hsspi_stop(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    HSSPI_EN_MCTRL_CDSS_t clk_sel,
    int syncon)
{
    (*reg_hsspi).MCTRL.DATA = HSSPI_MAKE_MCTRL(
        syncon,
        clk_sel,
        HSSPI_EN_MCTRL_CSEN_CMDSEQ,
        HSSPI_EN_MCTRL_MEM_DISABLE);
    do {
        __DSB();
    } while ((*reg_hsspi).MCTRL.bit.MES != 0);
}

static void hsspi_csen(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    HSSPI_EN_MCTRL_CDSS_t clk_sel,
    int syncon)
{
    (*reg_hsspi).MCTRL.DATA = HSSPI_MAKE_MCTRL(
        syncon, clk_sel, HSSPI_EN_MCTRL_CSEN_CMDSEQ, HSSPI_EN_MCTRL_MEM_ENABLE);
    do {
        __DSB();
    } while ((*reg_hsspi).MCTRL.bit.MES == 0);
}

static void hsspi_raw_set_csdc(
    volatile HSSPI_UN_CSDC_t *csdc,
    const uint16_t *command_list,
    int command)
{
    int nIndex;

    for (nIndex = 0; nIndex < 8;
         csdc->HALF[nIndex] = command_list[nIndex], ++nIndex)
        continue;

    if (!(command < 0))
        csdc->CSDC[0].CSDC_LO.bit.DATA = command;
}

static void hsspi_read_command_sequence(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    const uint16_t *command_list,
    int command)
{
    hsspi_raw_set_csdc(&reg_hsspi->RDCSDC, command_list, command);
}

void hsspi_read_command_sequence_addr2(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    int command)
{
    hsspi_read_command_sequence(
        reg_hsspi, m_awcCommandList_CMD_S_ADDR2_S, command);
}

static void hsspi_write_command_sequence(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    const uint16_t *command_list,
    int command)
{
    hsspi_raw_set_csdc(&reg_hsspi->WRCSDC, command_list, command);
}

void hsspi_write_command_sequence_addr2(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    int command)
{
    hsspi_write_command_sequence(
        reg_hsspi, m_awcCommandList_CMD_S_ADDR2_S, command);
}

void hsspi_write_command_direct(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi,
    int command)
{
    hsspi_raw_set_csdc(&reg_hsspi->WRCSDC, m_awcCommandList_NULL, -1);
    if (!(command < 0))
        MEM_HSSPI_BYTE(mem_hsspi)[0] = command;
}

uint8_t m_abyJEDEC_ID[3];

static void hsspi_read_jedec_id(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi)
{
    hsspi_read_command_sequence(
        reg_hsspi, m_awcCommandList_CMD_S, HSSPI_EN_STARTUP_COMMAND_READ_ID);

    /* +00h Manufacturer ID */
    m_abyJEDEC_ID[0] = MEM_HSSPI_BYTE(mem_hsspi)[0];
    /* +01h Device ID Most Significant Byte - Memory Interface Type */
    m_abyJEDEC_ID[1] = MEM_HSSPI_BYTE(mem_hsspi)[1];
    /* +02h Device ID Least Significant Byte - Density */
    m_abyJEDEC_ID[2] = MEM_HSSPI_BYTE(mem_hsspi)[2];
}

static void hsspi_dual_output_fast_read(volatile REG_ST_HSSPI_t *reg_hsspi)
{
#ifdef CONFIG_SCB_USE_4BYTE_MODE
    hsspi_read_command_sequence(
        reg_hsspi,
        m_awcCommandList_CMD_S_ADDR4_S_D8_S,
        HSSPI_EN_STARTUP_COMMAND_DOFR_4B);
#else /* CONFIG_SCB_USE_4BYTE_MODE */
    hsspi_read_command_sequence(
        reg_hsspi,
        m_awcCommandList_CMD_S_ADDR3_S_D8_S,
        HSSPI_EN_STARTUP_COMMAND_DOFR);
#endif /* CONFIG_SCB_USE_4BYTE_MODE */
}

static void hsspi_quad_output_fast_read(volatile REG_ST_HSSPI_t *reg_hsspi)
{
#ifdef CONFIG_SCB_USE_4BYTE_MODE
    hsspi_read_command_sequence(
        reg_hsspi,
        m_awcCommandList_CMD_S_ADDR4_S_D8_S,
        HSSPI_EN_STARTUP_COMMAND_QOFR_4B);
#else /* CONFIG_SCB_USE_4BYTE_MODE */
    hsspi_read_command_sequence(
        reg_hsspi,
        m_awcCommandList_CMD_S_ADDR3_S_D8_S,
        HSSPI_EN_STARTUP_COMMAND_QOFR);
#endif /* CONFIG_SCB_USE_4BYTE_MODE */
}

static void hsspi_wait_status_register_for_wip(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi)
{
    /* Status Register    */
    union general_status_register {
        uint32_t WORD;
        uint8_t BYTE[4];
        struct {
            uint32_t WIP : 1; /* B[0] Write in progress */
            uint32_t WEL : 1; /* B[1] Write enable latch */
            uint32_t BP : 3; /* B[4:2]    */
            uint32_t E_ERR : 1; /* B[5]    */
            uint32_t P_ERR : 1; /* B[6]    */
            uint32_t SRWD : 1; /* B[7]    */
            uint32_t reserved1 : 24; /* reserved */
        } bit;
    } unRDSR;

    /* 05h : RDSR Read Status Register    */
    hsspi_read_command_sequence(reg_hsspi, m_awcCommandList_CMD_S, 0x05);
    do {
        unRDSR.WORD = MEM_HSSPI_BYTE(mem_hsspi)[0];
    } while (unRDSR.bit.WIP);
}

unsigned char hsspi_read_command_direct(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi,
    int command)
{
    hsspi_raw_set_csdc(&reg_hsspi->RDCSDC, m_awcCommandList_SingleCMD, command);

    return MEM_HSSPI_BYTE(mem_hsspi)[0];
}

static void hsspi_enter_to_quad_for_spansion(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi)
{
    /* Status Register 1 (SR1) */
    union spansion_status_register_1 {
        uint32_t WORD;
        uint8_t BYTE[4];
        struct {
            uint32_t WIP : 1; /* B[0]     */
            uint32_t WEL : 1; /* B[1]     */
            uint32_t BP : 3; /* B[4:2]   */
            uint32_t E_ERR : 1; /* B[5]     */
            uint32_t P_ERR : 1; /* B[6]     */
            uint32_t SRWD : 1; /* B[7]     */
            uint32_t reserved1 : 24; /* reserved */
        } bit;
    } unSR1;

    /* Configuration Register 1 (CR1) */
    union spansion_configuration_register_1 {
        uint32_t WORD;
        uint8_t BYTE[4];
        struct {
            uint32_t FREEZE : 1; /* B[0] */
            uint32_t QUAD : 1; /* B[1] (0=Dual or Serial, 1=Quad) */
            uint32_t reserved1 : 1; /* B[2] Reserved for Future Use */
            uint32_t BPNV : 1; /* B[3] (0=Non-Volatile, 1=Volatile) */
            uint32_t reserved2 : 1; /* B[4] Reserved for Future Use */
            uint32_t TBPROT : 1; /* B[5] */
            uint32_t LC : 2; /* B[7:6] */
            uint32_t reserved3 : 24; /* reserved */
        } bit;
    } unCR1;

    /* 05h : RDSR1 Read Status Register-1 */
    hsspi_read_command_sequence(reg_hsspi, m_awcCommandList_CMD_S, 0x05);
    unSR1.WORD = MEM_HSSPI_BYTE(mem_hsspi)[0];
    /* 35h : RDCR Read Configuration Register-1 */
    hsspi_read_command_sequence(reg_hsspi, m_awcCommandList_CMD_S, 0x35);
    unCR1.WORD = MEM_HSSPI_BYTE(mem_hsspi)[0];
    unCR1.bit.QUAD = 1; /* QUAD MODE ENTRY */
    /* 06h : WRITE ENABLE */
    hsspi_write_command_direct(reg_hsspi, mem_hsspi, 0x06);
    /* 01h : WRR Write Register (Status-1, Configuration-1) */
    hsspi_write_command_sequence(reg_hsspi, m_awcCommandList_CMD_S, 0x01);
    MEM_HSSPI_BYTE(mem_hsspi)[0] = unSR1.BYTE[0];
    MEM_HSSPI_BYTE(mem_hsspi)[1] = unCR1.BYTE[0];
    /* 05h : READ STATUS REGISTER */
    hsspi_wait_status_register_for_wip(reg_hsspi, mem_hsspi);
}

static void hsspi_enter_to_quad_for_micron(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi)
{
    /* Enhanced Volatile Configuration Register */
    union micron_enhanced_volatile_configuration_register {
        uint32_t WORD;
        uint8_t BYTE[4];
        struct {
            uint32_t OUTPUT_DRIVER_STRENGTH : 3; /* B[2:0] */
            uint32_t VPP_ACCELERATOR : 1; /* B[3] */
            uint32_t RESET_HOLD : 1; /* B[4] Reset/hold */
            uint32_t reserved1 : 1; /* B[5]    reserved */
            uint32_t DUAL_IO_PROTOCOL : 1; /* B[6] */
            uint32_t QUAD_IO_PROTOCOL : 1; /* B[7] */
            uint32_t reserved2 : 24; /* reserved */
        } bit;
    } unEVCR;

    /* 65h : READ ENHANCED VOLATILE CONFIGURATION REGISTER */
    hsspi_read_command_sequence(reg_hsspi, m_awcCommandList_CMD_S, 0x65);
    unEVCR.WORD = MEM_HSSPI_BYTE(mem_hsspi)[0];
    unEVCR.bit.RESET_HOLD = 0; /* QUAD MODE ENTRY */
    /* 06h : WRITE ENABLE */
    hsspi_write_command_direct(reg_hsspi, mem_hsspi, 0x06);
    /* E9h : EXIT 4-BYTE ADDRESS MODE */
    hsspi_write_command_direct(reg_hsspi, mem_hsspi, 0xE9);
    /* 06h : WRITE ENABLE */
    hsspi_write_command_direct(reg_hsspi, mem_hsspi, 0x06);
    /* 61h : WRITE ENHANCED VOLATILE CONFIGURATION REGISTER */
    hsspi_write_command_sequence(reg_hsspi, m_awcCommandList_CMD_S, 0x61);
    MEM_HSSPI_BYTE(mem_hsspi)[0] = unEVCR.BYTE[0];
    /* 05h : READ STATUS REGISTER */
    hsspi_wait_status_register_for_wip(reg_hsspi, mem_hsspi);
}

static void hsspi_enter_to_quad_for_winbond(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi)
{
    /* Status Register-1 */
    union winbond_status_register_1 {
        uint32_t WORD;
        uint8_t BYTE[4];
        struct {
            uint32_t BUSY : 1; /* B[0]     */
            uint32_t WEL : 1; /* B[1]     */
            uint32_t BP : 3; /* B[4:2]   */
            uint32_t TB : 1; /* B[5]     */
            uint32_t SEC : 1; /* B[6]     */
            uint32_t SRP0 : 1; /* B[7]     */
            uint32_t reserved1 : 24; /* reserved */
        } bit;
    } unSR1;

    /* Status Register-2 */
    union winbond_status_register_2 {
        uint32_t WORD;
        uint8_t BYTE[4];
        struct {
            uint32_t SRP1 : 1; /* B[0]    */
            uint32_t QE : 1; /* B[1] Quad Enable (0=Disable, 1=Enable) */
            uint32_t reserved1 : 1; /* B[2]     */
            uint32_t LB : 3; /* B[5:3]   */
            uint32_t CMP : 1; /* B[6]     */
            uint32_t SUS : 1; /* B[7]     */
            uint32_t reserved2 : 24; /* reserved */
        } bit;
    } unSR2;

    /* 05h : Read Status Register-1 (S7-A0) */
    hsspi_read_command_sequence(reg_hsspi, m_awcCommandList_CMD_S, 0x05);
    unSR1.WORD = MEM_HSSPI_BYTE(mem_hsspi)[0];
    /* 35h : Read Status Register-2 (S15-A8) */
    hsspi_read_command_sequence(reg_hsspi, m_awcCommandList_CMD_S, 0x35);
    unSR2.WORD = MEM_HSSPI_BYTE(mem_hsspi)[0];
    unSR2.bit.QE = 1; /* QUAD MODE ENTRY */
    /* 06h : WRITE ENABLE */
    hsspi_write_command_direct(reg_hsspi, mem_hsspi, 0x06);
    /* 01h : Write Status Register-2 (S7-A0, S15-A8) */
    hsspi_write_command_sequence(reg_hsspi, m_awcCommandList_CMD_S, 0x01);
    MEM_HSSPI_BYTE(mem_hsspi)[0] = unSR1.BYTE[0];
    MEM_HSSPI_BYTE(mem_hsspi)[1] = unSR2.BYTE[0];
    /* 05h : READ STATUS REGISTER */
    hsspi_wait_status_register_for_wip(reg_hsspi, mem_hsspi);
}

static void hsspi_enter_to_quad_for_macronix(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi)
{
    /* Status Register */
    union macronix_status_register {
        uint32_t WORD;
        uint8_t BYTE[4];
        struct {
            uint32_t WIP : 1; /* B[0]    */
            uint32_t WEL : 1; /* B[1]    */
            uint32_t BP : 4; /* B[5:2]  */
            uint32_t QE : 1; /* B[6] Quad Enable (0=Disable, 1=Enable)  */
            uint32_t SRWD : 1; /* B[7]    */
            uint32_t reserved1: 24; /* reserved */
        } bit;
    } unSR;

    /* 05h : Read Status Register (RDSR) */
    hsspi_read_command_sequence(reg_hsspi, m_awcCommandList_CMD_S, 0x05);
    unSR.WORD = MEM_HSSPI_BYTE(mem_hsspi)[0];
    unSR.bit.QE = 1; /* QUAD MODE ENTRY */
    /* 06h : WRITE ENABLE */
    hsspi_write_command_direct(reg_hsspi, mem_hsspi, 0x06);
    /* 01h : Write Status Register (WRSR) */
    hsspi_write_command_sequence(reg_hsspi, m_awcCommandList_CMD_S, 0x01);
    MEM_HSSPI_BYTE(mem_hsspi)[0] = unSR.BYTE[0];
    /* 05h : READ STATUS REGISTER */
    hsspi_wait_status_register_for_wip(reg_hsspi, mem_hsspi);
}

static int hsspi_enter_to_quad_by_jedec_id(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi,
    int nJedecMID)
{
    static const HSSPI_ST_INIT_t m_astcHsspiInitTable[] = {
        { HSSPI_EN_JEDEC_MID_SPANSION, hsspi_enter_to_quad_for_spansion },
        { HSSPI_EN_JEDEC_MID_MICRON, hsspi_enter_to_quad_for_micron },
        { HSSPI_EN_JEDEC_MID_MACRONIX, hsspi_enter_to_quad_for_macronix },
        { HSSPI_EN_JEDEC_MID_WINBOND, hsspi_enter_to_quad_for_winbond },
        { HSSPI_EN_JEDEC_MID_UNKNOWN, NULL }
    };
    const HSSPI_ST_INIT_t *pstcIndex;

    for (pstcIndex = m_astcHsspiInitTable;
         pstcIndex->MID != HSSPI_EN_JEDEC_MID_UNKNOWN;
         ++pstcIndex) {
        if (pstcIndex->MID == nJedecMID) {
            if (pstcIndex->FUNC)
                (*pstcIndex->FUNC)(reg_hsspi, mem_hsspi);
            break;
        }
    }

    return pstcIndex->MID;
}

static void hsspi_software_reset(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi)
{
    HSSPI_UN_CSCFG_t unCSCFG;

    /* Status Register */
    union general_status_register {
        uint32_t WORD;
        uint8_t BYTE[4];
        struct {
            uint32_t WIP : 1; /* B[0] Write in progress */
            uint32_t WEL : 1; /* B[1] Write enable latch */
            uint32_t BP : 3; /* B[4:2]    */
            uint32_t E_ERR : 1; /* B[5]    */
            uint32_t P_ERR : 1; /* B[6]    */
            uint32_t SRWD : 1; /* B[7]    */
            uint32_t reserved1 : 24; /* reserved */
        } bit;
    } unRDSR;

    unCSCFG.DATA = (*reg_hsspi).CSCFG.DATA;

    unCSCFG.bit.SRAM = HSSPI_EN_CSCFG_SRAM_RW; /* allow rea/write access */
    unCSCFG.bit.MBM = HSSPI_EN_CSCFG_MBM_SINGLE;

    (*reg_hsspi).CSCFG.DATA = unCSCFG.DATA;

    hsspi_read_command_sequence(reg_hsspi, m_awcCommandList_CMD_S, 0x05);

    do {
        /* Reset Enable */
        hsspi_write_command_direct(reg_hsspi, mem_hsspi, 0x66);

        /* Wait for SPI device ready */
        osDelay(1);

        /* Reset */
        hsspi_write_command_direct(reg_hsspi, mem_hsspi, 0x99);

        /* Wait for SPI device ready */
        osDelay(1);

        /* 05h : RDSR Read Status Register    */
        unRDSR.WORD = MEM_HSSPI_BYTE(mem_hsspi)[0];
    } while (unRDSR.bit.WIP);

    return;
}

void hsspi_command_switch(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi,
    volatile void *reg_boot_ctl,
    HSSPI_EN_MCTRL_CDSS_t clk_sel,
    int clk_div,
    int syncon,
    int use_hsspi_cs1_flag,
    HSSPI_EN_CSCFG_MSEL_t msel)
{
    HSSPI_UN_PCC_t unPCC;
    HSSPI_UN_CSCFG_t unCSCFG;

    int known_jedec_id;

    hsspi_stop(reg_hsspi, clk_sel, syncon);

    unCSCFG.DATA = (*reg_hsspi).CSCFG.DATA;
    unCSCFG.bit.MSEL = msel;
    unCSCFG.bit.SSEL0EN = 1;
    unCSCFG.bit.SSEL1EN = use_hsspi_cs1_flag;
    unCSCFG.bit.BOOTEN = 0;
    unCSCFG.bit.SRAM = HSSPI_EN_CSCFG_SRAM_RW; /* allow read/write access */
    unCSCFG.bit.MBM = HSSPI_EN_CSCFG_MBM_SINGLE;
    (*reg_hsspi).CSCFG.DATA = unCSCFG.DATA;
    (*reg_hsspi).CSITIME.DATA = 0x100; /* IDLE TIMER=0x100cycle */

    MEM_HSSPI_WORD(reg_boot_ctl)[0x008 >> 2] |= (1 << 0);
    hsspi_csen(reg_hsspi, clk_sel, syncon); /* HS-SPI IP start(CS MODE) */

    hsspi_software_reset(reg_hsspi, mem_hsspi);

    hsspi_read_jedec_id(reg_hsspi, mem_hsspi);

    FWK_LOG_INFO(
        "[HS-SPI] CS#0: Manufacturer ID:%02x, DeviceID:%02x%02x",
        m_abyJEDEC_ID[0],
        m_abyJEDEC_ID[1],
        m_abyJEDEC_ID[2]);

    known_jedec_id =
        hsspi_enter_to_quad_by_jedec_id(reg_hsspi, mem_hsspi, m_abyJEDEC_ID[0]);

    if (known_jedec_id < 0) {
        FWK_LOG_INFO(
            "[HS-SPI] Unknown manufacturer ID:%02x,"
            " default to Dual-Output-Fast-Read mode",
            m_abyJEDEC_ID[0]);

        hsspi_dual_output_fast_read(reg_hsspi);
        unCSCFG.DATA = (*reg_hsspi).CSCFG.DATA;
        unCSCFG.bit.SRAM = HSSPI_EN_CSCFG_SRAM_RO;
        unCSCFG.bit.MBM = HSSPI_EN_CSCFG_MBM_DUAL;
        (*reg_hsspi).CSCFG.DATA = unCSCFG.DATA;
    } else {
        FWK_LOG_INFO("[HS-SPI] Configuring Quad-Output-Fast-Read mode");

        hsspi_quad_output_fast_read(reg_hsspi);
        unCSCFG.DATA = (*reg_hsspi).CSCFG.DATA;
        unCSCFG.bit.SRAM = HSSPI_EN_CSCFG_SRAM_RO;
        unCSCFG.bit.MBM = HSSPI_EN_CSCFG_MBM_QUAD;
        (*reg_hsspi).CSCFG.DATA = unCSCFG.DATA;
    }

#ifdef CONFIG_SCB_USE_4BYTE_MODE
    /* Enter 4-byte mode */
    hsspi_write_command_direct(
        reg_hsspi, mem_hsspi, HSSPI_EN_STARTUP_COMMAND_WEN);
    hsspi_write_command_direct(
        reg_hsspi, mem_hsspi, HSSPI_EN_STARTUP_COMMAND_ENTER_4B);
#endif /* CONFIG_SCB_USE_4BYTE_MODE */

    hsspi_stop(reg_hsspi, clk_sel, syncon);

    unPCC.DATA = (*reg_hsspi).PCC[0].DATA; /* Read original Value*/
    if (clk_div == 1)
        unPCC.bit.CDRS = (clk_div - 1);
    else
        unPCC.bit.CDRS = (clk_div >> 1);

    (*reg_hsspi).PCC[0].DATA = unPCC.DATA;

    hsspi_csen(reg_hsspi, clk_sel, syncon);
}

void hsspi_set_window_size(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    int use_hsspi_cs1_flag,
    HSSPI_EN_CSCFG_MSEL_t msel)
{
    HSSPI_UN_CSCFG_t unCSCFG;

    unCSCFG.DATA = (*reg_hsspi).CSCFG.DATA;

    unCSCFG.bit.SRAM = 1;

    unCSCFG.bit.MSEL = msel;
    unCSCFG.bit.SSEL0EN = 1;
    unCSCFG.bit.SSEL1EN = use_hsspi_cs1_flag;
    (*reg_hsspi).CSCFG.DATA = unCSCFG.DATA;
}

void hsspi_init(volatile REG_ST_HSSPI_t *reg_hsspi)
{
    HSSPI_UN_CSCFG_t unCSCFG;

    /* Stop HSSPI */
    (*reg_hsspi).MCTRL.bit.MEN = 0;
    do {
        __DSB();
    } while ((*reg_hsspi).MCTRL.bit.MES);

    (*reg_hsspi).MCTRL.bit.CSEN = 1;

    unCSCFG.DATA = (*reg_hsspi).CSCFG.DATA;
    unCSCFG.bit.SRAM = 1;
    unCSCFG.bit.MBM = 0;
    unCSCFG.bit.BOOTEN = 0;
    unCSCFG.bit.SSEL0EN = 1;
    unCSCFG.bit.SSEL1EN = 0;
    unCSCFG.bit.SSEL2EN = 0;
    unCSCFG.bit.SSEL3EN = 0;
    unCSCFG.bit.MSEL = HSSPI_EN_CSCFG_MSEL_256M;

    (*reg_hsspi).CSCFG.DATA = unCSCFG.DATA;

    (*reg_hsspi).MCTRL.bit.MEN = 1;
}

void hsspi_exit(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi,
    volatile void *reg_boot_ctl,
    HSSPI_EN_MCTRL_CDSS_t clk_sel,
    int clk_div,
    int syncon,
    int use_hsspi_cs1_flag,
    HSSPI_EN_CSCFG_MSEL_t msel)
{
    HSSPI_UN_PCC_t unPCC;
    HSSPI_UN_CSCFG_t unCSCFG;

    hsspi_stop(reg_hsspi, clk_sel, syncon);

    unCSCFG.DATA = (*reg_hsspi).CSCFG.DATA;
    unCSCFG.bit.MSEL = msel;
    unCSCFG.bit.SSEL0EN = 1;
    unCSCFG.bit.SSEL1EN = use_hsspi_cs1_flag;
    unCSCFG.bit.BOOTEN = 0;
    unCSCFG.bit.SRAM = HSSPI_EN_CSCFG_SRAM_RW;
    unCSCFG.bit.MBM = HSSPI_EN_CSCFG_MBM_SINGLE;
    (*reg_hsspi).CSCFG.DATA = unCSCFG.DATA;
    (*reg_hsspi).CSITIME.DATA = 0x100;

    MEM_HSSPI_WORD(reg_boot_ctl)[0x008 >> 2] |= (1 << 0);
    hsspi_csen(reg_hsspi, clk_sel, syncon);

    hsspi_software_reset(reg_hsspi, mem_hsspi);

    hsspi_stop(reg_hsspi, clk_sel, syncon);

    unPCC.DATA = (*reg_hsspi).PCC[0].DATA; /* Read original Value*/

    if (clk_div == 1)
        unPCC.bit.CDRS = (clk_div - 1);
    else
        unPCC.bit.CDRS = (clk_div >> 1);

    (*reg_hsspi).PCC[0].DATA = unPCC.DATA;

    hsspi_csen(reg_hsspi, clk_sel, syncon);

    /* set FAST_READ mode bit to run in FAST_READ after reboot */
    MEM_HSSPI_WORD(reg_boot_ctl)[0x008 >> 2] |= (0x1U << 1);

    MEM_HSSPI_WORD(reg_boot_ctl)[0x008 >> 2] &= ~(0x1U << 0);
}

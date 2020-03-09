/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERNAL_HSSPI_DRIVER_H
#define INTERNAL_HSSPI_DRIVER_H

#include "synquacer_mmap.h"

#include <internal/reg_HSSPI.h>

#include <stdint.h>

#define MEM_HSSPI_BYTE(var) ((volatile uint8_t *)(var))
#define MEM_HSSPI_HALF(var) ((volatile uint16_t *)(var))
#define MEM_HSSPI_WORD(var) ((volatile uint32_t *)(var))

typedef enum {
    HSSPI_EN_STARTUP_COMMAND_READ_ID = 0x9F, /* READ ID */
    HSSPI_EN_STARTUP_COMMAND_READ = 0x03, /* READ */
    HSSPI_EN_STARTUP_COMMAND_DOFR = 0x3B, /* DUAL OUTPUT FAST READ */
    HSSPI_EN_STARTUP_COMMAND_QOFR = 0x6B, /* QUAD OUTPUT FAST READ */
    HSSPI_EN_STARTUP_COMMAND_READ_4B = 0x13, /* 4-BYTE READ */
    HSSPI_EN_STARTUP_COMMAND_DOFR_4B = 0x3C, /* 4-BYTE DUAL OUTPUT FAST READ */
    HSSPI_EN_STARTUP_COMMAND_QOFR_4B = 0x6C, /* 4-BYTE QUAD OUTPUT FAST READ */
    HSSPI_EN_STARTUP_COMMAND_WEN = 0x06, /* WRITE ENABLE */
    HSSPI_EN_STARTUP_COMMAND_ENTER_4B = 0xB7, /* ENTER 4-BYTE MODE */
} HSSPI_EN_STARTUP_COMMAND_t;

typedef enum {
    HSSPI_EN_JEDEC_MID_UNKNOWN = -1,
    HSSPI_EN_JEDEC_MID_SPANSION = 0x01,
    HSSPI_EN_JEDEC_MID_MICRON = 0x20,
    HSSPI_EN_JEDEC_MID_MACRONIX = 0xC2,
    HSSPI_EN_JEDEC_MID_WINBOND = 0xEF
} HSSPI_EN_JEDEC_MID_t;

typedef void (*HSSPI_FUNC_INIT)(volatile REG_ST_HSSPI_t *, volatile void *);

typedef struct {
    HSSPI_EN_JEDEC_MID_t MID;
    HSSPI_FUNC_INIT FUNC;
} HSSPI_ST_INIT_t;

void hsspi_command_switch(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi,
    volatile void *reg_boot_ctl,
    HSSPI_EN_MCTRL_CDSS_t clk_sel,
    int clk_div,
    int syncon,
    int use_hsspi_cs1_flag,
    HSSPI_EN_CSCFG_MSEL_t msel);

void hsspi_exit(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi,
    volatile void *reg_boot_ctl,
    HSSPI_EN_MCTRL_CDSS_t clk_sel,
    int clk_div,
    int syncon,
    int use_hsspi_cs1_flag,
    HSSPI_EN_CSCFG_MSEL_t msel);

void hsspi_write_command_direct(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi,
    int command);
unsigned char hsspi_read_command_direct(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    volatile void *mem_hsspi,
    int command);

void hsspi_write_command_sequence_addr2(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    int command);
void hsspi_read_command_sequence_addr2(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    int command);

void hsspi_set_window_size(
    volatile REG_ST_HSSPI_t *reg_hsspi,
    int use_hsspi_cs1_flag,
    HSSPI_EN_CSCFG_MSEL_t msel);

void hsspi_init(volatile REG_ST_HSSPI_t *reg_hsspi);

#endif /* INTERNAL_HSSPI_DRIVER_H */

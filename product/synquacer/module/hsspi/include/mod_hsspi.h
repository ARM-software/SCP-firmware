/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_HSSPI_H
#define MOD_HSSPI_H

#include <fwk_id.h>

#include <stdint.h>
/*!
 * \addtogroup GroupModule  Product Modules
 * \{
 */

/*!
 * \defgroup GroupHSSPI HSSPI Driver
 *
 * \brief  High Speed SPI device driver.
 *
 * \details This module implements a device driver for the HSSPI
 *
 * \{
 */
enum hsspi_mode {
    HSSPI_MODE_DIRECT = 0,
    HSSPI_MODE_COMMAND_SEQUENCE,
    HSSPI_MODE_COUNT
};

enum hsspi_clock_select {
    HSSPI_CLOCK_SELECT_HSEL = 0,
    HSSPI_CLOCK_SELECT_PSEL,
    HSSPI_CLOCK_SELECT_COUNT
};

enum hsspi_deselect_time {
    HSSPI_DESELECT_TIME_1 = 0,
    HSSPI_DESELECT_TIME_2,
    HSSPI_DESELECT_TIME_3,
    HSSPI_DESELECT_TIME_4,
    HSSPI_DESELECT_TIME_5,
    HSSPI_DESELECT_TIME_6,
    HSSPI_DESELECT_TIME_7,
    HSSPI_DESELECT_TIME_8,
    HSSPI_DESELECT_TIME_9,
    HSSPI_DESELECT_TIME_10,
    HSSPI_DESELECT_TIME_11,
    HSSPI_DESELECT_TIME_12,
    HSSPI_DESELECT_TIME_13,
    HSSPI_DESELECT_TIME_14,
    HSSPI_DESELECT_TIME_15,
    HSSPI_DESELECT_TIME_16,
    HSSPI_DESELECT_TIME_COUNT
};

enum hsspi_memory_bank_size {
    HSSPI_MEMORY_BANK_SIZE_8K = 0,
    HSSPI_MEMORY_BANK_SIZE_16K,
    HSSPI_MEMORY_BANK_SIZE_32K,
    HSSPI_MEMORY_BANK_SIZE_64K,
    HSSPI_MEMORY_BANK_SIZE_128K,
    HSSPI_MEMORY_BANK_SIZE_256K,
    HSSPI_MEMORY_BANK_SIZE_512K,
    HSSPI_MEMORY_BANK_SIZE_1M,
    HSSPI_MEMORY_BANK_SIZE_2M,
    HSSPI_MEMORY_BANK_SIZE_4M,
    HSSPI_MEMORY_BANK_SIZE_8M,
    HSSPI_MEMORY_BANK_SIZE_16M,
    HSSPI_MEMORY_BANK_SIZE_32M,
    HSSPI_MEMORY_BANK_SIZE_64M,
    HSSPI_MEMORY_BANK_SIZE_128M,
    HSSPI_MEMORY_BANK_SIZE_256M,
    HSSPI_MEMORY_BANK_SIZE_COUNT,
};

/*!
 * \brief HSSPI device configuration data.
 */
struct mod_hsspi_dev_config {
    /*! Base address of the device registers */
    const uintptr_t reg_base;

    /*! Base address of the memory mapped area */
    const uintptr_t memory_base;

    /*! Division of clock */
    uint8_t clock_div;

    /*! Selection of clock source */
    enum hsspi_clock_select clock_sel;

    /*! Chip deselection time */
    enum hsspi_deselect_time deselect_time;

    /*! Bank size of memory area */
    enum hsspi_memory_bank_size memory_bank_size;
};

/*!
 * \brief HSSPI configuration data.
 */
struct mod_hsspi_config {
    /*! Base address of the boot_ctl reigster */
    const uintptr_t bootctl_base;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_HSSPI_H */

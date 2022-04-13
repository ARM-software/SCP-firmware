/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Winbond (W25Q256JV) definitions.
 *     https://cdn.sparkfun.com/assets/c/2/9/2/6/W25Q256JV.pdf
 */

#ifndef DEVICE_NOR_W25_H
#define DEVICE_NOR_W25_H

#include "qspi_api.h"

#define PROGRAM_PAGE_SIZE (0x1U << 8) /* 256 byte */
#define ERASE_BLOCK_SIZE  (0x1U << 16) /* 64KiB */

/*
 * Supports only 1bit I/O for instruction phase.
 * Therefore, it should not be implemented a command
 * which has 2bit or 4bit I/O for instruction phase.
 */

/* common command is referred by config_nor.c */
#define COMMAND_RESET_ENABLE     QSPI_COMMAND_TYPE_CODE(0x66)
#define COMMAND_RESET_MEMORY     QSPI_COMMAND_TYPE_CODE(0x99)
#define COMMAND_READ_ID          QSPI_COMMAND_TYPE_CODE_DATA(0x9F)
#define COMMAND_READ_SFDP        QSPI_COMMAND_TYPE_READ(0x5A, 3, 8, 1, 1, 1)
#define COMMAND_WRITE_ENABLE     QSPI_COMMAND_TYPE_CODE(0x06)
#define COMMAND_WRITE_DISABLE    QSPI_COMMAND_TYPE_CODE(0x04)
#define COMMAND_READ_STATUS_REG  QSPI_COMMAND_TYPE_CODE_DATA(0x05)
#define COMMAND_WRITE_STATUS_REG QSPI_COMMAND_TYPE_CODE_DATA(0x01)
#define COMMAND_READ             QSPI_COMMAND_TYPE_READ(0x03, 3, 0, 1, 1, 1)
#define COMMAND_FAST_READ        QSPI_COMMAND_TYPE_READ(0x0B, 3, 8, 1, 1, 1)
#define COMMAND_FAST_READ_1_1_2  QSPI_COMMAND_TYPE_READ(0x3B, 3, 8, 1, 1, 2)
#define COMMAND_FAST_READ_1_2_2 \
    QSPI_COMMAND_TYPE_READ_WITH_ALT_FF( \
        0xBB, 3, 1, 0, 1, 2, 2) /* M7-M0 should be 0xFF */
#define COMMAND_FAST_READ_1_1_4 QSPI_COMMAND_TYPE_READ(0x6B, 3, 8, 1, 1, 4)
#define COMMAND_FAST_READ_1_4_4 \
    QSPI_COMMAND_TYPE_READ_WITH_ALT_FF( \
        0xEB, 3, 1, 4, 1, 4, 4) /* M7-M0 should be 0xFF */
#define COMMAND_READ_4B            QSPI_COMMAND_TYPE_READ(0x13, 4, 0, 1, 1, 1)
#define COMMAND_FAST_READ_4B       QSPI_COMMAND_TYPE_READ(0x0C, 4, 8, 1, 1, 1)
#define COMMAND_FAST_READ_1_1_2_4B QSPI_COMMAND_TYPE_READ(0x3C, 4, 8, 1, 1, 2)
#define COMMAND_FAST_READ_1_2_2_4B \
    QSPI_COMMAND_TYPE_READ_WITH_ALT_FF( \
        0xBC, 4, 1, 0, 1, 2, 2) /* M7-M0 should be 0xFF */
#define COMMAND_FAST_READ_1_1_4_4B QSPI_COMMAND_TYPE_READ(0x6C, 4, 8, 1, 1, 4)
#define COMMAND_FAST_READ_1_4_4_4B \
    QSPI_COMMAND_TYPE_READ_WITH_ALT_FF( \
        0xEC, 4, 1, 4, 1, 4, 4) /* M7-M0 should be 0xFF */
#define COMMAND_PROGRAM          QSPI_COMMAND_TYPE_WRITE(0x02, 3, 1, 1, 1)
#define COMMAND_PROGRAM_1_1_2    QSPI_COMMAND_EMPTY
#define COMMAND_PROGRAM_1_2_2    QSPI_COMMAND_EMPTY
#define COMMAND_PROGRAM_1_1_4    QSPI_COMMAND_TYPE_WRITE(0x32, 3, 1, 1, 4)
#define COMMAND_PROGRAM_1_4_4    QSPI_COMMAND_EMPTY
#define COMMAND_PROGRAM_4B       QSPI_COMMAND_TYPE_WRITE(0x12, 4, 1, 1, 1)
#define COMMAND_PROGRAM_1_1_4_4B QSPI_COMMAND_TYPE_WRITE(0x34, 4, 1, 1, 4)
#define COMMAND_PROGRAM_1_4_4_4B QSPI_COMMAND_EMPTY
#define COMMAND_ERASE_4KB        QSPI_COMMAND_TYPE_WRITE_ADDR(0x20, 3, 1, 1, 1)
#define COMMAND_ERASE_32KB       QSPI_COMMAND_TYPE_WRITE_ADDR(0x52, 3, 1, 1, 1)
#define COMMAND_ERASE_BLOCK      QSPI_COMMAND_TYPE_WRITE_ADDR(0xD8, 3, 1, 1, 1)
#define COMMAND_ERASE_CHIP       QSPI_COMMAND_TYPE_CODE(0x60)
#define COMMAND_ERASE_4KB_4B     QSPI_COMMAND_TYPE_WRITE_ADDR(0x21, 4, 1, 1, 1)
#define COMMAND_ERASE_32KB_4B    QSPI_COMMAND_EMPTY
#define COMMAND_ERASE_BLOCK_4B   QSPI_COMMAND_TYPE_WRITE_ADDR(0xDC, 4, 1, 1, 1)

/* Winbond specific command */
#define W25_COMMAND_ENTER_4BYTE        QSPI_COMMAND_TYPE_CODE(0xB7)
#define W25_COMMAND_EXIT_4BYTE         QSPI_COMMAND_TYPE_CODE(0xE9)
#define W25_COMMAND_READ_STATUS_REG_2  QSPI_COMMAND_TYPE_CODE_DATA(0x35)
#define W25_COMMAND_READ_STATUS_REG_3  QSPI_COMMAND_TYPE_CODE_DATA(0x15)
#define W25_COMMAND_WRITE_STATUS_REG_2 QSPI_COMMAND_TYPE_CODE_DATA(0x31)
#define W25_COMMAND_WRITE_STATUS_REG_3 QSPI_COMMAND_TYPE_CODE_DATA(0x11)

/* Status Register */
#define W25_WEL_ENABLE            (0x1 << 1)
#define W25_IS_WEL_ENABLE(status) (((status)&W25_WEL_ENABLE) != 0)

/* Status Register 2 */
#define W25_QE_BIT              (0x1 << 2)
#define W25_QUAD_DISABLE(val)   ((val) &= ~W25_QE_BIT)
#define W25_QUAD_ENABLE(val)    ((val) |= W25_QE_BIT)
#define W25_IS_QUAD_ENABLE(val) (((val)&W25_QE_BIT) != 0)

int w25_nor_set_io_protocol(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg);
int w25_nor_set_4byte_addr_mode(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg);

// alias for configuration data
#define nor_set_io_protocol     w25_nor_set_io_protocol
#define nor_set_4byte_addr_mode w25_nor_set_4byte_addr_mode
#define nor_get_program_result  NULL
#define nor_get_erase_result    NULL

#endif /* DEVICE_NOR_W25_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Micron(MT25QL128ABA) definitions.
 *     https://media-www.micron.com/-/media/client/global/documents/products/data-sheet/nor-flash/serial-nor/mt25q/die-rev-a/mt25q_qlhs_l_128_aba_0.pdf?rev=8abb836059694966b54cf068a1803757
 */

#ifndef DEVICE_NOR_MT25_H
#define DEVICE_NOR_MT25_H

#include "qspi_api.h"

#define PROGRAM_PAGE_SIZE (0x1U << 8) /* 256 byte */
#define ERASE_BLOCK_SIZE  (0x1U << 16) /* 64KiB */

/*
 * Supports only 1bit I/O for instruction phase.
 * Therefore, it should not be implemented a command
 * which has 2bit or 4bit I/O for instruction phase.
 */

/* common command is referred by config_nor.c */
#define COMMAND_RESET_ENABLE       QSPI_COMMAND_TYPE_CODE(0x66)
#define COMMAND_RESET_MEMORY       QSPI_COMMAND_TYPE_CODE(0x99)
#define COMMAND_READ_ID            QSPI_COMMAND_TYPE_CODE_DATA(0x9F)
#define COMMAND_READ_SFDP          QSPI_COMMAND_TYPE_READ(0x5A, 3, 8, 1, 1, 1)
#define COMMAND_WRITE_ENABLE       QSPI_COMMAND_TYPE_CODE(0x06)
#define COMMAND_WRITE_DISABLE      QSPI_COMMAND_TYPE_CODE(0x04)
#define COMMAND_READ_STATUS_REG    QSPI_COMMAND_TYPE_CODE_DATA(0x05)
#define COMMAND_WRITE_STATUS_REG   QSPI_COMMAND_TYPE_CODE_DATA(0x01)
#define COMMAND_READ               QSPI_COMMAND_TYPE_READ(0x03, 3, 0, 1, 1, 1)
#define COMMAND_FAST_READ          QSPI_COMMAND_TYPE_READ(0x0B, 3, 8, 1, 1, 1)
#define COMMAND_FAST_READ_1_1_2    QSPI_COMMAND_TYPE_READ(0x3B, 3, 8, 1, 1, 2)
#define COMMAND_FAST_READ_1_2_2    QSPI_COMMAND_TYPE_READ(0xBB, 3, 8, 1, 2, 2)
#define COMMAND_FAST_READ_1_1_4    QSPI_COMMAND_TYPE_READ(0x6B, 3, 8, 1, 1, 4)
#define COMMAND_FAST_READ_1_4_4    QSPI_COMMAND_TYPE_READ(0xEB, 3, 10, 1, 4, 4)
#define COMMAND_READ_4B            QSPI_COMMAND_EMPTY
#define COMMAND_FAST_READ_4B       QSPI_COMMAND_EMPTY
#define COMMAND_FAST_READ_1_1_2_4B QSPI_COMMAND_EMPTY
#define COMMAND_FAST_READ_1_2_2_4B QSPI_COMMAND_EMPTY
#define COMMAND_FAST_READ_1_1_4_4B QSPI_COMMAND_EMPTY
#define COMMAND_FAST_READ_1_4_4_4B QSPI_COMMAND_EMPTY
#define COMMAND_PROGRAM            QSPI_COMMAND_TYPE_WRITE(0x02, 3, 1, 1, 1)
#define COMMAND_PROGRAM_1_1_2      QSPI_COMMAND_TYPE_WRITE(0xA2, 3, 1, 1, 2)
#define COMMAND_PROGRAM_1_2_2      QSPI_COMMAND_TYPE_WRITE(0xD2, 3, 1, 2, 2)
#define COMMAND_PROGRAM_1_1_4      QSPI_COMMAND_TYPE_WRITE(0x32, 3, 1, 1, 4)
#define COMMAND_PROGRAM_1_4_4      QSPI_COMMAND_TYPE_WRITE(0x38, 3, 1, 4, 4)
#define COMMAND_PROGRAM_4B         QSPI_COMMAND_EMPTY
#define COMMAND_PROGRAM_1_1_4_4B   QSPI_COMMAND_EMPTY
#define COMMAND_PROGRAM_1_4_4_4B   QSPI_COMMAND_EMPTY
#define COMMAND_ERASE_4KB          QSPI_COMMAND_TYPE_WRITE_ADDR(0x20, 3, 1, 1, 1)
#define COMMAND_ERASE_32KB         QSPI_COMMAND_TYPE_WRITE_ADDR(0x52, 3, 1, 1, 1)
#define COMMAND_ERASE_BLOCK        QSPI_COMMAND_TYPE_WRITE_ADDR(0xD8, 3, 1, 1, 1)
#define COMMAND_ERASE_CHIP         QSPI_COMMAND_TYPE_CODE(0x60)
#define COMMAND_ERASE_4KB_4B       QSPI_COMMAND_EMPTY
#define COMMAND_ERASE_32KB_4B      QSPI_COMMAND_EMPTY
#define COMMAND_ERASE_BLOCK_4B     QSPI_COMMAND_EMPTY

/* Micron specific command */
#define MT25_COMMAND_READ_FLAG_STATUS_REG QSPI_COMMAND_TYPE_CODE_DATA(0x70)
#define MT25_COMMAND_READ_NV_CONFIG_REG   QSPI_COMMAND_TYPE_CODE_DATA(0xB5)
#define MT25_COMMAND_WRITE_NV_CONFIG_REG  QSPI_COMMAND_TYPE_CODE_DATA(0xB1)

/* Status Register */
#define MT25_WEL_ENABLE            (0x1 << 1)
#define MT25_IS_WEL_ENABLE(status) (((status)&MT25_WEL_ENABLE) != 0)

/* Enhanced Configuration Register */
#define MT25_RESETHOLD_BIT       (0x1 << 4)
#define MT25_QUAD_DISABLE(val)   ((val) |= MT25_RESETHOLD_BIT)
#define MT25_QUAD_ENABLE(val)    ((val) &= ~MT25_RESETHOLD_BIT) // 0 is enable
#define MT25_IS_QUAD_ENABLE(val) (((val)&MT25_RESETHOLD_BIT) == 0)

/* Flag Status Register */
#define MT25_E_FAIL             (0x1 << 5)
#define MT25_IS_ERASE_FAIL(sec) (((sec)&MT25_E_FAIL) != 0)

#define MT25_P_FAIL               (0x1 << 4)
#define MT25_IS_PROGRAM_FAIL(sec) (((sec)&MT25_P_FAIL) != 0)

int mt25_nor_set_io_protocol(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg);
int mt25_nor_get_program_result(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg);
int mt25_nor_get_erase_result(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg);

/* aliase to refer nor_config.c */
#define nor_set_io_protocol     mt25_nor_set_io_protocol
#define nor_set_4byte_addr_mode NULL
#define nor_get_program_result  mt25_nor_get_program_result
#define nor_get_erase_result    mt25_nor_get_erase_result

#endif /* DEVICE_NOR_MT25_H */

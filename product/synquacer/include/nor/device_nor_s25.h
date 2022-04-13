/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Infineon(S25FL512S) definitions.
 *     https://www.infineon.com/dgdl/Infineon-S25FL512S_Military_512Mbit64_Mbyte_3.0V_SPI_Flash_Memory-DataSheet-v19_00-EN.pdf?fileId=8ac78c8c7d0d8da4017d0ed046ae4b53
 */

#ifndef DEVICE_NOR_S25_H
#define DEVICE_NOR_S25_H

#include "qspi_api.h"

#define PROGRAM_PAGE_SIZE (0x1U << 9) /* 512 byte */
#define ERASE_BLOCK_SIZE  (0x1U << 18) /* 256KiB */

/*
 * Supports only 1bit I/O for instruction phase.
 * Therefore, it should not be implemented a command
 * which has 2bit or 4bit I/O for instruction phase.
 */

/* common command is referred by config_nor.c */
#define COMMAND_RESET_ENABLE     QSPI_COMMAND_TYPE_CODE(0xFF)
#define COMMAND_RESET_MEMORY     QSPI_COMMAND_TYPE_CODE(0xF0)
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
    QSPI_COMMAND_TYPE_READ_WITH_ALT_00( \
        0xBB, 3, 1, 0, 1, 2, 2) /* M7-M0 is 0x00 and no dummy cycle */
#define COMMAND_FAST_READ_1_1_4 QSPI_COMMAND_TYPE_READ(0x6B, 3, 8, 1, 1, 4)
#define COMMAND_FAST_READ_1_4_4 \
    QSPI_COMMAND_TYPE_READ_WITH_ALT_00(0xEB, 3, 1, 4, 1, 4, 4) /* M7-M0 is 0x00 */
#define COMMAND_READ_4B            QSPI_COMMAND_TYPE_READ(0x13, 4, 0, 1, 1, 1)
#define COMMAND_FAST_READ_4B       QSPI_COMMAND_TYPE_READ(0x0C, 4, 8, 1, 1, 1)
#define COMMAND_FAST_READ_1_1_2_4B QSPI_COMMAND_TYPE_READ(0x3C, 4, 8, 1, 1, 2)
#define COMMAND_FAST_READ_1_2_2_4B \
    QSPI_COMMAND_TYPE_READ_WITH_ALT_00( \
        0xBC, 4, 1, 0, 1, 2, 2) /* M7-M0 is 0x00 and no dummy cycle */
#define COMMAND_FAST_READ_1_1_4_4B QSPI_COMMAND_TYPE_READ(0x6C, 4, 8, 1, 1, 4)
#define COMMAND_FAST_READ_1_4_4_4B \
    QSPI_COMMAND_TYPE_READ_WITH_ALT_00(0xEC, 4, 1, 4, 1, 4, 4) /* M7-M0 is 0x00 */
#define COMMAND_PROGRAM          QSPI_COMMAND_TYPE_WRITE(0x02, 3, 1, 1, 1)
#define COMMAND_PROGRAM_1_1_2    QSPI_COMMAND_EMPTY
#define COMMAND_PROGRAM_1_2_2    QSPI_COMMAND_EMPTY
#define COMMAND_PROGRAM_1_1_4    QSPI_COMMAND_TYPE_WRITE(0x32, 3, 1, 1, 4)
#define COMMAND_PROGRAM_1_4_4    QSPI_COMMAND_EMPTY
#define COMMAND_PROGRAM_4B       QSPI_COMMAND_TYPE_WRITE(0x12, 4, 1, 1, 1)
#define COMMAND_PROGRAM_1_1_4_4B QSPI_COMMAND_TYPE_WRITE(0x34, 4, 1, 1, 4)
#define COMMAND_PROGRAM_1_4_4_4B QSPI_COMMAND_EMPTY
#define COMMAND_ERASE_4KB        QSPI_COMMAND_EMPTY
#define COMMAND_ERASE_32KB       QSPI_COMMAND_EMPTY
#define COMMAND_ERASE_BLOCK      QSPI_COMMAND_TYPE_WRITE_ADDR(0xD8, 3, 1, 1, 1)
#define COMMAND_ERASE_CHIP       QSPI_COMMAND_TYPE_CODE(0x60)
#define COMMAND_ERASE_4KB_4B     QSPI_COMMAND_EMPTY
#define COMMAND_ERASE_32KB_4B    QSPI_COMMAND_EMPTY
#define COMMAND_ERASE_BLOCK_4B   QSPI_COMMAND_TYPE_WRITE_ADDR(0xDC, 4, 1, 1, 1)

/* S25 specific command */
#define S25_COMMAND_READ_CONFIG_REG  QSPI_COMMAND_TYPE_CODE_DATA(0x35)
#define S25_COMMAND_READ_BANK_REG    QSPI_COMMAND_TYPE_CODE_DATA(0x16)
#define S25_COMMAND_WRITE_CONFIG_REG QSPI_COMMAND_TYPE_CODE_DATA(0x01)
#define S25_COMMAND_WRITE_BANK_REG   QSPI_COMMAND_TYPE_CODE_DATA(0x17)

/* Status Register */
#define S25_P_FAIL               (0x1 << 6)
#define S25_IS_PROGRAM_FAIL(sec) (((sec)&S25_P_FAIL) != 0)
#define S25_E_FAIL               (0x1 << 5)
#define S25_IS_ERASE_FAIL(sec)   (((sec)&S25_E_FAIL) != 0)

#define S25_WEL_ENABLE            (0x1 << 1)
#define S25_IS_WEL_ENABLE(status) (((status)&S25_WEL_ENABLE) != 0)

/* Configuration Register */
#define S25_QUAD_BIT            (0x1 << 1)
#define S25_QUAD_DISABLE(val)   ((val) &= ~S25_QUAD_BIT)
#define S25_QUAD_ENABLE(val)    ((val) |= S25_QUAD_BIT)
#define S25_IS_QUAD_ENABLE(val) (((val)&S25_QUAD_BIT) != 0)

/* Bank Register */
#define S25_EXTADD_BIT           (0x1 << 7)
#define S25_4BYTE_DISABLE(val)   ((val) &= ~S25_EXTADD_BIT)
#define S25_4BYTE_ENABLE(val)    ((val) |= S25_EXTADD_BIT)
#define S25_IS_4BYTE_ENABLE(val) (((val)&S25_EXTADD_BIT) != 0)

int s25_nor_set_io_protocol(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg);
int s25_nor_set_4byte_addr_mode(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg);
int s25_nor_get_program_result(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg);
int s25_nor_get_erase_result(
    fwk_id_t id,
    const struct qspi_api *qspi_api,
    void *arg);

/* aliase to refer nor_config.c */
#define nor_set_io_protocol     s25_nor_set_io_protocol
#define nor_set_4byte_addr_mode s25_nor_set_4byte_addr_mode
#define nor_get_program_result  s25_nor_get_program_result
#define nor_get_erase_result    s25_nor_get_erase_result

#endif /* DEVICE_NOR_S25_H */

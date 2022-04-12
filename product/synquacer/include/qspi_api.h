/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QSPI_API_H
#define QSPI_API_H

#include <fwk_id.h>

#include <stdbool.h>
#include <stdint.h>

/*
 * \brief qspi command structure
 */
struct qspi_command {
    uint8_t code;
    struct len {
        uint8_t code_byte;
        uint8_t addr_byte;
        uint8_t alt_byte;
        uint8_t dummy_cycle;
        uint8_t data_byte;
    } len;
    struct alt {
        uint8_t data[4];
        bool is_nibble;
    } alt;
    struct io {
        uint8_t code_bit;
        uint8_t addr_bit;
        uint8_t data_bit;
    } io;
};

/*
 * \brief qspi command generator macro definitions
 */
#define QSPI_GEN_COMMAND( \
    CODE, \
    CODE_BYTE, \
    ADDR_BYTE, \
    ALT_BYTE, \
    DUMMY, \
    DATA_BYTE, \
    ALT_0, \
    ALT_1, \
    ALT_2, \
    ALT_3, \
    NIBBLE, \
    IO_CODE, \
    IO_ADDR, \
    IO_DATA) \
    { \
        .code = CODE,                 \
        .len = {                      \
            .code_byte = CODE_BYTE,   \
            .addr_byte = ADDR_BYTE,   \
            .alt_byte = ALT_BYTE,     \
            .dummy_cycle = DUMMY,     \
            .data_byte = DATA_BYTE,   \
        },                            \
        .alt = {                      \
            .data = {                 \
                ALT_0,                \
                ALT_1,                \
                ALT_2,                \
                ALT_3                 \
            },                        \
            .is_nibble = NIBBLE,      \
        },                            \
        .io = {                       \
            .code_bit = IO_CODE,      \
            .addr_bit = IO_ADDR,      \
            .data_bit = IO_DATA,      \
        }, \
    }
#define QSPI_COMMAND_EMPTY \
    ((struct qspi_command) \
        QSPI_GEN_COMMAND(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false, 0, 0, 0))
#define QSPI_COMMAND_TYPE_CODE(CODE) \
    ((struct qspi_command) \
        QSPI_GEN_COMMAND(CODE, 1, 0, 0, 0, 0, 0, 0, 0, 0, false, 1, 0, 1))
#define QSPI_COMMAND_TYPE_CODE_DATA(CODE) \
    ((struct qspi_command) \
        QSPI_GEN_COMMAND(CODE, 1, 0, 0, 0, 1, 0, 0, 0, 0, false, 1, 0, 1))

#define QSPI_COMMAND_TYPE_READ(CODE, ADDR, DUMMY, IO_CODE, IO_ADDR, IO_DATA) \
    ((struct qspi_command) QSPI_GEN_COMMAND( \
        CODE, \
        1, \
        ADDR, \
        0, \
        DUMMY, \
        1, \
        0, \
        0, \
        0, \
        0, \
        false, \
        IO_CODE, \
        IO_ADDR, \
        IO_DATA))
#define QSPI_COMMAND_TYPE_WRITE(CODE, ADDR, IO_CODE, IO_ADDR, IO_DATA) \
    ((struct qspi_command) QSPI_GEN_COMMAND( \
        CODE, 1, ADDR, 0, 0, 1, 0, 0, 0, 0, false, IO_CODE, IO_ADDR, IO_DATA))
#define QSPI_COMMAND_TYPE_WRITE_ADDR(CODE, ADDR, IO_CODE, IO_ADDR, IO_DATA) \
    ((struct qspi_command) QSPI_GEN_COMMAND( \
        CODE, 1, ADDR, 0, 0, 0, 0, 0, 0, 0, false, IO_CODE, IO_ADDR, IO_DATA))
#define QSPI_COMMAND_TYPE_READ_WITH_ALT_00( \
    CODE, ADDR, ALT, DUMMY, IO_CODE, IO_ADDR, IO_DATA) \
    ((struct qspi_command) QSPI_GEN_COMMAND( \
        CODE, \
        1, \
        ADDR, \
        ALT, \
        DUMMY, \
        1, \
        0, \
        0, \
        0, \
        0, \
        false, \
        IO_CODE, \
        IO_ADDR, \
        IO_DATA))
#define QSPI_COMMAND_TYPE_READ_WITH_ALT_FF( \
    CODE, ADDR, ALT, DUMMY, IO_CODE, IO_ADDR, IO_DATA) \
    ((struct qspi_command) QSPI_GEN_COMMAND( \
        CODE, \
        1, \
        ADDR, \
        ALT, \
        DUMMY, \
        1, \
        0xFF, \
        0xFF, \
        0xFF, \
        0xFF, \
        false, \
        IO_CODE, \
        IO_ADDR, \
        IO_DATA))

#define IS_QSPI_COMMAND_EMPTY(cmd) \
    (cmd.code == 0 && cmd.len.code_byte == 0 && cmd.len.addr_byte == 0 && \
     cmd.len.alt_byte == 0 && cmd.len.dummy_cycle == 0 && \
     cmd.len.data_byte == 0)

/*
 * \brief QSPI API indices
 */
enum qspi_api_type {
    QSPI_API_TYPE_DEFAULT = 0,
    QSPI_API_TYPE_COUNT,
};

/*!
 * \brief APIs to access the descriptors in the flash memory.
 */
struct qspi_api {
    /*!
     * \brief set read command to spi flash memory
     *
     * \param id The qspi device identifier.
     * \param command Read command to send spi flash memofy.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid command was encountered.
     * \retval ::FWK_E_STATE The qspi module isn't started yet.
     * \return One of the other specific error codes.
     */
    int (*set_read_command)(fwk_id_t id, struct qspi_command *command);

    /*!
     * \brief set write command to spi flash memory
     *
     * \param id The qspi device identifier.
     * \param command Write command to send spi flash memofy.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid command was encountered.
     * \retval ::FWK_E_STATE The qspi module isn't started yet.
     * \return One of the other specific error codes.
     */
    int (*set_write_command)(fwk_id_t id, struct qspi_command *command);

    /*!
     * \brief read data from spi flash memory
     *
     * \param id The qspi device identifier.
     * \param offset Offset address of spi flash memory to read.
     * \param buf The pointer address to store the data.
     * \param len Number of data to read.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered.
     * \retval ::FWK_E_STATE The qspi module isn't started yet.
     * \return One of the other specific error codes.
     */
    int (*read)(fwk_id_t id, uint32_t offset, void *buf, uint32_t len);

    /*!
     * \brief write data to spi flash memory
     *
     * \param id The qspi device identifier.
     * \param offset Offset address of spi flash memory to write.
     * \param buf The pointer address to store the data.
     * \param len Number of data to write.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered.
     * \retval ::FWK_E_STATE The qspi module isn't started yet.
     * \return One of the other specific error codes.
     */
    int (*write)(fwk_id_t id, uint32_t offset, void *buf, uint32_t len);

    /*!
     * \brief erase data on the spi flash memory
     *
     * \param id The qspi device identifier.
     * \param offset Offset address of spi flash memory to write.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_STATE The qspi module isn't started yet.
     * \return One of the other specific error codes.
     */
    int (*erase)(fwk_id_t id, uint32_t offset);

    /*!
     * \brief reset the device and set hsspi controller to the command sequence
     * mode
     *
     * \param id The qspi device identifier.
     *
     * \return never return.
     */
    int (*init_csmode)(fwk_id_t id);
};

/*
 * \brief function pointer for command operation
 */
typedef int (*qspi_ope_func_t)(fwk_id_t, const struct qspi_api *, void *);

#endif /* QSPI_API_H */

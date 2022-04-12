/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_NOR_H
#define MOD_NOR_H

#include "qspi_api.h"

#include <fwk_id.h>

#include <stdint.h>
/*!
 * \addtogroup GroupModule Product Modules
 * \{
 */

/*!
 * \defgroup GroupNOR NOR Driver
 *
 * \brief NOR device driver.
 *
 * \details This module implements a device driver for the NOR
 *
 * \{
 */

/*
 * \brief NOR API indices
 */
enum mod_nor_api_type {
    MOD_NOR_API_TYPE_DEFAULT = 0,
    MOD_NOR_API_TYPE_COUNT,
};

/*
 * \brief NOR read mode indices
 */
enum mod_nor_read_mode {
    /* Read Operation */
    MOD_NOR_READ = 0,
    MOD_NOR_READ_FAST,
    MOD_NOR_READ_FAST_1_1_2,
    MOD_NOR_READ_FAST_1_2_2,
    MOD_NOR_READ_FAST_1_1_4,
    MOD_NOR_READ_FAST_1_4_4,
    MOD_NOR_READ_4BYTE,
    MOD_NOR_READ_FAST_4BYTE,
    MOD_NOR_READ_FAST_1_1_2_4BYTE,
    MOD_NOR_READ_FAST_1_2_2_4BYTE,
    MOD_NOR_READ_FAST_1_1_4_4BYTE,
    MOD_NOR_READ_FAST_1_4_4_4BYTE,

    MOD_NOR_READ_MODE_COUNT
};

/*
 * \brief NOR program mode indices
 */
enum mod_nor_program_mode {
    /* Program Operation */
    MOD_NOR_PROGRAM = 0,
    MOD_NOR_PROGRAM_1_1_2,
    MOD_NOR_PROGRAM_1_2_2,
    MOD_NOR_PROGRAM_1_1_4,
    MOD_NOR_PROGRAM_1_4_4,
    MOD_NOR_PROGRAM_4BYTE,
    MOD_NOR_PROGRAM_1_1_4_4BYTE,
    MOD_NOR_PROGRAM_1_4_4_4BYTE,

    MOD_NOR_PROGRAM_MODE_COUNT
};

/*
 * \brief NOR erase mode indices
 */
enum mod_nor_erase_mode {
    /* Erase Operation */
    MOD_NOR_ERASE_SECTOR_4KB = 0,
    MOD_NOR_ERASE_SECTOR_32KB,
    MOD_NOR_ERASE_BLOCK,
    MOD_NOR_ERASE_CHIP,
    MOD_NOR_ERASE_SECTOR_4KB_4BYTE,
    MOD_NOR_ERASE_SECTOR_32KB_4BYTE,
    MOD_NOR_ERASE_BLOCK_4BYTE,

    MOD_NOR_ERASE_MODE_COUNT
};

/*!
 * \brief APIs to access the descriptors in the flash memory.
 */
struct mod_nor_api {
    /*!
     * \brief Read data from the SPI-NOR device
     *
     * \param id The nor element identifier.
     * \param slave Slvae device number of the SPI-NOR.
     * \param mode Read mode indices.
     * \param offset The top address on the SPI-NOR for reading.
     * \param buf Pointer to store reading data.
     * \param len Length of reading data.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid command was encountered.
     * \retval ::FWK_E_SUPPORT The SPI-NOR does not support a mode specified by
     * the argmument. \retval ::FWK_E_STATE The qspi module isn't started yet.
     * \return One of the other specific error codes.
     */
    int (*read)(
        fwk_id_t id,
        uint8_t slave,
        enum mod_nor_read_mode mode,
        uint32_t offset,
        void *buf,
        uint32_t len);

    /*!
     * \brief Program data to the SPI-NOR device
     *
     * \param id The nor element identifier.
     * \param slave Slvae device number of the SPI-NOR.
     * \param mode Program mode indices.
     * \param offset The top address on the SPI-NOR for programming.
     * \param buf Pointer to store programming data.
     * \param len Length of programming data.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid command was encountered.
     * \retval ::FWK_E_SUPPORT The SPI-NOR does not support a mode specified by
     * the argmument. \retval ::FWK_E_DEVICE Programming failed. \retval
     * ::FWK_E_STATE The qspi module isn't started yet. \return One of the other
     * specific error codes.
     */
    int (*program)(
        fwk_id_t id,
        uint8_t slave,
        enum mod_nor_program_mode mode,
        uint32_t offset,
        void *buf,
        uint32_t len);

    /*!
     * \brief Erase data to the SPI-NOR device
     *
     * \param id The nor element identifier.
     * \param slave Slvae device number of the SPI-NOR.
     * \param mode Erase mode indices.
     * \param offset The top address on the SPI-NOR for erase.
     * \param len Length of erase data.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid command was encountered.
     * \retval ::FWK_E_SUPPORT The SPI-NOR does not support a mode specified by
     * the argmument. \retval ::FWK_E_DEVICE Erase failed. \retval ::FWK_E_STATE
     * The qspi module isn't started yet. \return One of the other specific
     * error codes.
     */
    int (*erase)(
        fwk_id_t id,
        uint8_t slave,
        enum mod_nor_erase_mode mode,
        uint32_t offset,
        uint32_t len);

    /*!
     * \brief Reset the SPI-NOR device
     *
     * \param id The nor element identifier.
     * \param slave Slvae device number of the SPI-NOR.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid command was encountered.
     * \retval ::FWK_E_STATE The qspi module isn't started yet.
     * \return One of the other specific error codes.
     */
    int (*reset)(fwk_id_t id, uint8_t slave);

    /*!
     * \brief configure the memory mapped read for the SPI-NOR device
     *
     * \param id The nor element identifier.
     * \param slave Slave device number of the SPI-NOR.
     * \param mode Read mode indices.
     * \param enable flag to enable or disable XIP.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid command was encountered.
     * \retval ::FWK_E_STATE The qspi module isn't started yet.
     * \return One of the other specific error codes.
     */
    int (*configure_mmap_read)(
        fwk_id_t id,
        uint8_t slave,
        enum mod_nor_read_mode mode,
        bool enable);
};

/*
 * \brief NOR command indices
 */
enum mod_nor_command_idx {
    MOD_NOR_COMMAND_RESET_ENABLE = 0,
    MOD_NOR_COMMAND_RESET_EXECUTE,
    MOD_NOR_COMMAND_READ_ID,
    MOD_NOR_COMMAND_READ_SFDP,
    MOD_NOR_COMMAND_WRITE_ENABLE,
    MOD_NOR_COMMAND_WRITE_DISABLE,
    MOD_NOR_COMMAND_READ_STATUS,
    MOD_NOR_COMMAND_WRITE_STATUS,
    MOD_NOR_COMMAND_READ,
    MOD_NOR_COMMAND_FAST_READ,
    MOD_NOR_COMMAND_FAST_READ_1_1_2,
    MOD_NOR_COMMAND_FAST_READ_1_2_2,
    MOD_NOR_COMMAND_FAST_READ_1_1_4,
    MOD_NOR_COMMAND_FAST_READ_1_4_4,
    MOD_NOR_COMMAND_READ_4B,
    MOD_NOR_COMMAND_FAST_READ_4B,
    MOD_NOR_COMMAND_FAST_READ_1_1_2_4B,
    MOD_NOR_COMMAND_FAST_READ_1_2_2_4B,
    MOD_NOR_COMMAND_FAST_READ_1_1_4_4B,
    MOD_NOR_COMMAND_FAST_READ_1_4_4_4B,
    MOD_NOR_COMMAND_PROGRAM,
    MOD_NOR_COMMAND_PROGRAM_1_1_2,
    MOD_NOR_COMMAND_PROGRAM_1_2_2,
    MOD_NOR_COMMAND_PROGRAM_1_1_4,
    MOD_NOR_COMMAND_PROGRAM_1_4_4,
    MOD_NOR_COMMAND_PROGRAM_4B,
    MOD_NOR_COMMAND_PROGRAM_1_1_4_4B,
    MOD_NOR_COMMAND_PROGRAM_1_4_4_4B,
    MOD_NOR_COMMAND_ERASE_SECTOR_4KB,
    MOD_NOR_COMMAND_ERASE_SECTOR_32KB,
    MOD_NOR_COMMAND_ERASE_BLOCK,
    MOD_NOR_COMMAND_ERASE_CHIP,
    MOD_NOR_COMMAND_ERASE_SECTOR_4KB_4B,
    MOD_NOR_COMMAND_ERASE_SECTOR_32KB_4B,
    MOD_NOR_COMMAND_ERASE_BLOCK_4B,

    MOD_NOR_COMMAND_COUNT
};

/*
 * \brief operation list of NOR device
 */
struct mod_nor_operation {
    qspi_ope_func_t set_io_protocol;
    qspi_ope_func_t set_4byte_addr_mode;
    qspi_ope_func_t get_program_result;
    qspi_ope_func_t get_erase_result;
};

/*!
 * \brief NOR device configuration data.
 */
struct mod_nor_dev_config {
    fwk_id_t driver_id;
    fwk_id_t api_id;
    bool enable_reset_on_boot;
    uint16_t program_page_size;
    uint32_t erase_block_size;
    struct qspi_command *command_table;
    struct mod_nor_operation *operation;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_NOR_H */

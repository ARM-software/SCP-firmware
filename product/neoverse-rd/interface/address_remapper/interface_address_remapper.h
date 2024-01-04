/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef INTERFACE_ADDRESS_REMAPPER
#define INTERFACE_ADDRESS_REMAPPER

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupInterfaces Interfaces
 * @{
 */

/*!
 * \defgroup GroupAddressRemapper Address Remapper module interface
 *
 * \brief Interface definition for Address Remapper drivers.
 *
 * \details This provides an generic interface for the modules to bind to a
 *          platform specific Address Remapper driver module.
 * @{
 */

/*!
 * \brief API to read/write to an address greater than 32 bit.
 */
struct interface_address_remapper_rw_api {
    /*!
     * \brief Read 8 bits from the target address
     *
     * \param addr Read Address
     *
     * \return 8 bits value from the address
     */
    uint8_t (*read8)(uint64_t addr);

    /*!
     * \brief Read 16 bits from the target address
     *
     * \param addr Read Address
     *
     * \return  16 bits value from the address
     */
    uint16_t (*read16)(uint64_t addr);

    /*!
     * \brief Read 32 bits from the target address
     *
     * \param addr Read Address
     *
     * \return 32 bits value from the address
     */
    uint32_t (*read32)(uint64_t addr);

    /*!
     * \brief Read 64 bits from the target address
     *
     * \param addr Read Address
     *
     * \return 64 bits value from the address
     */
    uint64_t (*read64)(uint64_t addr);

    /*!
     * \brief Write 8 bits to the target address
     *
     * \param addr Target Address
     * \param value 8 bits value to be written
     *
     * \return Nothing
     */
    void (*write8)(uint64_t addr, uint8_t value);

    /*!
     * \brief Write 16 bits to the target address
     *
     * \param addr Target Address
     * \param value 16 bits value to be written
     *
     * \return Nothing
     */
    void (*write16)(uint64_t addr, uint16_t value);

    /*!
     * \brief Write 32 bits to the target address
     *
     * \param addr Target Address
     * \param value 32 bits value to be written
     *
     * \return Nothing
     */
    void (*write32)(uint64_t addr, uint32_t value);

    /*!
     * \brief Write 64 bits to the target address
     *
     * \param addr Target Address
     * \param value 64 bits value to be written
     *
     * \return Nothing
     */
    void (*write64)(uint64_t addr, uint64_t value);
};

#endif /* INTERFACE_ADDRESS_REMAPPER */

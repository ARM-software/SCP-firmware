/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_HSSPI_H
#define MOD_HSSPI_H

#include <internal/hsspi_api.h>

#include <fwk_id.h>

#include <stdint.h>
/*!
 * \addtogroup GroupSYNQUACERModule SYNQUACER Product Modules
 * @{
 */

/*!
 * \defgroup GroupSYNQUACER_HSSPI HSSPI Driver
 *
 * \brief SynQuacer High Speed SPI device driver.
 *
 * \details This module implements a device driver for the HSSPI
 *
 * @{
 */

/*!
 * \brief APIs to access the descriptors in the flash memory.
 */
struct mod_hsspi_api {
    /*!
     * \brief initialize the high speed spi controller
     *
     * \param void
     * \return void
     */
    void (*hsspi_init)(void);

    /*!
     * \brief uninitialize the high speed spi controller
     *
     * \param void
     * \return void
     */
    void (*hsspi_exit)(void);
};

/*!
 * \brief HSSPI device configuration data.
 */
struct mod_hsspi_config {
    /*! Base address of the device registers */
    uintptr_t reg_base;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_HSSPI_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_F_I2C_H
#define MOD_F_I2C_H

#include <i2c_api.h>

#include <internal/i2c_depend.h>
#include <internal/i2c_driver.h>

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupSYNQUACERModule SYNQUACER Product Modules
 * @{
 */

/*!
 * \defgroup GroupF_I2C F_I2C Driver
 *
 * \brief SynQuacer F_I2C device driver.
 *
 * \details This module implements a device driver for the F_I2C
 *
 * @{
 */

/*!
 * \brief APIs to access the i2c device.
 */
struct mod_f_i2c_api {
    /*!
     * \brief APIs to access the i2c device.
     *
     * \param id Module identifier.
     * \param count Pointer to storage for the descriptor table.
     *
     * \retval I2C_ERR_OK Operation succeeded.
     * \return One of the I2C_ERR_t codes described by f_i2c module.
     */
    void (*init)(void);

    /*!
     * \brief receive data from i2c device
     *
     * \param ch channel
     * \param slave_address slave address
     * \param address target address
     * \param data buffer to receive the data
     * \param length length to read
     *
     * \retval I2C_ERR_OK Operation succeeded.
     * \retval I2C_ERR_PARAM Invalid parameter.
     * \retval I2C_ERR_BUSY I2C controller is busy.
     * \retval I2C_ERR_BER Bus error.
     * \return One of the other specific error codes.
     */
    I2C_ERR_t (*recv_data)
    (I2C_EN_CH_t ch,
     uint32_t slave_address,
     uint32_t address,
     uint8_t *data,
     int length);

    /*!
     * \brief send data to i2c device
     *
     * \param ch channel
     * \param slave_address slave address
     * \param address target address
     * \param data buffer to be sent
     * \param length length to be sent
     *
     * \retval I2C_ERR_OK Operation succeeded.
     * \retval I2C_ERR_PARAM Invalid parameter.
     * \retval I2C_ERR_BUSY I2C controller is busy.
     * \retval I2C_ERR_BER Bus error.
     * \return One of the other specific error codes.
     */
    I2C_ERR_t (*send_data)
    (I2C_EN_CH_t ch,
     uint32_t slave_address,
     uint32_t address,
     const uint8_t *data,
     int length);
};

/*!
 * \brief F_I2C device configuration data.
 */
struct mod_f_i2c_config {
    /*! Base address of the device registers */
    uintptr_t reg_base;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_F_I2C_H */

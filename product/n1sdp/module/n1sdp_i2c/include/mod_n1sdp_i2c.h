/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP I2C module.
 */

#ifndef MOD_N1SDP_I2C_H
#define MOD_N1SDP_I2C_H

#include <internal/n1sdp_i2c.h>
#include <stdbool.h>
#include <stdint.h>
#include <fwk_id.h>
#include <fwk_macros.h>

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * @{
 */

/*!
 * \defgroup N1SDP I2C Driver
 *
 * \brief Driver support for N1SDP I2C.
 *
 * \details This module provides driver support for Cadence I2C controller.
 *
 * \{
 */

/*!
 * \brief I2C API ID.
 */
#define MOD_N1SDP_I2C_API_ID  FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_I2C, 0)

/*!
 * \brief I2C Speed.
 */
enum mod_n1sdp_i2c_speed {
    MOD_N1SDP_I2C_SPEED_NORMAL = (100 * FWK_KHZ),
    MOD_N1SDP_I2C_SPEED_FAST = (400 * FWK_KHZ),
    MOD_N1SDP_I2C_SPEED_FAST_PLUS = (1 * FWK_MHZ),
};

/*!
 * \brief Operating mode.
 */
enum mod_n1sdp_i2c_mode {
    MOD_N1SDP_I2C_SLAVE_MODE,
    MOD_N1SDP_I2C_MASTER_MODE,
};

/*!
 * \brief Address size.
 */
enum mod_n1sdp_i2c_address_size {
    MOD_N1SDP_I2C_ADDRESS_10_BIT,
    MOD_N1SDP_I2C_ADDRESS_7_BIT,
};

/*!
 * \brief Hold mode.
 */
enum mod_n1sdp_i2c_hold {
    MOD_N1SDP_I2C_HOLD_OFF,
    MOD_N1SDP_I2C_HOLD_ON,
};

/*!
 * \brief I2C acknowledge.
 */
enum mod_n1sdp_i2c_ack {
    MOD_N1SDP_I2C_ACK_DISABLE,
    MOD_N1SDP_I2C_ACK_ENABLE,
};

/*!
 * \brief I2C device configuration.
 */
struct mod_n1sdp_i2c_device_config {
    /*! Base address of the device registers */
    uintptr_t reg_base;

    /*! Reference clock (Hertz) */
    uint64_t clock_rate_hz;

    /*! I2C bus speed (Hertz) */
    enum mod_n1sdp_i2c_speed bus_speed_hz;

    /*! Acknowledge enable */
    enum mod_n1sdp_i2c_ack ack_en;

    /*! Operating mode (Master/Slave) */
    enum mod_n1sdp_i2c_mode mode;

    /*! Address size (7Bit/10Bit) */
    enum mod_n1sdp_i2c_address_size addr_size;

    /*! Hold mode (ON/OFF) */
    enum mod_n1sdp_i2c_hold hold_mode;

    /*! Slave address */
    uint16_t slave_addr;
};

/*!
 * \brief API to access the I2C functions.
 */
struct mod_n1sdp_i2c_master_api {
    /*!
     * \brief I2C read function.
     *
     * \param device_id Element identifier.
     * \param address Address of the slave.
     * \param data Pointer to data buffer.
     * \param length Data size to be read in bytes.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \return One of the other specific error codes described by the framework.
     */
    int (*read)(fwk_id_t device_id, uint16_t address, char *data,
                uint16_t length);

    /*!
     * \brief I2C write function.
     *
     * \param device_id Element identifier.
     * \param address Address of slave.
     * \param data Pointer to data buffer.
     * \param length Data size to be written in bytes.
     * \param stop When set to true indicates end of data transfer and interface
     *      releases the sclk line.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \return One of the other specific error codes described by the framework.
     */
    int (*write)(fwk_id_t device_id, uint16_t address, const char *data,
                 uint16_t length, bool stop);
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_N1SDP_I2C_H */

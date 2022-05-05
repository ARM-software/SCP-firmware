/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Cadence I2C module.
 */

#ifndef MOD_CDNS_I2C_H
#define MOD_CDNS_I2C_H

#include <internal/cdns_i2c.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupCDNSModule CDNS Product Modules
 * \{
 */

/*!
 * \defgroup Cadence I2C Driver
 *
 * \brief Driver support for Cadence I2C.
 *
 * \details This module provides driver support for Cadence I2C controller.
 *
 * \{
 */

/*!
 * \brief I2C API ID.
 */
enum mod_cdns_i2c_api {
    MOD_CDNS_I2C_API_CONTROLLER_POLLED,
    MOD_CDNS_I2C_API_TARGET_IRQ,
    MOD_CDNS_I2C_API_COUNT,
};

/*!
 * \brief I2C Speed.
 */
enum mod_cdns_i2c_speed {
    MOD_CDNS_I2C_SPEED_NORMAL = (100 * FWK_KHZ),
    MOD_CDNS_I2C_SPEED_FAST = (400 * FWK_KHZ),
    MOD_CDNS_I2C_SPEED_FAST_PLUS = (1 * FWK_MHZ),
};

/*!
 * \brief Operating mode.
 */
enum mod_cdns_i2c_mode {
    MOD_CDNS_I2C_TARGET_MODE,
    MOD_CDNS_I2C_CONTROLLER_MODE,
};

/*!
 * \brief Address size.
 */
enum mod_cdns_i2c_address_size {
    MOD_CDNS_I2C_ADDRESS_10_BIT,
    MOD_CDNS_I2C_ADDRESS_7_BIT,
};

/*!
 * \brief Hold mode.
 */
enum mod_cdns_i2c_hold {
    MOD_CDNS_I2C_HOLD_OFF,
    MOD_CDNS_I2C_HOLD_ON,
};

/*!
 * \brief I2C acknowledge.
 */
enum mod_cdns_i2c_ack {
    MOD_CDNS_I2C_ACK_DISABLE,
    MOD_CDNS_I2C_ACK_ENABLE,
};

/*!
 * \brief I2C transmission state.
 */
enum mod_cdns_i2c_trx_state {
    MOD_CDNS_I2C_STATE_TX,
    MOD_CDNS_I2C_STATE_RX,
};

/*!
 * \brief I2C interrupt mode data context.
 */
struct mod_cdns_i2c_irq_data_ctx {
    /*! Transmission state */
    enum mod_cdns_i2c_trx_state state;
    /*! Pointer to buffer */
    uint8_t *data;
    /*! Buffer size */
    uint8_t size;
    /*! Index to the buffer */
    uint8_t index;
    /*! Flag to indicate ongoing transaction */
    bool busy;
};

/*!
 * \brief I2C device configuration.
 */
struct mod_cdns_i2c_device_config {
    /*! Base address of the device registers */
    uintptr_t reg_base;

    /*! Reference clock (Hertz) */
    uint64_t clock_rate_hz;

    /*! I2C bus speed (Hertz) */
    enum mod_cdns_i2c_speed bus_speed_hz;

    /*! Acknowledge enable */
    enum mod_cdns_i2c_ack ack_en;

    /*! Operating mode (Controller/Target) */
    enum mod_cdns_i2c_mode mode;

    /*! Address size (7Bit/10Bit) */
    enum mod_cdns_i2c_address_size addr_size;

    /*! Hold mode (ON/OFF) */
    enum mod_cdns_i2c_hold hold_mode;

    /*! Target address */
    uint16_t target_addr;

    /*! I2C IRQ */
    uint8_t irq;

    /*! Identifier where callback notifications are sent */
    fwk_id_t callback_mod_id;

    /*! I2C Transfer Register Size (TSR) */
    uint16_t max_xfr_size;

    /*! I2C FIFO depth */
    uint8_t fifo_depth;
};

/*!
 * \brief API to access the I2C controller functions in polled mode.
 */
struct mod_cdns_i2c_controller_api_polled {
    /*!
     * \brief I2C read function.
     *
     * \param device_id Element identifier.
     * \param address Address of the target.
     * \param data Pointer to data buffer.
     * \param length Data size to be read in bytes.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \return One of the other specific error codes described by the framework.
     */
    int (*read)(
        fwk_id_t device_id,
        uint16_t address,
        char *data,
        uint16_t length);

    /*!
     * \brief I2C write function.
     *
     * \param device_id Element identifier.
     * \param address Address of target.
     * \param data Pointer to data buffer.
     * \param length Data size to be written in bytes.
     * \param stop When set to true indicates end of data transfer and interface
     *      releases the sclk line.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \return One of the other specific error codes described by the framework.
     */
    int (*write)(
        fwk_id_t device_id,
        uint16_t address,
        const char *data,
        uint16_t length,
        bool stop);
};

/*!
 * \brief API to access the I2C target functions in interrupt mode.
 */
struct mod_cdns_i2c_target_api_irq {
    /*!
     * \brief I2C read function.
     *
     * \param device_id Element identifier.
     * \param data Pointer to data buffer.
     * \param length Data size to be read in bytes.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \return One of the other specific error codes described by the framework.
     */
    int (*read)(fwk_id_t device_id, uint8_t *data, uint8_t length);

    /*!
     * \brief I2C write function.
     *
     * \param device_id Element identifier.
     * \param data Pointer to data buffer.
     * \param length Data size to be written in bytes.
     * \param stop When set to true indicates end of data transfer and interface
     *      releases the sclk line.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \return One of the other specific error codes described by the framework.
     */
    int (*write)(fwk_id_t device_id, uint8_t *data, uint8_t length);
};

/*!
 * \brief I2C module notifications.
 */
enum mod_cdns_i2c_notifications {
    MOD_CDNS_I2C_NOTIFICATION_IDX_RX,
    MOD_CDNS_I2C_NOTIFICATION_IDX_TX,
    MOD_CDNS_I2C_NOTIFICATION_IDX_ERROR,
    MOD_CDNS_I2C_NOTIFICATION_COUNT,
};

/*!
 * \brief Identifier for I2C target receive callback notification.
 */
static const fwk_id_t mod_cdns_i2c_notification_id_target_rx =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_CDNS_I2C,
        MOD_CDNS_I2C_NOTIFICATION_IDX_RX);

/*!
 * \brief Identifier for I2C target transmit callback notification.
 */
static const fwk_id_t mod_cdns_i2c_notification_id_target_tx =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_CDNS_I2C,
        MOD_CDNS_I2C_NOTIFICATION_IDX_TX);

/*!
 * \brief Identifier for I2C target error callback notification.
 */
static const fwk_id_t mod_cdns_i2c_notification_id_target_error =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_CDNS_I2C,
        MOD_CDNS_I2C_NOTIFICATION_IDX_ERROR);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_CDNS_I2C_H */

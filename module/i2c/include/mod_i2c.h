/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     I2C HAL
 */

#ifndef MOD_I2C_H
#define MOD_I2C_H

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupI2C I2C HAL
 *
 * \details Support for transmitting and receiving data through I2C Bus
 *     peripherals
 * @{
 */

/*!
 * \brief Configuration data for an I2C device.
 */
struct mod_i2c_dev_config {
    /*!
     *  Identifier of the module or element providing the driver for the I2C
     *  device.
     */
    fwk_id_t driver_id;

    /*! Identifier of the driver API. */
    fwk_id_t api_id;
};

/*!
 * \brief Parameters of the event.
 */
struct mod_i2c_event_param {
    /*! Status of the I2C transaction. */
    int status;
};

/*!
 * \brief  I2C transaction request parameters.
 */
struct mod_i2c_request {
    /*!
     * \brief Number of data bytes to transmit.
     */
    uint8_t transmit_byte_count;

    /*!
     * \brief Number of data bytes to receive.
     */
    uint8_t receive_byte_count;

     /*!
     * \brief Address of the slave on the I2C bus.
     */
    uint8_t slave_address;

    /*!
     * \brief Pointer to the data to transmit.
     */
    uint8_t *transmit_data;

    /*!
     * \brief Pointer to the received data.
     */
    uint8_t *receive_data;
};

static_assert(sizeof(struct mod_i2c_request) <= FWK_EVENT_PARAMETERS_SIZE,
    "An I2C request should fit in the params field of an event\n");

/*!
 * \brief I2C driver interface.
 *
 * \details The interface the I2C HAL module relies on to perform actions on an
 *      I2C device.
 */
struct mod_i2c_driver_api {
    /*!
     * \brief Request transmission of data as Master to a selected slave.
     *
     * \details When the function returns the transmission may not be completed.
     *      The driver can assume the integrity of the data during the
     *      transmission. When the transmission operation has finished,
     *      the driver shall report it through the I2C HAL module driver
     *      response API.
     *
     * \param dev_id Identifier of the I2C device
     * \param transmit_request Request information for the I2C transmission
     *
     * \retval FWK_PENDING The request was submitted.
     * \retval FWK_SUCCESS The request was successfully completed.
     * \retval FWK_E_PARAM One or more parameters were invalid.
     * \return One of the standard framework status codes.
     */
    int (*transmit_as_master)(
        fwk_id_t dev_id, struct mod_i2c_request *transmit_request);

    /*!
     * \brief Request the reception of data as Master from a selected slave.
     *
     * \details When the function returns the reception may not be completed.
     *      The driver can assume the integrity of the data pointer during the
     *      reception. When the reception operation has finished, the driver
     *      shall report it through the I2C HAL module driver response API.
     *
     * \param dev_id Identifier of the I2C device
     * \param receive_request Request information for the I2C reception
     *
     * \retval FWK_PENDING The request was submitted.
     * \retval FWK_SUCCESS The request was successfully completed.
     * \retval FWK_E_PARAM One or more parameters were invalid.
     * \return One of the standard framework status codes.
     */
    int (*receive_as_master)(
        fwk_id_t dev_id, struct mod_i2c_request *receive_request);
};

/*!
 * \brief I2C HAL module interface.
 */
struct mod_i2c_api {
    /*!
     * \brief Request transmission of data as Master to a selected slave.
     *
     * \details When the function returns the transmission is not completed,
     *      possibly not even started. The data buffer must stay allocated
     *      and its content must not be modified until the transmission is
     *      completed or aborted. When the transmission operation has finished
     *      a response event is sent to the client.
     *
     * \param dev_id Identifier of the I2C device
     * \param slave_address Address of the slave on the I2C bus
     * \param data Pointer to the data bytes to transmit to the slave
     * \param byte_count Number of data bytes to transmit
     *
     * \retval FWK_PENDING The request was submitted.
     * \retval FWK_E_PARAM One or more parameters were invalid.
     * \retval FWK_E_BUSY An I2C transaction is already on-going.
     * \retval FWK_E_DEVICE The transmission is aborted due to a device error.
     * \return One of the standard framework status codes.
     */
    int (*transmit_as_master)(fwk_id_t dev_id, uint8_t slave_address,
        uint8_t *data, uint8_t byte_count);

    /*!
     * \brief Request reception of data as Master from a selected slave.
     *
     * \details When the function returns the reception is not completed,
     *      possibly not even started. The data buffer must stay allocated
     *      and its content must not be modified until the reception is
     *      completed or aborted. When the reception operation has finished
     *      a response event is sent to the client.
     *
     * \param dev_id Identifier of the I2C device
     * \param slave_address Address of the slave on the I2C bus
     * \param data Pointer to the buffer to receive data from the slave
     * \param byte_count Number of data bytes to receive
     *
     * \retval FWK_PENDING The request was submitted.
     * \retval FWK_E_PARAM One or more parameters were invalid.
     * \retval FWK_E_BUSY An I2C transaction is already on-going.
     * \retval FWK_E_DEVICE The reception is aborted due to a device error.
     * \return One of the standard framework status codes.
     */
    int (*receive_as_master)(fwk_id_t dev_id, uint8_t slave_address,
        uint8_t *data, uint8_t byte_count);

    /*!
     * \brief Request the transmission followed by the reception of data as
     *      Master to/from a selected slave.
     *
     * \details When the function returns the transaction is not completed,
     *      possibly not even started. The data buffer must stay allocated
     *      and its content must not be modified until the transaction is
     *      completed or aborted. When the transaction operation has finished
     *      a response event is sent to the client.
     *
     * \param dev_id Identifier of the I2C device
     * \param slave_address Address of the slave on the I2C bus
     * \param transmit_data Pointer to the buffer to transmit the data in the
     *      first phase.
     * \param receive_data Pointer to the buffer to receive the data in the
     *      second phase.
     * \param transmit_byte_count Number of data bytes to transmit in the first
     *      phase.
     * \param receive_byte_count Number of data bytes to receive in the second
     *      phase.
     *
     * \retval FWK_PENDING The request was submitted.
     * \retval FWK_E_PARAM One or more parameters were invalid.
     * \retval FWK_E_BUSY An I2C transaction is already on-going.
     * \retval FWK_E_DEVICE The reception is aborted due to a device error.
     * \return One of the standard framework status codes.
     */
    int (*transmit_then_receive_as_master)(fwk_id_t dev_id,
        uint8_t slave_address, uint8_t *transmit_data, uint8_t *receive_data,
        uint8_t transmit_byte_count, uint8_t receive_byte_count);
};

/*!
 * \brief I2C HAL module driver response API.
 *
 * \details The interface the I2C HAL module exposes to its module drivers to
 *      report transmission or reception completion following the occurrence of
 *      interrupts.
 */
struct mod_i2c_driver_response_api {
    /*!
     * \brief Function called back after the completion or abortion of an I2C
     *      transaction request.
     *
     * \param dev_id Identifier of the I2C device
     * \param i2c_status I2C transaction status
     */
    void (*transaction_completed)(fwk_id_t dev_id, int i2c_status);
};

/*!
 * \defgroup GroupI2CIds Identifiers
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_i2c_api_idx {
    MOD_I2C_API_IDX_I2C,
    MOD_I2C_API_IDX_DRIVER_RESPONSE,
    MOD_I2C_API_IDX_COUNT,
};

/*! I2C API identifier */
static const fwk_id_t mod_i2c_api_id_i2c =
    FWK_ID_API_INIT(FWK_MODULE_IDX_I2C, MOD_I2C_API_IDX_I2C);

/*! Driver response API identifier */
static const fwk_id_t mod_i2c_api_id_driver_response =
    FWK_ID_API_INIT(FWK_MODULE_IDX_I2C, MOD_I2C_API_IDX_DRIVER_RESPONSE);

/*!
 * \brief Event indices
 */
enum mod_i2c_event_idx {
    MOD_I2C_EVENT_IDX_REQUEST_TRANSMIT,
    MOD_I2C_EVENT_IDX_REQUEST_RECEIVE,
    MOD_I2C_EVENT_IDX_REQUEST_TRANSMIT_THEN_RECEIVE,
    MOD_I2C_EVENT_IDX_COUNT,
};

/*! Transmit request event identifier */
static const fwk_id_t mod_i2c_event_id_request_tx = FWK_ID_EVENT_INIT(
    FWK_MODULE_IDX_I2C, MOD_I2C_EVENT_IDX_REQUEST_TRANSMIT);

/*! Receive request event identifier */
static const fwk_id_t mod_i2c_event_id_request_rx = FWK_ID_EVENT_INIT(
    FWK_MODULE_IDX_I2C, MOD_I2C_EVENT_IDX_REQUEST_RECEIVE);

/*! Transmit then receive request event identifier */
static const fwk_id_t mod_i2c_event_id_request_tx_rx = FWK_ID_EVENT_INIT(
    FWK_MODULE_IDX_I2C, MOD_I2C_EVENT_IDX_REQUEST_TRANSMIT_THEN_RECEIVE);

/*!
 * \}
 */

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_I2C_H */

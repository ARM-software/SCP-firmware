/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_PSU_H
#define MOD_PSU_H

#include <fwk_id.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules
 * \defgroup GroupPsu Power Supply Unit (PSU)
 *
 * \details The `psu` module represents an abstract interface through which
 *      power supply units can be used. It provides functions for manipulating
 *      the voltage as well as enabling/disabling the supply.
 *
 *      Users of the HAL interact with the individual power supply units through
 *      the [device API](::mod_psu_device_api). This is the primary interface
 *      that this module exposes and implements.
 *
 *      Separate to this is the [driver API](::mod_psu_driver_api), through
 *      which the HAL will communicate with various driver implementations.
 *      Driver implementations that need it may also utilise the [driver
 *      response API](::mod_psu_driver_response_api), which allows power supply
 *      units to pend requests and respond on-demand.
 *
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_psu_api_idx {
    /*!
     * \brief Device API.
     *
     * \note This API identifier implements the ::mod_psu_device_api interface.
     *
     * \warning Binding to this API must occur through an element of this
     *      module.
     */
    MOD_PSU_API_IDX_DEVICE,

    /*!
     * \brief Driver response API index.
     *
     * \note This API identifier implements the ::mod_psu_driver_response_api
     *      interface.
     *
     * \warning Binding to this API must occur through an element of this
     *      module.
     */
    MOD_PSU_API_IDX_DRIVER_RESPONSE,

    /*!
     * \brief Number of defined APIs.
     */
    MOD_PSU_API_IDX_COUNT
};

/*!
 * \brief Event indices.
 */
enum mod_psu_event_idx {
    /*!
     * \brief Response event to a ::mod_psu_device_api::get_enabled call.
     *
     * \note This event identifier uses the ::mod_psu_response structure as its
     *      event parameters. The ::mod_psu_response::enabled field is active.
     */
    MOD_PSU_EVENT_IDX_GET_ENABLED,

    /*!
     * \brief Response event to a ::mod_psu_device_api::set_enabled call.
     *
     * \note This event identifier uses the ::mod_psu_response structure as its
     *      event parameters.
     */
    MOD_PSU_EVENT_IDX_SET_ENABLED,

    /*!
     * \brief Response event to a ::mod_psu_device_api::get_voltage call.
     *
     * \note This event identifier uses the ::mod_psu_response structure as its
     *      event parameters. The ::mod_psu_response::voltage field is active.
     */
    MOD_PSU_EVENT_IDX_GET_VOLTAGE,

    /*!
     * \brief Response event to a ::mod_psu_device_api::set_voltage call.
     *
     * \note This event identifier uses the ::mod_psu_response structure as its
     *      event parameters.
     */
    MOD_PSU_EVENT_IDX_SET_VOLTAGE,

    /*!
     * \brief Number of defined events.
     */
    MOD_PSU_EVENT_IDX_COUNT,
};

/*!
 * \brief Device API.
 *
 * \details The PSU device API represents the abstract interface used to control
 *      individual power supplies.
 *
 *      This is the primary interface through which users of the HAL will
 *      interact with individual power supply units.
 */
struct mod_psu_device_api {
    /*!
     * \brief Get whether the device is enabled or not.
     *
     * \param[in] device_id Identifier of the device to get the state of.
     * \param[out] enabled `true` if the device is enabled, otherwise `false`.
     *
     * \retval ::FWK_E_HANDLER An error occurred in the device driver.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     * \retval ::FWK_E_STATE The device cannot currently accept the request.
     * \retval ::FWK_SUCCESS The operation succeeded.
     *
     * \return Status code representing the result of the operation.
     */
    int (*get_enabled)(fwk_id_t device_id, bool *enabled);

    /*!
     * \brief Enable or disable the device.
     *
     * \param[in] device_id Identifier of the device to set the state of.
     * \param[in] enable `true` to enable the device, or `false` to disable it.
     *
     * \retval ::FWK_E_HANDLER An error occurred in the device driver.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     * \retval ::FWK_E_STATE The device cannot currently accept the request.
     * \retval ::FWK_SUCCESS The operation succeeded.
     *
     * \return Status code representing the result of the operation.
     */
    int (*set_enabled)(fwk_id_t device_id, bool enable);

    /*!
     * \brief Get the voltage of a device.
     *
     * \param[in] device_id Identifier of the device to get the voltage of.
     * \param[out] voltage Voltage in millivolts (mV).
     *
     * \retval ::FWK_E_HANDLER An error occurred in the device driver.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     * \retval ::FWK_E_STATE The device cannot currently accept the request.
     * \retval ::FWK_SUCCESS The operation succeeded.
     *
     * \return Status code representing the result of the operation.
     */
    int (*get_voltage)(fwk_id_t device_id, uint64_t *voltage);

    /*!
     * \brief Set the voltage of a device.
     *
     * \param[in] device_id Identifier of the device to set the voltage of.
     * \param[in] voltage New voltage in millivolts (mV).
     *
     * \retval ::FWK_E_HANDLER An error occurred in the device driver.
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     * \retval ::FWK_E_STATE The device cannot currently accept the request.
     * \retval ::FWK_SUCCESS The operation succeeded.
     *
     * \return Status code representing the result of the operation.
     */
    int (*set_voltage)(fwk_id_t device_id, uint64_t voltage);
};

/*!
 * \brief Device API identifier.
 *
 * \note This identifier corresponds to the ::MOD_PSU_API_IDX_DEVICE API index.
 */
static const fwk_id_t mod_psu_api_id_device =
    FWK_ID_API_INIT(FWK_MODULE_IDX_PSU, MOD_PSU_API_IDX_DEVICE);

/*!
 * \brief Response parameters.
 *
 * \details This structure is passed as the parameters to a response event
 *      dispatched by the `psu` module.
 */
struct mod_psu_response {
    /*!
     * \brief Status code representing the result of the operation.
     */
    int status;

    /*!
     * \brief Response fields.
     *
     * \details These fields are used if the module is responding with extra
     *      information. The activated field, if there is one, is based on the
     *      event identifier of the response.
     */
    union {
        /*!
         * \brief `true` if the device is enabled, otherwise `false`.
         *
         * \warning Used only in response to a ::mod_psu_device_api::get_enabled
         *      call.
         */
        bool enabled;

        /*!
         * \brief Voltage in millivolts (mV).
         *
         * \warning Used only in response to a ::mod_psu_device_api::get_voltage
         *      call.
         */
        uint64_t voltage;
    };
};

/*!
 * \brief Identifier for a ::mod_psu_device_api::get_enabled call response
 *      event.
 *
 * \note This identifier corresponds to the
 *      ::MOD_PSU_EVENT_IDX_GET_ENABLED event index.
 */
static const fwk_id_t mod_psu_event_id_get_enabled = FWK_ID_EVENT_INIT(
    FWK_MODULE_IDX_PSU, MOD_PSU_EVENT_IDX_GET_ENABLED);

/*!
 * \brief Identifier for a ::mod_psu_device_api::set_enabled call response
 *      event.
 *
 * \note This identifier corresponds to the
 *      ::MOD_PSU_EVENT_IDX_SET_ENABLED event index.
 */
static const fwk_id_t mod_psu_event_id_set_enabled = FWK_ID_EVENT_INIT(
    FWK_MODULE_IDX_PSU, MOD_PSU_EVENT_IDX_SET_ENABLED);

/*!
 * \brief Identifier for a ::mod_psu_device_api::get_voltage call response
 *      event.
 *
 * \note This identifier corresponds to the
 *      ::MOD_PSU_EVENT_IDX_GET_VOLTAGE event index.
 */
static const fwk_id_t mod_psu_event_id_get_voltage = FWK_ID_EVENT_INIT(
    FWK_MODULE_IDX_PSU, MOD_PSU_EVENT_IDX_GET_VOLTAGE);

/*!
 * \brief Identifier for a ::mod_psu_device_api::set_voltage call response
 *      event.
 *
 * \note This identifier corresponds to the
 *      ::MOD_PSU_EVENT_IDX_SET_VOLTAGE event index.
 */
static const fwk_id_t mod_psu_event_id_set_voltage = FWK_ID_EVENT_INIT(
    FWK_MODULE_IDX_PSU, MOD_PSU_EVENT_IDX_SET_VOLTAGE);

/*!
 * \brief Driver API.
 *
 * \details The PSU driver API represents the abstract interface that power
 *      supply drivers must implement in order for the PSU module to interact
 *      with them.
 */
struct mod_psu_driver_api {
    /*!
     * \brief Get whether the device is enabled or not.
     *
     * \param[in] id Identifier of the device to get the state of.
     * \param[out] enabled `true` if the device is enabled, or `false` it is
     *      disabled.
     *
     * \retval ::FWK_E_HANDLER The operation failed.
     * \retval ::FWK_PENDING The result of the operation is pending.
     * \retval ::FWK_SUCCESS The operation succeeded.
     *
     * \return Status code representing the result of the operation.
     */
    int (*get_enabled)(fwk_id_t id, bool *enabled);

    /*!
     * \brief Enable or disable the device.
     *
     * \param[in] id Identifier of the device to set the state of.
     * \param[in] enable `true` to enable the device, or `false` to disable it.
     *
     * \retval ::FWK_E_HANDLER The operation failed.
     * \retval ::FWK_PENDING The result of the operation is pending.
     * \retval ::FWK_SUCCESS The operation succeeded.
     *
     * \return Status code representing the result of the operation.
     */
    int (*set_enabled)(fwk_id_t id, bool enable);

    /*!
     * \brief Get the voltage of a device.
     *
     * \param[in] id Identifier of the device to get the voltage of.
     * \param[out] voltage Voltage in millivolts (mV).
     *
     * \retval ::FWK_E_HANDLER The operation failed.
     * \retval ::FWK_PENDING The result of the operation is pending.
     * \retval ::FWK_SUCCESS The operation succeeded.
     *
     * \return Status code representing the result of the operation.
     */
    int (*get_voltage)(fwk_id_t id, uint64_t *voltage);

    /*!
     * \brief Set the voltage of a device.
     *
     * \param[in] id Identifier of the device to set the voltage of.
     * \param[in] voltage New voltage in millivolts (mV).
     *
     * \retval ::FWK_E_HANDLER The operation failed.
     * \retval ::FWK_PENDING The result of the operation is pending.
     * \retval ::FWK_SUCCESS The operation succeeded.
     *
     * \return Status code representing the result of the operation.
     */
    int (*set_voltage)(fwk_id_t id, uint64_t voltage);
};

/*!
 * \brief Driver response.
 *
 * \details This structure is passed to the `psu` module through the [driver
 *      response API](::mod_psu_driver_response_api::respond).
 */
struct mod_psu_driver_response {
    /*!
     * \brief Status code representing the result of the operation.
     */
    int status;

    /*!
     * \brief Response fields.
     *
     * \details These fields are used if the driver is responding with extra
     *      information. The activated field, if there is one, is based on the
     *      event type of the response.
     */
    union {
        /*!
         * \brief `true` if the device is enabled, otherwise `false`.
         *
         * \warning Used only in response to a
         *      ::mod_psu_driver_api::get_enabled call.
         */
        bool enabled;

        /*!
         * \brief Voltage in millivolts (mV).
         *
         * \warning Used only in response to a
         *      ::mod_psu_driver_api::get_voltage call.
         */
        uint64_t voltage;
    };
};

/*!
 * \brief Driver response API.
 *
 * \details The driver response API is the API used by drivers to communicate
 *      the result of a previously-pended operation.
 */
struct mod_psu_driver_response_api {
    /*!
     * \brief Respond to a previous driver request.
     *
     * \param element_id Identifier of the power supply element which submitted
     *      the request to the driver.
     * \param response Driver operation response.
     */
    void (*respond)(
        fwk_id_t element_id,
        struct mod_psu_driver_response response);
};

/*!
 * \brief Driver response API identifier.
 *
 * \note This identifier corresponds to the ::MOD_PSU_API_IDX_DRIVER_RESPONSE
 *      API index.
 */
static const fwk_id_t mod_psu_api_id_driver_response =
    FWK_ID_API_INIT(FWK_MODULE_IDX_PSU, MOD_PSU_API_IDX_DRIVER_RESPONSE);

/*!
 * \brief Device configuration.
 *
 * \details This structure describes configuration parameters for power supply
 *      devices, which are represented as elements of the PSU module.
 */
struct mod_psu_element_cfg {
    /*!
     * \brief Driver entity identifier.
     *
     * \details This identifies the entity of the driver, which must implement
     *      ::mod_psu_driver_api.
     */
    fwk_id_t driver_id;

    /*!
     * \brief Driver API identifier.
     *
     * \details This identifies the API of the driver, which must implement
     *      ::mod_psu_driver_api.
     */
    fwk_id_t driver_api_id;
};

/*!
 * \}
 */

#endif /* MOD_PSU_H */

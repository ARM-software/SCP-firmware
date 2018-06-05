/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_PSU_H
#define MOD_PSU_H

#include <stdbool.h>
#include <stdint.h>
#include <fwk_id.h>
#include <fwk_module_idx.h>

/*!
 * \ingroup GroupModules
 * \defgroup GroupPsu PSU HAL
 * \{
 */

/*!
 * \defgroup GroupPsuApis APIs
 * \{
 */

/*!
 * \brief Device API.
 */
struct mod_psu_device_api {
    /*!
     * \brief Get the enabled state of a device.
     *
     * \param device_id Identifier of the device to get the state of.
     * \param [out] enabled \c true if the device is enabled, or \c if it is
     *      disabled.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM One or more parameters were invalid.
     * \retval FWK_E_STATE The element cannot accept the request.
     * \retval FWK_E_HANDLER An error occurred in the device driver.
     */
    int (*get_enabled)(fwk_id_t device_id, bool *enabled);

    /*!
     * \brief Set the enabled state of a device.
     *
     * \param device_id Identifier of the device to set the state of.
     * \param enable \c true to enable the device, or \c false to disable it.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM One or more parameters were invalid.
     * \retval FWK_E_STATE The element cannot accept the request.
     * \retval FWK_E_HANDLER An error occurred in the device driver.
     */
    int (*set_enabled)(fwk_id_t device_id, bool enable);

    /*!
     * \brief Set the enabled state of a device.
     *
     * \param device_id Identifier of the device to set the state of.
     * \param enable \c true to enable the device, or \c false to disable it.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM One or more parameters were invalid.
     * \retval FWK_E_STATE The element cannot accept the request.
     * \retval FWK_E_NOMEM The event queue is full.
     * \retval FWK_E_PANIC An error in the framework occurred.
     */
    int (*set_enabled_async)(fwk_id_t device_id, bool enable);

    /*!
     * \brief Get the voltage of a device.
     *
     * \param device_id Identifier of the device to get the voltage of.
     * \param [out] voltage Voltage in mV.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM One or more parameters were invalid.
     * \retval FWK_E_STATE The element cannot accept the request.
     * \retval FWK_E_HANDLER An error occurred in the device driver.
     */
    int (*get_voltage)(fwk_id_t device_id, uintmax_t *voltage);

    /*!
     * \brief Set the voltage of a device.
     *
     * \param device_id Identifier of the device to set the voltage of.
     * \param voltage New voltage in mV.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM One or more parameters were invalid.
     * \retval FWK_E_STATE The element cannot accept the request.
     * \retval FWK_E_HANDLER An error occurred in the device driver.
     */
    int (*set_voltage)(fwk_id_t device_id, uintmax_t voltage);

    /*!
     * \brief Set the voltage of a device.
     *
     * \param device_id Identifier of the device to set the voltage of.
     * \param voltage New voltage in mV.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM One or more parameters were invalid.
     * \retval FWK_E_STATE The element cannot accept the request.
     * \retval FWK_E_NOMEM The event queue is full.
     * \retval FWK_E_PANIC An error in the framework occurred.
     */
    int (*set_voltage_async)(fwk_id_t device_id, uintmax_t voltage);
};

/*!
 * \brief Driver API.
 */
struct mod_psu_driver_api {
    /*!
     * \brief Set the enabled state of a device.
     *
     * \param id Identifier of the device to set the state of.
     * \param enable \c true to enable the device, or \c false to disable it.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the other driver-defined error codes.
     */
    int (*set_enabled)(fwk_id_t id, bool enable);

    /*!
     * \brief Get the enabled state of a device.
     *
     * \param id Identifier of the device to get the state of.
     * \param [out] enabled \c true if the device is enabled, or \c if it is
     *      disabled.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the other driver-defined error codes.
     */
    int (*get_enabled)(fwk_id_t id, bool *enabled);

    /*!
     * \brief Set the voltage of a device.
     *
     * \param id Identifier of the device to set the voltage of.
     * \param voltage New voltage in millivolts (mV).
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the other driver-defined error codes.
     */
    int (*set_voltage)(fwk_id_t id, uintmax_t voltage);

    /*!
     * \brief Get the voltage of a device.
     *
     * \param id Identifier of the device to get the voltage of.
     * \param [out] voltage Voltage in millivolts (mV).
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the other driver-defined error codes.
     */
    int (*get_voltage)(fwk_id_t id, uintmax_t *voltage);
};

/*!
 * \}
 */

/*!
 * \defgroup GroupPsuConfig Configuration
 * \{
 */

/*!
 * \brief Device configuration.
 */
struct mod_psu_device_config {
    fwk_id_t driver_id; /*!< Driver identifier */
    fwk_id_t driver_api_id; /*!< Driver API identifier */
};

/*!
 * \}
 */

/*!
 * \defgroup GroupPsuEvents Events
 * \{
 */

/*!
 * \brief <tt>Set enabled</tt> event response parameters.
 */
struct mod_psu_event_params_set_enabled_response {
    int status; /*!< Status of the request */
};

/*!
 * \brief <tt>Set voltage</tt> event response parameters.
 */
struct mod_psu_event_params_set_voltage_response {
    int status; /*!< Status of the request */
};

/*!
 * \}
 */

/*!
 * \defgroup GroupPsuIds Identifiers
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_psu_api_idx {
    /*! API index for mod_psu_api_id_psu_device */
    MOD_PSU_API_IDX_PSU_DEVICE,

    /*! Number of defined APIs */
    MOD_PSU_API_IDX_COUNT
};

/*! Device API identifier */
static const fwk_id_t mod_psu_api_id_psu_device =
    FWK_ID_API_INIT(FWK_MODULE_IDX_PSU, MOD_PSU_API_IDX_PSU_DEVICE);

/*!
 * \brief Event indices.
 */
enum mod_psu_event_idx {
    /*! Event index for mod_psu_event_id_set_enabled */
    MOD_PSU_EVENT_IDX_SET_ENABLED,

    /*! Event index for mod_psu_event_id_set_voltage */
    MOD_PSU_EVENT_IDX_SET_VOLTAGE,

    /*! Number of defined events */
    MOD_PSU_EVENT_IDX_COUNT
};

/*! <tt>Set enabled</tt> event identifier */
static const fwk_id_t mod_psu_event_id_set_enabled =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_PSU, MOD_PSU_EVENT_IDX_SET_ENABLED);

/*! <tt>Set voltage</tt> event identifier */
static const fwk_id_t mod_psu_event_id_set_voltage =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_PSU, MOD_PSU_EVENT_IDX_SET_VOLTAGE);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_PSU_H */

/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_PMIC_H
#define MOD_RCAR_PMIC_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \ingroup GroupRCARModule
 * \defgroup GroupRCARPmic PMIC
 * \{
 */

/*!
 * \defgroup GroupRCARPmicApis APIs
 * \{
 */

/*!
 * \brief Device API.
 */
struct mod_rcar_pmic_device_api {
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
    int (*get_voltage)(fwk_id_t device_id, uint64_t *voltage);

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
    int (*set_voltage)(fwk_id_t device_id, uint64_t voltage);

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
    int (*set_voltage_async)(fwk_id_t device_id, uint64_t voltage);
};

/*!
 * \brief Driver API.
 */
struct mod_rcar_pmic_driver_api {
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
    int (*set_voltage)(fwk_id_t id, uint64_t voltage);

    /*!
     * \brief Get the voltage of a device.
     *
     * \param id Identifier of the device to get the voltage of.
     * \param [out] voltage Voltage in millivolts (mV).
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the other driver-defined error codes.
     */
    int (*get_voltage)(fwk_id_t id, uint64_t *voltage);
};

/*!
 * \}
 */

/*!
 * \defgroup GroupRCARPmicConfig Configuration
 * \{
 */

/*!
 * \brief Device configuration.
 */
struct mod_rcar_pmic_device_config {
    fwk_id_t driver_id; /*!< Driver identifier */
    fwk_id_t driver_api_id; /*!< Driver API identifier */
};

/*!
 * \}
 */

/*!
 * \defgroup GroupRCARPmicEvents Events
 * \{
 */

/*!
 * \brief <tt>Set enabled</tt> event response parameters.
 */
struct mod_rcar_pmic_event_params_set_enabled_response {
    int status; /*!< Status of the request */
};

/*!
 * \brief <tt>Set voltage</tt> event response parameters.
 */
struct mod_rcar_pmic_event_params_set_voltage_response {
    int status; /*!< Status of the request */
};

/*!
 * \}
 */

/*!
 * \defgroup GroupRCARPmicIds Identifiers
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_rcar_pmic_api_idx {
    /*! API index for mod_rcar_pmic_api_id_device */
    MOD_RCAR_PMIC_API_IDX_DEVICE,

    /*! Number of defined APIs */
    MOD_RCAR_PMIC_API_IDX_COUNT
};

/*! Device API identifier */
static const fwk_id_t mod_rcar_pmic_api_id_device =
    FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PMIC, MOD_RCAR_PMIC_API_IDX_DEVICE);

/*!
 * \brief Event indices.
 */
enum mod_rcar_pmic_event_idx {
    /*! Event index for mod_rcar_pmic_event_id_set_enabled */
    MOD_RCAR_PMIC_EVENT_IDX_SET_ENABLED,

    /*! Event index for mod_rcar_pmic_event_id_set_voltage */
    MOD_RCAR_PMIC_EVENT_IDX_SET_VOLTAGE,

    /*! Number of defined events */
    MOD_RCAR_PMIC_EVENT_IDX_COUNT
};

/*! <tt>Set enabled</tt> event identifier */
static const fwk_id_t mod_rcar_pmic_event_id_set_enabled =
    FWK_ID_EVENT_INIT(
        FWK_MODULE_IDX_RCAR_PMIC,
        MOD_RCAR_PMIC_EVENT_IDX_SET_ENABLED);

/*! <tt>Set voltage</tt> event identifier */
static const fwk_id_t mod_rcar_pmic_event_id_set_voltage =
     FWK_ID_EVENT_INIT(
         FWK_MODULE_IDX_RCAR_PMIC,
         MOD_RCAR_PMIC_EVENT_IDX_SET_VOLTAGE);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_RCAR_PMIC_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
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
 * \{
 */

/*!
 * \brief Device API.
 *
 * \details The PSU device API represents the abstract interface used to control
 *      individual power supplies.
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
     * \retval ::FWK_SUCCESS The operation succeeded.
     *
     * \return Status code representing the result of the operation.
     */
    int (*set_voltage)(fwk_id_t id, uint64_t voltage);
};

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
 * \brief API indices.
 */
enum mod_psu_api_idx {
    /*!
     * \brief Device API index.
     *
     * \details This API is the abstract power supply interface exposed and
     *      implemented by this module.
     *
     * \note This API identifier implements the ::mod_psu_device_api interface.
     *
     * \warning Binding to this API must occur through an element of this
     *      module.
     */
    MOD_PSU_API_IDX_DEVICE,

    /*!
     * \brief Number of defined APIs.
     */
    MOD_PSU_API_IDX_COUNT
};

/*!
 * \brief Device API identifier.
 *
 * \note This identifier corresponds to the ::MOD_PSU_API_IDX_DEVICE API index.
 */
static const fwk_id_t mod_psu_api_id_device =
    FWK_ID_API_INIT(FWK_MODULE_IDX_PSU, MOD_PSU_API_IDX_DEVICE);

/*!
 * \}
 */

#endif /* MOD_PSU_H */

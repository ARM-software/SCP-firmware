/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MOCK_VOLTAGE_DOMAIN_H
#define MOD_MOCK_VOLTAGE_DOMAIN_H

#include <mod_voltage_domain.h>

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules
 * \defgroup GroupMockPsu Mock Power Supply Driver
 *
 * \details The `mock_voltage_domain` module provides a mock power supply driver
 *      for use alongside the `mock_voltage_domain` interface on systems that do
 *      not provide a real power supply.
 *
 *      In addition to the standard synchronous mode of operation, mock PSUs
 *      support an emulated form of asynchronous operation. This mode reproduces
 *      the behavior of some real devices which cannot respond to requests
 *      immediately, and is useful for testing interfaces that must support
 *      devices of this kind.
 *
 *      To enable the asynchronous operation mode, see the documentation for
 *      ::mod_mock_voltage_domain_element_cfg::async_alarm_id.
 *
 * \{
 */

/*!
 * \brief Driver response.
 *
 * \details This structure is passed to the `mock_voltage_domain` module through
 *      the [driver response
 *      API](::mod_mock_voltage_domain_driver_response_api::respond).
 */
struct mod_mock_voltage_domain_driver_response {
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
         *      ::mod_voltd_drv_api::get_config call.
         */
        enum mod_voltd_mode_id mode_id;

        /*!
         * \brief Voltage in microvolts (uV).
         *
         * \warning Used only in response to a
         *      ::mod_voltd_drv_api::get_level call.
         */
        int32_t voltage;
    };
};

/*!
 * \brief Driver response API.
 *
 * \details The driver response API is the API used by drivers to communicate
 *      the result of a previously-pended operation.
 */
struct mod_mock_voltage_domain_driver_response_api {
    /*!
     * \brief Respond to a previous driver request.
     *
     * \param element_id Identifier of the power supply element which submitted
     *      the request to the driver.
     * \param response Driver operation response.
     */
    void (*respond)(
        fwk_id_t element_id,
        struct mod_mock_voltage_domain_driver_response response);
};

/*!
 * \brief Element configuration.
 */
struct mod_mock_voltage_domain_element_cfg {
    /*!
     * \brief Alarm entity identifier used for asynchronous driver emulation.
     *
     * \details This identifies the entity of a timer alarm, which must
     *      implement ::mod_timer_alarm_api.
     *
     * \note This field may be set to ::FWK_ID_NONE, in which case asynchronous
     *      driver emulation is not enabled.
     */
    fwk_id_t async_alarm_id;

    /*!
     * \brief Alarm API identifier used for asynchronous driver emulation.
     *
     * \details This identifies the API of a timer alarm, which must
     *      implement ::mod_timer_alarm_api.
     *
     * \note This field may be set to ::FWK_ID_NONE if asynchronous driver
     *      emulation is not enabled.
     */
    fwk_id_t async_alarm_api_id;

    /*!
     * \brief Driver response entity identifier used for asynchronous driver
     *      emulation.
     *
     * \details This identifies the entity of a driver response entity, which
     *      must implement ::mod_mock_voltage_domain_driver_response_api.
     *
     * \note This field may be set to ::FWK_ID_NONE if asynchronous driver
     *      emulation is not enabled.
     */
    fwk_id_t async_response_id;

    /*!
     * \brief Driver response API identifier used for asynchronous driver
     *      emulation.
     *
     * \details This identifies the API of a driver response entity, which must
     *      implement ::mod_mock_voltage_domain_driver_response_api.
     *
     * \note This field may be set to ::FWK_ID_NONE if asynchronous driver
     *      emulation is not enabled.
     */
    fwk_id_t async_response_api_id;

    /*!
     * \brief Default mode id of the mock device's supply (enabled or disabled).
     */
    enum mod_voltd_mode_id default_mode_id;

    /*!
     * \brief Default voltage, in microvolts (uV), of the device's supply.
     */
    uint32_t default_voltage;

    /*!
     * \brief The type of level range description provided (discrete or
     *      continuous).
     */
    enum mod_voltd_voltage_level_type level_type;

    /*!
     * \brief The number of unique voltage levels that the domain can attain.
     */
    size_t level_count;

    /*!
     * \brief Array of voltage values, in microvolts(uV), that the PSU can
     *      attain.
     */
    const int32_t *voltage_levels;
};

/*!
 * \brief API indices.
 */
enum mod_mock_voltage_domain_api_idx {
    /*!
     * \brief Driver API index.
     *
     * \note This API implements the ::mod_voltd_drv_api interface.
     *
     * \warning Binding to this API must occur through an element of this
     *      module.
     */
    MOD_MOCK_VOLTAGE_DOMAIN_API_IDX_VOLTD,

    /*!
     * \brief Number of defined APIs.
     */
    MOD_MOCK_VOLTAGE_DOMAIN_API_IDX_COUNT
};

/*!
 * \brief Voltage Domain driver module descriptor.
 */
extern const struct fwk_module module_mock_voltage_domain;

/*!
 * \}
 */

#endif /* MOD_MOCK_VOLTAGE_DOMAIN_H */

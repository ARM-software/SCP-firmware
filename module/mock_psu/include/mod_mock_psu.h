/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MOCK_PSU_H
#define MOD_MOCK_PSU_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules
 * \defgroup GroupMockPsu Mock Power Supply Driver
 *
 * \details The `mock_psu` module provides a mock power supply driver for use
 *      alongside the `psu` interface on systems that do not provide a real
 *      power supply.
 *
 *      In addition to the standard synchronous mode of operation, mock PSUs
 *      support an emulated form of asynchronous operation. This mode reproduces
 *      the behaviour of some real devices which cannot respond to requests
 *      immediately, and is useful for testing interfaces that must support
 *      devices of this kind.
 *
 *      To enable the asynchronous operation mode, see the documentation for
 *      ::mod_mock_psu_element_cfg::async_alarm_id.
 *
 * \{
 */

/*!
 * \brief Element configuration.
 */
struct mod_mock_psu_element_cfg {
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
     *      must implement ::mod_psu_driver_response_api.
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
     *      implement ::mod_psu_driver_response_api.
     *
     * \note This field may be set to ::FWK_ID_NONE if asynchronous driver
     *      emulation is not enabled.
     */
    fwk_id_t async_response_api_id;

    /*!
     * \brief Default state of the mock device's supply (enabled or disabled).
     */
    bool default_enabled;

    /*!
     * \brief Default voltage, in millivolts (mV), of the device's supply.
     */
    uint64_t default_voltage;
};

/*!
 * \brief API indices.
 */
enum mod_mock_psu_api_idx {
    /*!
     * \brief Driver API index.
     *
     * \note This API implements the ::mod_psu_driver_api interface.
     *
     * \warning Binding to this API must occur through an element of this
     *      module.
     */
    MOD_MOCK_PSU_API_IDX_DRIVER,

    /*!
     * \brief Number of defined APIs.
     */
    MOD_MOCK_PSU_API_IDX_COUNT
};

/*!
 * \brief Driver API identifier.
 *
 * \note This identifier corresponds to the ::MOD_MOCK_PSU_API_IDX_DRIVER API
 *      index.
 */
static const fwk_id_t mod_mock_psu_api_id_driver =
    FWK_ID_API_INIT(FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_API_IDX_DRIVER);

/*!
 * \}
 */

#endif /* MOD_MOCK_PSU_H */

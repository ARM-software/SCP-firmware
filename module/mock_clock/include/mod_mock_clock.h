/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MOCK_CLOCK_H
#define MOD_MOCK_CLOCK_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules
 * \defgroup GroupMockClock Mock Clock Driver
 *
 * \details The `mock_clock` module provides a mock clock driver for use
 *      alongside the `clock` interface on systems that do not provide a real
 *      clock driver.
 *
 * \warning When using this module to mock a pre-existing driver
 *      for any reason, this driver must be forced to bind to the
 *      ::MOD_MOCK_CLOCK_API_TYPE_RESPONSE_DRIVER API through its module
 *      configuration. This will avoid any conflict between the pre-existing
 *      driver and the mocked one.
 * \{
 */

/*!
 * \brief Rate lookup table entry.
 */
struct mod_mock_clock_rate {
    /*! Rate of the clock in Hertz. */
    uint32_t rate;

    /*! The clock divider used to attain the rate. */
    unsigned int divider;
};

/*!
 * \brief Element configuration.
 */
struct mod_mock_clock_element_cfg {
    /*! Pointer to the clock's rate lookup table. */
    const struct mod_mock_clock_rate *rate_table;

    /*! The number of rates in the rate lookup table. */
    unsigned int rate_count;

    /*! The default rate value if the clock device is running at startup. */
    uint32_t default_rate;
};

/*!
 * \brief API indices.
 *
 * \warning The mock clock implements the clock driver API only. The response
 *      driver API only acts as a pointer to be bound to.
 */
enum mod_mock_clock_api_type {
    /*!
     * \brief Clock driver.
     *
     * \note This API implements the mod_clock::mod_clock_driver_api interface.
     *
     * \warning Binding to this API must occur through an element of this
     *      module.
     */
    MOD_MOCK_CLOCK_API_TYPE_DRIVER,

    /*!
     * \brief Clock driver response.
     *
     * \note This API implements the mod_clock::mod_clock_driver_response_api
     * interface.
     *
     * \warning Binding to this API must occur through an element of this
     *      module.
     */
    MOD_MOCK_CLOCK_API_TYPE_RESPONSE_DRIVER,

    /*!
     * \brief Number of defined APIs.
     */
    MOD_MOCK_CLOCK_API_COUNT,
};

/*!
 * \brief Driver API identifier.
 *
 * \note This identifier corresponds to the ::MOD_MOCK_CLOCK_API_TYPE_DRIVER API
 *      index.
 */
static const fwk_id_t mod_mock_clock_api_id_driver =
    FWK_ID_API_INIT(FWK_MODULE_IDX_MOCK_CLOCK, MOD_MOCK_CLOCK_API_TYPE_DRIVER);

/*!
 * \}
 */

#endif /* MOD_MOCK_CLOCK_H */

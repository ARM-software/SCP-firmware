/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MOCK_PSU_H
#define MOD_MOCK_PSU_H

#include <stdbool.h>
#include <stdint.h>
#include <fwk_module_idx.h>

/*!
 * \ingroup GroupModules
 * \defgroup GroupMockPsu Mock Power Supply Driver
 * \{
 */

/*!
 * \defgroup GroupMockPsuConfig Configuration
 * \{
 */

/*!
 * \brief Element configuration.
 */
struct mod_mock_psu_device_config {
    /*! Default state of the mock device's supply (enabled or disabled) */
    bool default_enabled;

    /*! Default voltage, in millivolts (mV), of the device's supply */
    uint64_t default_voltage;
};

/*!
 * \}
 */

/*!
 * \defgroup GroupMockPsuIds Identifiers
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_mock_psu_api_idx {
    /*! API index for PSU driver API */
    MOD_MOCK_PSU_API_IDX_PSU_DRIVER,

    /*! Number of defined APIs */
    MOD_MOCK_PSU_API_COUNT
};

/*! Driver API identifier */
static const fwk_id_t mod_mock_psu_api_id_psu_driver =
    FWK_ID_API_INIT(FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_API_IDX_PSU_DRIVER);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_MOCK_PSU_H */

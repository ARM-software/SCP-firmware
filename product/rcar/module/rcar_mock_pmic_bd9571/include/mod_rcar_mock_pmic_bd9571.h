/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_MOCK_PMIC_BD9571_H
#define MOD_RCAR_MOCK_PMIC_BD9571_H

#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \ingroup GroupRCARModule
 * \defgroup GroupRCARMockPmic Mock Power Supply Driver
 * \{
 */

/*!
 * \defgroup GroupRCARMockPmicConfig Configuration
 * \{
 */

/*!
 * \brief Element configuration.
 */
struct mod_rcar_mock_pmic_device_config {
    /*! Default state of the mock device's supply (enabled or disabled) */
    bool default_enabled;

    /*! Default voltage, in millivolts (mV), of the device's supply */
    uint64_t default_voltage;
};

/*!
 * \}
 */

/*!
 * \defgroup GroupRCARMockPmicIds Identifiers
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_rcar_mock_pmic_api_idx {
    /*! API index for PSU driver API */
    MOD_RCAR_MOCK_PMIC_API_IDX_PSU_DRIVER,

    /*! Number of defined APIs */
    MOD_RCAR_MOCK_PMIC_API_COUNT
};

/*! Driver API identifier */
static const fwk_id_t mod_rcar_mock_pmic_api_id_psu_driver = FWK_ID_API_INIT(
    FWK_MODULE_IDX_RCAR_MOCK_PMIC_BD9571,
    MOD_RCAR_MOCK_PMIC_API_IDX_PSU_DRIVER);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_RCAR_MOCK_PMIC_BD9571_H */

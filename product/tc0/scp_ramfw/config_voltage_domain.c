/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_mock_voltage_domain.h>

#include <mod_mock_voltage_domain.h>
#include <mod_voltage_domain.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static const struct fwk_element voltage_domain_element_table[] = {
    [0] = {
        .name = "DUMMY_VOLTD",
        .data = &((const struct mod_voltd_dev_config){
        .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_VOLTAGE_DOMAIN,
                CONFIG_MOCK_VOLTAGE_DOMAIN_ELEMENT_IDX_DUMMY),
        .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MOCK_VOLTAGE_DOMAIN,
                MOD_MOCK_VOLTAGE_DOMAIN_API_IDX_VOLTD),
        }),
    },

    [1] = { 0 }, /* Termination description */
};

static const struct fwk_element *voltage_domain_get_element_table(
    fwk_id_t module_id)
{
    return voltage_domain_element_table;
}

const struct fwk_module_config config_voltage_domain = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(voltage_domain_get_element_table),
};

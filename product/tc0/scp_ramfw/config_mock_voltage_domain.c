/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <config_mock_voltage_domain.h>

#include <mod_mock_voltage_domain.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <stdbool.h>
#include <stddef.h>

static const int32_t dummy_discrete_voltage_values[] = {
    -500000,
    800000,
    10000,
};

static const struct fwk_element element_table[] = {
    [CONFIG_MOCK_VOLTAGE_DOMAIN_ELEMENT_IDX_DUMMY] = {
        .name = "DUMMY_VOLTD",
        .data = &(const struct mod_mock_voltage_domain_element_cfg) {
            .async_alarm_id = FWK_ID_NONE_INIT,
            .async_alarm_api_id = FWK_ID_NONE_INIT,

            .async_response_id = FWK_ID_NONE_INIT,
            .async_response_api_id = FWK_ID_NONE_INIT,

            .default_mode_id = MOD_VOLTD_MODE_ID_OFF,
            .default_voltage = 500000,

            .level_type = MOD_VOLTD_VOLTAGE_LEVEL_CONTINUOUS,
            .level_count = FWK_ARRAY_SIZE(dummy_discrete_voltage_values),
            .voltage_levels = dummy_discrete_voltage_values,
        },
    },
    { 0 }
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_mock_voltage_domain = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};

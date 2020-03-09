/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_mock_psu.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <stdbool.h>
#include <stddef.h>

static const struct fwk_element element_table[] = {
    [0] = {
        .name = "",
        .data = &(const struct mod_mock_psu_element_cfg) {
            .async_alarm_id = FWK_ID_NONE_INIT,
            .default_enabled = true,
            .default_voltage = 900,
        },
    },
    [1] = {
        .name = "",
        .data = &(const struct mod_mock_psu_element_cfg) {
            .async_alarm_id = FWK_ID_NONE_INIT,
            .default_enabled = true,
            .default_voltage = 900,
        },
    },
    [2] = {
        .name = "",
        .data = &(const struct mod_mock_psu_element_cfg) {
            .async_alarm_id = FWK_ID_NONE_INIT,
            .default_enabled = true,
            .default_voltage = 900,
        },
    },
    [3] = {
        .name = "",
        .data = &(const struct mod_mock_psu_element_cfg) {
            .async_alarm_id = FWK_ID_NONE_INIT,
            .default_enabled = true,
            .default_voltage = 850,
        },
    },
    [4] = { 0 }
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_mock_psu = {
    .get_element_table = get_element_table,
    .data = NULL,
};

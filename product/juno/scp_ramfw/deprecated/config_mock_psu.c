/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_mock_psu.h"

#include <mod_mock_psu.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <stdbool.h>
#include <stddef.h>

static const struct fwk_element element_table[] = {
    [MOD_MOCK_PSU_ELEMENT_IDX_VSYS] = {
        .name = "VSYS",
        .data = &(const struct mod_mock_psu_element_cfg) {
            .async_alarm_id = FWK_ID_NONE_INIT,
            .default_enabled = true,
            .default_voltage = 900,
        },
    },
    [MOD_MOCK_PSU_ELEMENT_IDX_VBIG] = {
        .name = "VBIG",
        .data = &(const struct mod_mock_psu_element_cfg) {
            .async_alarm_id = FWK_ID_NONE_INIT,
            .default_enabled = true,
            .default_voltage = 900,
        },
    },
    [MOD_MOCK_PSU_ELEMENT_IDX_VLITTLE] = {
        .name = "VLITTLE",
        .data = &(const struct mod_mock_psu_element_cfg) {
            .async_alarm_id = FWK_ID_NONE_INIT,
            .default_enabled = true,
            .default_voltage = 900,
        },
    },
    [MOD_MOCK_PSU_ELEMENT_IDX_VGPU] = {
        .name = "VGPU",
        .data = &(const struct mod_mock_psu_element_cfg) {
            .async_alarm_id = FWK_ID_NONE_INIT,
            .default_enabled = true,
            .default_voltage = 850,
        },
    },
    [MOD_MOCK_PSU_ELEMENT_IDX_COUNT] = { 0 }
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_mock_psu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};

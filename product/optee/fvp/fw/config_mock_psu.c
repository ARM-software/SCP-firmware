/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_mock_psu.h"

#include <mod_mock_psu.h>

#include <fwk_module.h>

static const struct fwk_element element_table[] = {
    [CONFIG_MOCK_PSU_ELEMENT_IDX_CPU_GROUP_LITTLE] = {
        .name = "CPU_GROUP_LITTLE",
        .data = &(const struct mod_mock_psu_element_cfg) {
            .default_enabled = true,
            .default_voltage = 100,
            .async_alarm_id =  FWK_ID_NONE_INIT,
        },
    },
    [CONFIG_MOCK_PSU_ELEMENT_IDX_CPU_GROUP_BIG] = {
        .name = "CPU_GROUP_BIG",
        .data = &(const struct mod_mock_psu_element_cfg) {
            .default_enabled = true,
            .default_voltage = 100,
            .async_alarm_id = FWK_ID_NONE_INIT,
        },
    },
    [CONFIG_MOCK_PSU_ELEMENT_IDX_GPU] = {
        .name = "GPU",
        .data = &(const struct mod_mock_psu_element_cfg) {
            .default_enabled = true,
            .default_voltage = 100,
            .async_alarm_id = FWK_ID_NONE_INIT,
        },
    },
    [CONFIG_MOCK_PSU_ELEMENT_IDX_VPU] = {
        .name = "VPU",
        .data = &(const struct mod_mock_psu_element_cfg) {
            .default_enabled = true,
            .default_voltage = 100,
            .async_alarm_id = FWK_ID_NONE_INIT,
        },
    },
    { 0 }
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_mock_psu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};

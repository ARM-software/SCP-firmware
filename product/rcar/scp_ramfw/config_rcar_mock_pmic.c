/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_element.h>
#include <fwk_module.h>
#include <mod_rcar_mock_pmic.h>

static const struct fwk_element element_table[] = {
    {
        .name = "CPU_GROUP_LITTLE",
        .data = &(const struct mod_rcar_mock_pmic_device_config) {
            .default_enabled = true,
            .default_voltage = 820000,
        },
    },
    {
        .name = "CPU_GROUP_BIG",
        .data = &(const struct mod_rcar_mock_pmic_device_config) {
            .default_enabled = true,
            .default_voltage = 830000,
        },
    },
    {
        .name = "GPU",
        .data = &(const struct mod_rcar_mock_pmic_device_config) {
            .default_enabled = true,
            .default_voltage = 100,
        },
    },
    {
        .name = "VPU",
        .data = &(const struct mod_rcar_mock_pmic_device_config) {
            .default_enabled = true,
            .default_voltage = 100,
        },
    },
    { 0 }
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_rcar_mock_pmic = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
    .data = NULL,
};

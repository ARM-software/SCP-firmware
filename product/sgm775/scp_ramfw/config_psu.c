/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_mock_psu.h"
#include "config_psu.h"

#include <mod_mock_psu.h>
#include <mod_psu.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>

static const struct fwk_element element_table[] = {
    [CONFIG_PSU_ELEMENT_IDX_CPU_GROUP_LITTLE] = {
        .name = "CPU_GROUP_LITTLE",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PSU,
                CONFIG_MOCK_PSU_ELEMENT_IDX_CPU_GROUP_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MOCK_PSU,
                MOD_MOCK_PSU_API_IDX_DRIVER),
        },
    },
    [CONFIG_PSU_ELEMENT_IDX_CPU_GROUP_BIG] = {
        .name = "CPU_GROUP_BIG",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PSU,
                CONFIG_MOCK_PSU_ELEMENT_IDX_CPU_GROUP_BIG),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MOCK_PSU,
                MOD_MOCK_PSU_API_IDX_DRIVER),
        },
    },
    [CONFIG_PSU_ELEMENT_IDX_GPU] = {
        .name = "GPU",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PSU,
                CONFIG_MOCK_PSU_ELEMENT_IDX_GPU),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MOCK_PSU,
                MOD_MOCK_PSU_API_IDX_DRIVER),
        },
    },
    [CONFIG_PSU_ELEMENT_IDX_VPU] = {
        .name = "VPU",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PSU,
                CONFIG_MOCK_PSU_ELEMENT_IDX_VPU),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MOCK_PSU,
                MOD_MOCK_PSU_API_IDX_DRIVER),
        },
    },
    { 0 }
};

static const struct fwk_element *psu_get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_psu = {
    .get_element_table = psu_get_element_table,
    .data = NULL,
};

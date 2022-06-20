/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_mock_psu.h>
#include <config_psu.h>
#include <config_xr77128.h>

#include <mod_mock_psu.h>
#include <mod_psu.h>
#include <mod_xr77128.h>

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element
    element_table[CONFIG_PSU_ELEMENT_IDX_COUNT + 1] = {
            [CONFIG_PSU_ELEMENT_IDX_CLUS0] = {
                .name = "CLUSTER_0_CPUS",
                .data = &(const struct mod_psu_element_cfg) {
                    .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                        FWK_MODULE_IDX_XR77128,
                        CONFIG_XR77128_ELEMENT_IDX_0,
                        CONFIG_PSU_ELEMENT_IDX_CLUS0),
                    .driver_api_id = FWK_ID_API_INIT(
                        FWK_MODULE_IDX_XR77128,
                        MOD_XR77128_API_IDX_PSU),
                },
            },
            [CONFIG_PSU_ELEMENT_IDX_CLUS1] = {
                .name = "CLUSTER_1_CPUS",
                .data = &(const struct mod_psu_element_cfg) {
                    .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                        FWK_MODULE_IDX_XR77128,
                        CONFIG_XR77128_ELEMENT_IDX_0,
                        CONFIG_PSU_ELEMENT_IDX_CLUS1),
                    .driver_api_id = FWK_ID_API_INIT(
                        FWK_MODULE_IDX_XR77128,
                        MOD_XR77128_API_IDX_PSU),
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
            { 0 },
};

static const struct fwk_element *psu_get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_psu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(psu_get_element_table),
    .data = NULL,
};

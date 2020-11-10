/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_rcar_mock_pmic.h>
#include <mod_rcar_pmic.h>

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element element_table[] = {
    {
        .name = "CPU_GROUP_LITTLE",
        .data =
            &(const struct mod_rcar_pmic_device_config){
                .driver_id =
                    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MOCK_PMIC, 0),
                .driver_api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_RCAR_MOCK_PMIC,
                    MOD_RCAR_MOCK_PMIC_API_IDX_PSU_DRIVER) },
    },
    {
        .name = "CPU_GROUP_BIG",
        .data =
            &(const struct mod_rcar_pmic_device_config){
                .driver_id =
                    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MOCK_PMIC, 1),
                .driver_api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_RCAR_MOCK_PMIC,
                    MOD_RCAR_MOCK_PMIC_API_IDX_PSU_DRIVER) },
    },
    {
        .name = "GPU",
        .data =
            &(const struct mod_rcar_pmic_device_config){
                .driver_id =
                    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_MOCK_PMIC, 2),
                .driver_api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_RCAR_MOCK_PMIC,
                    MOD_RCAR_MOCK_PMIC_API_IDX_PSU_DRIVER) },
    },
    { 0 }
};

static const struct fwk_element *rcar_pmic_get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_rcar_pmic = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(rcar_pmic_get_element_table),
    .data = NULL,
};

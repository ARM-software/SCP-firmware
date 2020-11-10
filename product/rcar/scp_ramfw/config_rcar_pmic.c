/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_rcar_pmic.h>
#include <config_rcar_mock_pmic_bd9571.h>

#include <mod_rcar_mock_pmic_bd9571.h>
#include <mod_rcar_pmic.h>

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element element_table[] = {
    [MOD_RCAR_PMIC_ELEMENT_IDX_LITTLE] = {
        .name = "CPU_GROUP_LITTLE",
        .data =
            &(const struct mod_rcar_pmic_device_config){
                .driver_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_RCAR_MOCK_PMIC_BD9571,
                    MOD_RCAR_MOCK_PMIC_ELEMENT_IDX_LITTLE),
                .driver_api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_RCAR_MOCK_PMIC_BD9571,
                    MOD_RCAR_MOCK_PMIC_API_IDX_PSU_DRIVER) },
    },
    [MOD_RCAR_PMIC_ELEMENT_IDX_BIG] = {
        .name = "CPU_GROUP_BIG",
        .data =
            &(const struct mod_rcar_pmic_device_config){
                .driver_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_RCAR_MOCK_PMIC_BD9571,
                    MOD_RCAR_MOCK_PMIC_ELEMENT_IDX_BIG),
                .driver_api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_RCAR_MOCK_PMIC_BD9571,
                    MOD_RCAR_MOCK_PMIC_API_IDX_PSU_DRIVER) },
    },
    [MOD_RCAR_PMIC_ELEMENT_IDX_DDR_BKUP] = {
        .name = "PMIC_DDR_BKUP",
        .data =
            &(const struct mod_rcar_pmic_device_config){
                .driver_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_RCAR_MOCK_PMIC_BD9571,
                    RCAR_PMIC_CPU_DDR_BKUP),
                .driver_api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_RCAR_MOCK_PMIC_BD9571,
                    MOD_RCAR_MOCK_PMIC_API_IDX_PSU_DRIVER) },
    },
    [MOD_RCAR_PMIC_ELEMENT_IDX_GPU] = {
        .name = "GPU",
        .data =
            &(const struct mod_rcar_pmic_device_config){
                .driver_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_RCAR_MOCK_PMIC_BD9571,
                    MOD_RCAR_MOCK_PMIC_ELEMENT_IDX_GPU),
                .driver_api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_RCAR_MOCK_PMIC_BD9571,
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

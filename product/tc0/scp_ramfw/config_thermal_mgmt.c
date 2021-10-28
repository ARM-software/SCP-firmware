/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <tc0_dvfs.h>

#include <mod_tc0_power_model.h>
#include <mod_thermal_mgmt.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <stddef.h>
#include <stdint.h>

static const struct fwk_element thermal_mgmt_elem_table[] = {
    [0] = {
        .name = "Thermal Actor 0",
        .data = &((struct mod_thermal_mgmt_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TC0_POWER_MODEL, 0),
            .driver_api_id =
                FWK_ID_API_INIT(
                    FWK_MODULE_IDX_TC0_POWER_MODEL,
                    MOD_TC0_POWER_MODEL_THERMAL_DRIVER_API_IDX),
            .dvfs_domain_id =
                FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_KLEIN),
            .weight = 100,
        }),
    },
    [1] = {
        .name = "Thermal Actor 1",
        .data = &((struct mod_thermal_mgmt_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TC0_POWER_MODEL, 1),
            .driver_api_id =
                FWK_ID_API_INIT(
                    FWK_MODULE_IDX_TC0_POWER_MODEL,
                    MOD_TC0_POWER_MODEL_THERMAL_DRIVER_API_IDX),
            .dvfs_domain_id =
                FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_MATTERHORN),
            .weight = 100,
        }),
    },
    [2] = {
        .name = "Thermal Actor 2",
        .data = &((struct mod_thermal_mgmt_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TC0_POWER_MODEL, 2),
            .driver_api_id =
                FWK_ID_API_INIT(
                    FWK_MODULE_IDX_TC0_POWER_MODEL,
                    MOD_TC0_POWER_MODEL_THERMAL_DRIVER_API_IDX),
            .dvfs_domain_id =
                FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_MATTERHORN_ELP_ARM),
            .weight = 100,
        }),
    },

    [3] = { 0 } /* Termination description */
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return thermal_mgmt_elem_table;
}

struct fwk_module_config config_thermal_mgmt = {
    .data =
        &(struct mod_thermal_mgmt_config){
            .slow_loop_mult = 25,
            .tdp = 10,
            .switch_on_temperature = 50,
            .control_temperature = 60,
            .integral_cutoff = 0,
            .integral_max = 100,
            .k_p_undershoot = 1,
            .k_p_overshoot = 1,
            .k_integral = 1,
            .sensor_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR, 0),
        },
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};

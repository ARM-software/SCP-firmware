/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <tc2_dvfs.h>

#include <mod_tc2_power_model.h>
#include <mod_thermal_mgmt.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <stddef.h>
#include <stdint.h>

static struct mod_thermal_mgmt_actor_config actor_table_domain0[2] = {
    [0] = {
        .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TC2_POWER_MODEL, 0),
        .dvfs_domain_id =
            FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_CORTEX_A520),
        .weight = 100,
    },
    [1] = {
        .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TC2_POWER_MODEL, 1),
        .dvfs_domain_id =
            FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_CORTEX_A720),
        .weight = 100,
    },
};

static const struct fwk_element thermal_mgmt_domains_elem_table[2] = {
    [0] = {
        .name = "Thermal Domain 0",
        .data = &((struct mod_thermal_mgmt_dev_config){
            .slow_loop_mult = 25,
            .tdp = 10,
            .cold_state_power = 11,
            .pid_controller = {
                .switch_on_temperature = 50,
                .control_temperature = 60,
                .integral_cutoff = 0,
                .integral_max = 100,
                .k_p_undershoot = 1,
                .k_p_overshoot = 1,
                .k_integral = 1,
            },
            .sensor_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR, 0),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TC2_POWER_MODEL,
                MOD_TC2_POWER_MODEL_THERMAL_DRIVER_API_IDX),
            .thermal_actors_table = actor_table_domain0,
            .thermal_actors_count = FWK_ARRAY_SIZE(actor_table_domain0),
        }),
    },
    [1] = { 0 } /* Termination description */
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return thermal_mgmt_domains_elem_table;
}

struct fwk_module_config config_thermal_mgmt = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_thermal_mgmt.h>
#include <mod_thermal_mgmt_extra.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

enum fake_thermal_domains {
    MOD_THERMAL_MGMT_DOM_0,
    MOD_THERMAL_MGMT_DOM_1,
    MOD_THERMAL_MGMT_DOM_COUNT,
};

#define FAKE_ACTOR_PER_DOMAIN 2

enum fake_actors {
    FAKE_ACTOR_0,
    FAKE_ACTOR_1,
    FAKE_ACTOR_2,
    FAKE_ACTOR_3,
    FAKE_ACTOR_COUNT,
};

enum fake_sensors {
    FAKE_SENSOR_0,
    FAKE_SENSOR_1,
    FAKE_SENSOR_COUNT,
};

enum fake_activity_factor {
    FAKE_ACTIVITY_FACTOR_0,
    FAKE_ACTIVITY_FACTOR_COUNT,
};

enum fake_activity_factor_api {
    FAKE_ACTIVITY_FACTOR_API,
    FAKE_ACTIVITY_FACTOR_API_COUNT,
};

/*
 * System Power Req module config
 */
static struct mod_thermal_mgmt_actor_config actor_table_domain0[2] = {
    [0] = {
        .driver_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_FAKE_POWER_MODEL,
            FAKE_ACTOR_0),
        .dvfs_domain_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_DVFS,
            FAKE_ACTOR_0),
        .weight = 100,
        .activity_factor =
            &((struct mod_thermal_mgmt_activity_factor_config){
                .driver_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_FAKE_ACTIVITY_COUNTER,
                    FAKE_ACTIVITY_FACTOR_0),
                .driver_api_id = FWK_ID_API_INIT(
                    FWK_MODULE_IDX_FAKE_ACTIVITY_COUNTER,
                    FAKE_ACTIVITY_FACTOR_API),
            }),
    },
    [1] = {
        .driver_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_FAKE_POWER_MODEL,
            FAKE_ACTOR_1),
        .dvfs_domain_id =
            FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DVFS, FAKE_ACTOR_1),
        .weight = 100,
    },
};

static struct mod_thermal_mgmt_actor_config actor_table_domain1[2] = {
    [0] = {
        .driver_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_FAKE_POWER_MODEL,
            FAKE_ACTOR_2),
        .dvfs_domain_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_DVFS,
            FAKE_ACTOR_2),
        .weight = 100,
    },
    [1] = {
        .driver_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_FAKE_POWER_MODEL,
            FAKE_ACTOR_3),
        .dvfs_domain_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_DVFS,
            FAKE_ACTOR_3),
        .weight = 100,
    },
};

static struct mod_thermal_mgmt_actor_config
    *actor_table_domain[MOD_THERMAL_MGMT_DOM_COUNT] = {
        [MOD_THERMAL_MGMT_DOM_0] = actor_table_domain0,
        [MOD_THERMAL_MGMT_DOM_1] = actor_table_domain1,
    };

static struct mod_thermal_mgmt_protection_config temp_protection = {
    .driver_id = fwk_module_id_fake_thermal_protection,
    .driver_api_id = mod_fake_thermal_protection_api_id,
    .warn_temp_threshold = 70,
    .crit_temp_threshold = 80,
};

static const struct mod_thermal_mgmt_dev_config
    dev_config_default_table[MOD_THERMAL_MGMT_DOM_COUNT] = {
    [MOD_THERMAL_MGMT_DOM_0] = {
        .slow_loop_mult = 2,
        .tdp = 10,
        .pid_controller = {
            .switch_on_temperature = 50,
            .control_temperature = 60,
            .integral_cutoff = 0,
            .integral_max = 100,
            .k_p_undershoot = 1,
            .k_p_overshoot = 1,
            .k_integral = 1,
        },
        .sensor_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR, FAKE_SENSOR_0),
        .driver_api_id = FWK_ID_API_INIT(
            FWK_MODULE_IDX_FAKE_POWER_MODEL,
            0),
        .thermal_actors_table = actor_table_domain0,
        .thermal_actors_count = 2,
        .temp_protection = &temp_protection,
    },
    [MOD_THERMAL_MGMT_DOM_1] = {
        .slow_loop_mult = 2,
        .tdp = 10,
        .pid_controller = {
            .switch_on_temperature = 50,
            .control_temperature = 60,
            .integral_cutoff = 0,
            .integral_max = 100,
            .k_p_undershoot = 1,
            .k_p_overshoot = 1,
            .k_integral = 1,
        },
        .sensor_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SENSOR, FAKE_SENSOR_1),
        .driver_api_id = FWK_ID_API_INIT(
            FWK_MODULE_IDX_FAKE_POWER_MODEL,
            0),
        .thermal_actors_table = actor_table_domain1,
        .thermal_actors_count = 2,
    },
};

struct mod_thermal_mgmt_dev_config dev_config_table[MOD_THERMAL_MGMT_DOM_COUNT];

static const struct fwk_element
    thermal_mgmt_domains_elem_table[MOD_THERMAL_MGMT_DOM_COUNT + 1] = {
    [MOD_THERMAL_MGMT_DOM_0] = {
        .name = "Fake Thermal Domain 0",
        .data = (struct mod_thermal_mgmt_dev_config *)
            &dev_config_table[MOD_THERMAL_MGMT_DOM_0],
    },
    [MOD_THERMAL_MGMT_DOM_1] = {
        .name = "Fake Thermal Domain 1",
        .data = (struct mod_thermal_mgmt_dev_config *)
            &dev_config_table[MOD_THERMAL_MGMT_DOM_1],
    },
    [MOD_THERMAL_MGMT_DOM_COUNT] = { 0 } /* Termination description */
};

static const struct fwk_element *get_thermal_mgmt_element_table(
    fwk_id_t module_id)
{
    return thermal_mgmt_domains_elem_table;
};

const struct fwk_module_config config_thermal_mgmt = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_thermal_mgmt_element_table),
};

static struct mod_thermal_mgmt_protection_api thermal_protection_api = {
    .warning = mod_thermal_mgmt_protection_api_warning,
    .critical = mod_thermal_mgmt_protection_api_critical,
};

static struct mod_sensor_api sensor_api = {
    .get_data = mod_sensor_get_data,
};

static struct mod_thermal_mgmt_dev_ctx
    dev_ctx_table[MOD_THERMAL_MGMT_DOM_COUNT];

static struct mod_thermal_mgmt_actor_ctx
    actor_ctx_table[MOD_THERMAL_MGMT_DOM_COUNT][FAKE_ACTOR_PER_DOMAIN];

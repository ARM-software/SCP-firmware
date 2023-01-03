/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_morello_sensor.h>
#include <mod_sensor.h>
#include <mod_timer.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>

/*
 * MORELLO sensor driver config
 */

static struct mod_sensor_info info_temp_sensor = {
    .type = MOD_SENSOR_TYPE_DEGREES_C,
    .update_interval = 0,
    .update_interval_multiplier = 0,
    .unit_multiplier = 0,
};

static struct mod_sensor_info info_volt_sensor = {
    .type = MOD_SENSOR_TYPE_VOLTS,
    .update_interval = 0,
    .update_interval_multiplier = 0,
    .unit_multiplier = 0,
};

static const struct fwk_element
    morello_sensor_element_table[MOD_MORELLO_VOLT_SENSOR_COUNT + 1] = {
    [MOD_MORELLO_TEMP_SENSOR_IDX_CLUSTER0] = {
        .name = "CLUS0 Temperature",
        .data = &((struct mod_morello_temp_sensor_config) {
                .alarm_threshold = 85,
                .alarm_hyst_threshold = 83,
                .shutdown_threshold = 95,
                .shutdown_hyst_threshold = 93,
                .info = &info_temp_sensor,
            }
        ),
    },
    [MOD_MORELLO_TEMP_SENSOR_IDX_CLUSTER1] = {
        .name = "CLUS1 Temperature",
        .data = &((struct mod_morello_temp_sensor_config) {
                .alarm_threshold = 85,
                .alarm_hyst_threshold = 83,
                .shutdown_threshold = 95,
                .shutdown_hyst_threshold = 93,
                .info = &info_temp_sensor,
            }
        ),
    },
    [MOD_MORELLO_TEMP_SENSOR_IDX_SYSTEM] = {
        .name = "SYS Temperature",
        .data = &((struct mod_morello_temp_sensor_config) {
                .alarm_threshold = 85,
                .alarm_hyst_threshold = 83,
                .shutdown_threshold = 95,
                .shutdown_hyst_threshold = 93,
                .info = &info_temp_sensor,
            }
        ),
    },
    [MOD_MORELLO_VOLT_SENSOR_IDX_CLUS0CORE0] = {
        .name = "CLUS0CORE0 Voltage",
        .data = &((struct mod_morello_volt_sensor_config) {
                .info = &info_volt_sensor,
            }
        ),
    },
    [MOD_MORELLO_VOLT_SENSOR_IDX_CLUS0CORE1] = {
        .name = "CLUS0CORE1 Voltage",
        .data = &((struct mod_morello_volt_sensor_config) {
                .info = &info_volt_sensor,
            }
        ),
    },
    [MOD_MORELLO_VOLT_SENSOR_IDX_CLUS1CORE0] = {
        .name = "CLUS1CORE0 Voltage",
        .data = &((struct mod_morello_volt_sensor_config) {
                .info = &info_volt_sensor,
            }
        ),
    },
    [MOD_MORELLO_VOLT_SENSOR_IDX_CLUS1CORE1] = {
        .name = "CLUS1CORE1 Voltage",
        .data = &((struct mod_morello_volt_sensor_config) {
                .info = &info_volt_sensor,
            }
        ),
    },
    [MOD_MORELLO_VOLT_SENSOR_COUNT] = { 0 },
};

static const struct fwk_element *get_morello_sensor_element_table(fwk_id_t id)
{
    return morello_sensor_element_table;
}

struct fwk_module_config config_morello_sensor = {
    .data =
        &(struct mod_morello_sensor_config){
            .t_sensor_count = 3,
            .v_sensor_count = 4,
        },

    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_morello_sensor_element_table),
};

/*
 * Sensor module config
 */
static const struct fwk_element
    sensor_element_table[MOD_MORELLO_VOLT_SENSOR_COUNT + 1] = {
    [MOD_MORELLO_TEMP_SENSOR_IDX_CLUSTER0] = {
        .name = "CLUS0 Temperature",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MORELLO_SENSOR,
                             MOD_MORELLO_TEMP_SENSOR_IDX_CLUSTER0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MORELLO_SENSOR, 0),
        }),
    },
    [MOD_MORELLO_TEMP_SENSOR_IDX_CLUSTER1] = {
        .name = "CLUS1 Temperature",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MORELLO_SENSOR,
                             MOD_MORELLO_TEMP_SENSOR_IDX_CLUSTER1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MORELLO_SENSOR, 0),
        }),
    },
    [MOD_MORELLO_TEMP_SENSOR_IDX_SYSTEM] = {
        .name = "SYS Temperature",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MORELLO_SENSOR,
                             MOD_MORELLO_TEMP_SENSOR_IDX_SYSTEM),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MORELLO_SENSOR, 0),
        }),
    },
    [MOD_MORELLO_VOLT_SENSOR_IDX_CLUS0CORE0] = {
        .name = "CLUS0CORE0 Voltage",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MORELLO_SENSOR,
                             MOD_MORELLO_VOLT_SENSOR_IDX_CLUS0CORE0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MORELLO_SENSOR, 0),
        }),
    },
    [MOD_MORELLO_VOLT_SENSOR_IDX_CLUS0CORE1] = {
        .name = "CLUS0CORE1 Voltage",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MORELLO_SENSOR,
                             MOD_MORELLO_VOLT_SENSOR_IDX_CLUS0CORE1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MORELLO_SENSOR, 0),
        }),
    },
    [MOD_MORELLO_VOLT_SENSOR_IDX_CLUS1CORE0] = {
        .name = "CLUS1CORE0 Voltage",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MORELLO_SENSOR,
                             MOD_MORELLO_VOLT_SENSOR_IDX_CLUS1CORE0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MORELLO_SENSOR, 0),
        }),
    },
    [MOD_MORELLO_VOLT_SENSOR_IDX_CLUS1CORE1] = {
        .name = "CLUS1CORE1 Voltage",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MORELLO_SENSOR,
                             MOD_MORELLO_VOLT_SENSOR_IDX_CLUS1CORE1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MORELLO_SENSOR, 0),
        }),
    },
    [MOD_MORELLO_VOLT_SENSOR_COUNT] = { 0 },
};

static const struct fwk_element *get_sensor_element_table(fwk_id_t module_id)
{
    return sensor_element_table;
}

struct fwk_module_config config_sensor = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_sensor_element_table),
};

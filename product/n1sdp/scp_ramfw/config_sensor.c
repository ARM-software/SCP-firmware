/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_n1sdp_sensor.h>
#include <mod_sensor.h>
#include <mod_timer.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>

/*
 * N1SDP sensor driver config
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

static const struct fwk_element n1sdp_sensor_element_table[] = {
    [MOD_N1SDP_TEMP_SENSOR_IDX_CLUSTER0] = {
        .name = "CLUS0 Temperature",
        .data = &((struct mod_n1sdp_temp_sensor_config) {
            .alarm_threshold = 70,
            .shutdown_threshold = 80,
            .info = &info_temp_sensor,
        }),
    },
    [MOD_N1SDP_TEMP_SENSOR_IDX_CLUSTER1] = {
        .name = "CLUS1 Temperature",
        .data = &((struct mod_n1sdp_temp_sensor_config) {
            .alarm_threshold = 70,
            .shutdown_threshold = 80,
            .info = &info_temp_sensor,
        }),
    },
    [MOD_N1SDP_TEMP_SENSOR_IDX_SYSTEM] = {
        .name = "SYS Temperature",
        .data = &((struct mod_n1sdp_temp_sensor_config) {
            .alarm_threshold = 70,
            .shutdown_threshold = 80,
            .info = &info_temp_sensor,
        }),
    },
    [MOD_N1SDP_VOLT_SENSOR_IDX_CLUS0CORE0] = {
        .name = "CLUS0CORE0 Voltage",
        .data = &((struct mod_n1sdp_volt_sensor_config) {
            .info = &info_volt_sensor,
        }),
    },
    [MOD_N1SDP_VOLT_SENSOR_IDX_CLUS0CORE1] = {
        .name = "CLUS0CORE1 Voltage",
        .data = &((struct mod_n1sdp_volt_sensor_config) {
            .info = &info_volt_sensor,
        }),
    },
    [MOD_N1SDP_VOLT_SENSOR_IDX_CLUS1CORE0] = {
        .name = "CLUS1CORE0 Voltage",
        .data = &((struct mod_n1sdp_volt_sensor_config) {
            .info = &info_volt_sensor,
        }),
    },
    [MOD_N1SDP_VOLT_SENSOR_IDX_CLUS1CORE1] = {
        .name = "CLUS1CORE1 Voltage",
        .data = &((struct mod_n1sdp_volt_sensor_config) {
            .info = &info_volt_sensor,
        }),
    },
    [MOD_N1SDP_VOLT_SENSOR_IDX_SYSTEM] = {
        .name = "SYS Voltage",
        .data = &((struct mod_n1sdp_volt_sensor_config) {
            .info = &info_volt_sensor,
        }),
    },
    [MOD_N1SDP_VOLT_SENSOR_COUNT] = { 0 },
};

static const struct fwk_element *get_n1sdp_sensor_element_table(fwk_id_t id)
{
    return n1sdp_sensor_element_table;
}

struct fwk_module_config config_n1sdp_sensor = {
    .get_element_table = get_n1sdp_sensor_element_table,
    .data = &((struct mod_n1sdp_sensor_config) {
        .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0, 0),
        .alarm_api = FWK_ID_API_INIT(FWK_MODULE_IDX_TIMER,
                                     MOD_TIMER_API_IDX_ALARM),
        .t_sensor_count = 3,
        .v_sensor_count = 5,
    }),
};

/*
 * Sensor module config
 */
static const struct fwk_element sensor_element_table[] = {
    [MOD_N1SDP_TEMP_SENSOR_IDX_CLUSTER0] = {
        .name = "CLUS0 Temperature",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_SENSOR,
                             MOD_N1SDP_TEMP_SENSOR_IDX_CLUSTER0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_SENSOR, 0),
        }),
    },
    [MOD_N1SDP_TEMP_SENSOR_IDX_CLUSTER1] = {
        .name = "CLUS1 Temperature",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_SENSOR,
                             MOD_N1SDP_TEMP_SENSOR_IDX_CLUSTER1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_SENSOR, 0),
        }),
    },
    [MOD_N1SDP_TEMP_SENSOR_IDX_SYSTEM] = {
        .name = "SYS Temperature",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_SENSOR,
                             MOD_N1SDP_TEMP_SENSOR_IDX_SYSTEM),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_SENSOR, 0),
        }),
    },
    [MOD_N1SDP_VOLT_SENSOR_IDX_CLUS0CORE0] = {
        .name = "CLUS0CORE0 Voltage",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_SENSOR,
                             MOD_N1SDP_VOLT_SENSOR_IDX_CLUS0CORE0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_SENSOR, 0),
        }),
    },
    [MOD_N1SDP_VOLT_SENSOR_IDX_CLUS0CORE1] = {
        .name = "CLUS0CORE1 Voltage",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_SENSOR,
                             MOD_N1SDP_VOLT_SENSOR_IDX_CLUS0CORE1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_SENSOR, 0),
        }),
    },
    [MOD_N1SDP_VOLT_SENSOR_IDX_CLUS1CORE0] = {
        .name = "CLUS1CORE0 Voltage",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_SENSOR,
                             MOD_N1SDP_VOLT_SENSOR_IDX_CLUS1CORE0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_SENSOR, 0),
        }),
    },
    [MOD_N1SDP_VOLT_SENSOR_IDX_CLUS1CORE1] = {
        .name = "CLUS1CORE1 Voltage",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_SENSOR,
                             MOD_N1SDP_VOLT_SENSOR_IDX_CLUS1CORE1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_SENSOR, 0),
        }),
    },
    [MOD_N1SDP_VOLT_SENSOR_IDX_SYSTEM] = {
        .name = "SYS Voltage",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_N1SDP_SENSOR,
                             MOD_N1SDP_VOLT_SENSOR_IDX_SYSTEM),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_SENSOR, 0),
        }),
    },
    [MOD_N1SDP_VOLT_SENSOR_COUNT] = { 0 },
};

static const struct fwk_element *get_sensor_element_table(fwk_id_t module_id)
{
    return sensor_element_table;
}

struct fwk_module_config config_sensor = {
    .get_element_table = get_sensor_element_table,
    .data = NULL,
};

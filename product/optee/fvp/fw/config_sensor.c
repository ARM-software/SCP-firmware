/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_reg_sensor.h"

#include <mod_reg_sensor.h>
#include <mod_scmi_sensor.h>
#include <mod_sensor.h>

#include <fwk_module.h>
#include <fwk_module_idx.h>

uint64_t soc_temp[3] = { 25000, 25000, 25000 };
uint64_t ddr_temp[1] = { 25000 };
/*
 * Register Sensor driver config
 */
static struct mod_sensor_info info_soc_temperature = {
    .type = MOD_SENSOR_TYPE_DEGREES_C,
    .update_interval = 0,
    .update_interval_multiplier = 0,
    .unit_multiplier = -3,
    .disabled = false,
};

static const struct fwk_element reg_sensor_element_table[] = {
    [SENSOR_DEV_SOC_TEMP] = {
        .name = "Soc Temperature",
        .data = &((struct mod_reg_sensor_dev_config) {
            .reg = (uintptr_t)(soc_temp),
            .info = &info_soc_temperature,
        }),
    },
    [SENSOR_DEV_DDR_TEMP] = {
        .name = "DDR Temperature",
        .data = &((struct mod_reg_sensor_dev_config) {
            .reg = (uintptr_t)(ddr_temp),
            .info = &info_soc_temperature,
        }),
    },
    [SENSOR_DEV_COUNT] = { 0 },
};

static const struct fwk_element *get_reg_sensor_element_table(fwk_id_t id)
{
    return reg_sensor_element_table;
}

struct fwk_module_config config_reg_sensor = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_reg_sensor_element_table),
};

/*
 * Sensor module config
 */
static const struct fwk_element sensor_element_table[] = {
    [0] = {
        .name = "Soc Temperature",
        .data = &((const struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_REG_SENSOR,
                                             SENSOR_DEV_SOC_TEMP),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_REG_SENSOR, 0),
        .trip_point = {
            .count = 2,
        }
        }),
    },
    [1] = {
        .name = "DDR Temperature",
        .data = &((const struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_REG_SENSOR,
                                             SENSOR_DEV_DDR_TEMP),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_REG_SENSOR, 0),
        .trip_point = {
            .count = 2,
        }
        }),
    },
    [2] = { 0 },
};

static const struct fwk_element *get_sensor_element_table(fwk_id_t module_id)
{
    return sensor_element_table;
}
static const struct mod_sensor_config sensor_config = {
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    .notification_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_SENSOR),
    .trip_point_api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_SCMI_SENSOR,
        SCMI_SENSOR_API_IDX_TRIP_POINT),
#else
    .notification_id = FWK_ID_NONE_INIT
#endif
};

struct fwk_module_config config_sensor = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_sensor_element_table),
    .data = &sensor_config,
};

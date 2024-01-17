/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2024, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <system_mmap.h>
#include <config_sensor.h>

#include <mod_reg_sensor.h>
#include <mod_sensor.h>

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>

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
    [REG_SENSOR_DEV_SOC_TEMP1] = {
        .name = "thermal1",
        .data = &((struct mod_reg_sensor_dev_config) {
            .reg = (uintptr_t)(SENSOR_SOC_TEMP1),
            .info = &info_soc_temperature,
        }),
    },
    [REG_SENSOR_DEV_SOC_TEMP2] = {
        .name = "thermal2",
        .data = &((struct mod_reg_sensor_dev_config) {
            .reg = (uintptr_t)(SENSOR_SOC_TEMP2),
            .info = &info_soc_temperature,
        }),
    },
    [REG_SENSOR_DEV_SOC_TEMP3] = {
        .name = "thermal3",
        .data = &((struct mod_reg_sensor_dev_config) {
            .reg = (uintptr_t)(SENSOR_SOC_TEMP3),
            .info = &info_soc_temperature,
        }),
    },
    [REG_SENSOR_DEV_COUNT] = { 0 },
};

static const struct fwk_element *get_reg_sensor_element_table(fwk_id_t id)
{
    return reg_sensor_element_table;
}

struct fwk_module_config config_rcar_reg_sensor = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_reg_sensor_element_table),
};

/*
 * Sensor module config
 */
static const struct fwk_element sensor_element_table[] = {
    [R8A7795_SNSR_THERMAL1] = {
        .name = "thermal1",
        .data = &((const struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_REG_SENSOR,
                                             REG_SENSOR_DEV_SOC_TEMP1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_REG_SENSOR, 0),
        }),
    },
    [R8A7795_SNSR_THERMAL2] = {
        .name = "thermal2",
        .data = &((const struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_REG_SENSOR,
                                             REG_SENSOR_DEV_SOC_TEMP2),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_REG_SENSOR, 0),
        }),
    },
    [R8A7795_SNSR_THERMAL3] = {
        .name = "thermal3",
        .data = &((const struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_REG_SENSOR,
                                             REG_SENSOR_DEV_SOC_TEMP3),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_REG_SENSOR, 0),
        }),
    },
    [R8A7795_SNSR_COUNT] = { 0 },
};

static const struct fwk_element *get_sensor_element_table(fwk_id_t module_id)
{
    return sensor_element_table;
}

struct fwk_module_config config_sensor = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_sensor_element_table),
    .data = NULL,
};

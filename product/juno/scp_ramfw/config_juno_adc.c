/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_sensor.h>

#include <mod_juno_adc.h>
#include <mod_sensor.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

static const struct fwk_element adc_juno_element_table[] = {
    [ADC_TYPE_VOLT] = {
        .name = "",
        .sub_element_count = ADC_DEV_TYPE_COUNT,
        .data = &((struct mod_juno_adc_dev_config) {
            .info = &((struct mod_sensor_info) {
                .type = MOD_SENSOR_TYPE_VOLTS,
                .unit_multiplier = -3,
            }),
        }),
    },

    #if USE_FULL_SET_SENSORS
    [ADC_TYPE_CURRENT] = {
        .name = "",
        .sub_element_count = ADC_DEV_TYPE_COUNT,
        .data = &((struct mod_juno_adc_dev_config) {
            .info = &((struct mod_sensor_info) {
                .type = MOD_SENSOR_TYPE_AMPS,
                .unit_multiplier = -3,
            }),
        }),
    },
    [ADC_TYPE_POWER] = {
        .name = "",
        .sub_element_count = ADC_DEV_TYPE_COUNT,
        .data = &((struct mod_juno_adc_dev_config) {
            .info = &((struct mod_sensor_info) {
                .type = MOD_SENSOR_TYPE_WATTS,
                .unit_multiplier = -6,
            }),
        }),
    },
    [ADC_TYPE_ENERGY] = {
        .name = "",
        .sub_element_count = ADC_DEV_TYPE_COUNT,
        .data = &((struct mod_juno_adc_dev_config) {
            .info = &((struct mod_sensor_info) {
                .type = MOD_SENSOR_TYPE_JOULES,
                .unit_multiplier = -6,
            }),
        }),
    },
    #endif

    [ADC_TYPE_COUNT] = { 0 },
};

static const struct fwk_element *get_adc_juno_element_table(fwk_id_t id)
{
    return adc_juno_element_table;
}

struct fwk_module_config config_juno_adc = {
    .get_element_table = get_adc_juno_element_table,
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_reg_sensor.h>
#include <mod_sensor.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <stddef.h>
#include <stdint.h>

static uint64_t fake_register = UINT64_C(0x1234);

/*
 * Register Sensor driver config
 */
static struct mod_sensor_info info_fake_temperature = {
    .type = MOD_SENSOR_TYPE_DEGREES_C,
    .update_interval = 0,
    .update_interval_multiplier = 0,
    .unit_multiplier = 0,
};

static const struct fwk_element reg_sensor_element_table[] = {
    [0] = {
        .name = "",
        .data = &((struct mod_reg_sensor_dev_config) {
            .reg = (uintptr_t)(&fake_register),
            .info = &info_fake_temperature,
        }),
    },

    [1] = { 0 },
};

static const struct fwk_element *get_reg_sensor_element_table(fwk_id_t id)
{
    return reg_sensor_element_table;
}

struct fwk_module_config config_reg_sensor = {
    .get_element_table = get_reg_sensor_element_table,
    .data = NULL,
};

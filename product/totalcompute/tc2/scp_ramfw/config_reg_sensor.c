/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
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

static uint64_t fake_sensor_register = UINT64_C(0x0);

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
        .name = "TC0 Fake Temperature Sensor Register",
        .data = &((struct mod_reg_sensor_dev_config) {
            .reg = (uintptr_t)(&fake_sensor_register),
            .info = &info_fake_temperature,
        }),
    },

    [1] = { 0 }, /* Termination description */
};

static const struct fwk_element *get_reg_sensor_element_table(fwk_id_t id)
{
    return reg_sensor_element_table;
}

struct fwk_module_config config_reg_sensor = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_reg_sensor_element_table),
};

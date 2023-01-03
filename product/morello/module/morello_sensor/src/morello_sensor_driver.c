/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_morello_sensor.h>

#include <fwk_attributes.h>
#include <fwk_module.h>

#define DEFAULT_TEMP_VALUE 25
#define DEFAULT_VOLT_VALUE 1000

/*
 * Driver API
 */

int morello_sensor_lib_get_sensor_value(
    int32_t *value,
    enum sensor_type type,
    int offset)
{
    int status = FWK_SUCCESS;

    if (value == NULL)
        return FWK_E_DATA;

    switch (type) {
    case MOD_MORELLO_TEMP_SENSOR:
        *value = DEFAULT_TEMP_VALUE;
        break;

    case MOD_MORELLO_VOLT_SENSOR:
        *value = DEFAULT_VOLT_VALUE;
        break;

    default:
        status = FWK_E_DATA;
        break;
    }

    return status;
}

int morello_sensor_lib_init(uint32_t *msg)
{
    return FWK_SUCCESS;
}

void morello_enable_temp_sensor_alarm(
    int offset,
    float alarm_A_threshold,
    float alarm_B_threshold,
    float alarm_A_hysteresis,
    float alarm_B_hysteresis)
{
}

int morello_sensor_lib_handle_irq(int *offset)
{
    return MOD_MORELLO_SENSOR_INVALID_INTERRUPT;
}

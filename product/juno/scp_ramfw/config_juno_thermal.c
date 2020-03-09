/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_juno_thermal.h"
#include "config_sensor.h"
#include "juno_alarm_idx.h"

#include <mod_juno_thermal.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static struct fwk_element juno_thermal_element_table[] = {
    [MOD_JUNO_THERMAL_ELEMENT_IDX_CRITICAL] = {
        .name = "Soc",
        .data = &(struct mod_juno_thermal_element_config) {
            .thermal_threshold_mdc = UINT64_C(70 * 1000),
            .period_ms = (2 * 1000),
            .sensor_id =
                FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_SENSOR,
                    MOD_JUNO_PVT_SENSOR_TEMP_SOC),
            .alarm_id =
                FWK_ID_SUB_ELEMENT_INIT(
                    FWK_MODULE_IDX_TIMER,
                    0,
                    JUNO_THERMAL_ALARM_IDX),
        }
    },
    [MOD_JUNO_THERMAL_ELEMENT_IDX_COUNT] = { 0 },
};

static const struct fwk_element *juno_thermal_get_element_table(
    fwk_id_t module_id)
{
    return juno_thermal_element_table;
}

const struct fwk_module_config config_juno_thermal = {
    .get_element_table = juno_thermal_get_element_table,
};

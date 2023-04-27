/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_sensor.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <string.h>

/*
 * When running on a model at least one fake sensor is required in order to
 * properly initialize the entire sensor support.
 */
static const struct fwk_element sensor_element_table[2] = {
    [0] = {
        .name = "Fake sensor",
        .data = &((struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_REG_SENSOR, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_REG_SENSOR, 0),
        }),
    },

    [1] = { 0 } /* Termination description */
};

static const struct fwk_element *get_sensor_element_table(fwk_id_t module_id)
{
    return sensor_element_table;
}

struct fwk_module_config config_sensor = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_sensor_element_table),
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Sensor unit test configuration.
 */

#include <sensor.h>

#include <mod_sensor.h>

#define SENSOR_ELEMENT_COUNT 2

#define SENSOR_FAKE_EXPONENT_VALUE 0x5A

enum {
    SENSOR_FAKE_INDEX_0,
    SENSOR_FAKE_INDEX_1,
    SENSOR_FAKE_INDEX_2,
    SENSOR_FAKE_INDEX_INVALID
};
enum { SENSOR_TRIP_POINT_0, SENSOR_TRIP_POINT_1, SENSOR_TRIP_POINT_COUNT };

enum { SENSOR_ROUND_0, SENSOR_ROUND_1 };
enum { SENSOR_FAKE_MODULE_0 };

static struct sensor_dev_ctx sensor_dev_context[SENSOR_ELEMENT_COUNT];

static const struct fwk_element sensor_element_table[SENSOR_ELEMENT_COUNT] = {
    [SENSOR_FAKE_INDEX_0] = {
        .name = "Fake Sensor 0",
        .data = &((
            struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_REG_SENSOR, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_REG_SENSOR, 0),
            .trip_point = {
                .count = SENSOR_TRIP_POINT_0,
            },
        }),
    },
    [SENSOR_FAKE_INDEX_1] = {
        .name = "Fake Sensor 1",
        .data = &((
            struct mod_sensor_dev_config) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_REG_SENSOR, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_REG_SENSOR, 0),
            .trip_point = {
                .count = SENSOR_TRIP_POINT_1,
            },
        }),
    },
};

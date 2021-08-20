/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SENSOR_H
#define SENSOR_H

#include <mod_sensor.h>

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \cond
 */

/*
 * Sensor concurrency readings max pending requests.
 */
#define SENSOR_MAX_PENDING_REQUESTS 3

/*
 * Sensor trip point element context
 */
struct sensor_trip_point_ctx {
    /* Structure that describes the trip point configuration */
    struct mod_sensor_trip_point_params params;

    /* This flag is used to latch status if the trip point were triggered */
    bool above_threshold;

    /* This flag describes if this feature is enabled or not */
    bool enabled;
};

/*
 * Sensor element context
 */
struct sensor_dev_ctx {
    struct mod_sensor_dev_config *config;

    struct mod_sensor_driver_api *driver_api;

    struct sensor_trip_point_ctx *trip_point_ctx;
    uint32_t cookie;

    struct {
        uint32_t pending_requests;
        bool dequeuing;
    } concurrency_readings;

    struct mod_sensor_data last_read;

    unsigned int axis_count;

#ifdef BUILD_HAS_SENSOR_TIMESTAMP
    struct mod_sensor_timestamp_info timestamp;
#endif

#ifdef BUILD_HAS_SENSOR_EXT_ATTRIBS
    bool mod_extended_attrib;

    struct mod_sensor_ext_properties sensor_property_values;
#endif
};

struct mod_sensor_ctx {
    struct mod_sensor_config *config;
    struct mod_sensor_trip_point_api *sensor_trip_point_api;
};

struct sensor_dev_ctx *sensor_get_ctx(fwk_id_t id);

/*
 * Sensor event indexes
 */
enum mod_sensor_event_idx {
    SENSOR_EVENT_IDX_READ_REQUEST = MOD_SENSOR_EVENT_IDX_READ_REQUEST,
    SENSOR_EVENT_IDX_READ_COMPLETE,
    SENSOR_EVENT_IDX_COUNT
};

/*
 * Event identifiers
 */
static const fwk_id_t mod_sensor_event_id_read_complete =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_SENSOR,
                      SENSOR_EVENT_IDX_READ_COMPLETE);

#ifdef BUILD_HAS_SENSOR_TIMESTAMP

int sensor_timestamp_dev_init(fwk_id_t id, struct sensor_dev_ctx *ctx);

int sensor_set_timestamp_config(
    fwk_id_t id,
    const struct mod_sensor_timestamp_info *config);

int sensor_get_timestamp_config(
    fwk_id_t id,
    struct mod_sensor_timestamp_info *config);

uint64_t sensor_get_timestamp(fwk_id_t id);
#endif

#ifdef BUILD_HAS_SENSOR_MULTI_AXIS

int sensor_axis_start(fwk_id_t id);

int sensor_get_axis_info(
    fwk_id_t id,
    uint32_t axis,
    struct mod_sensor_axis_info *info);
#endif

/*!
 * \endcond
 */

#endif /* SENSOR_H */

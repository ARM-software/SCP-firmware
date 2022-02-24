/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sensor.h"

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_status.h>
#include <fwk_time.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef BUILD_HAS_SENSOR_TIMESTAMP

int sensor_timestamp_dev_init(fwk_id_t id, struct sensor_dev_ctx *ctx)
{
    ctx->timestamp.timestamp_support = ctx->config->timestamp.timestamp_support;
    if (ctx->timestamp.timestamp_support) {
        return sensor_set_timestamp_config(id, &ctx->config->timestamp);
    }
    return FWK_SUCCESS;
}

int sensor_set_timestamp_config(
    fwk_id_t id,
    const struct mod_sensor_timestamp_info *config)
{
    struct sensor_dev_ctx *ctx;

    fwk_assert(!fwk_id_is_equal(id, FWK_ID_NONE));
    if (config == NULL) {
        return FWK_E_PARAM;
    }

    ctx = sensor_get_ctx(id);

    if (!ctx->timestamp.timestamp_support) {
        return FWK_E_SUPPORT;
    }

    ctx->timestamp.enabled = config->enabled;
    ctx->timestamp.exponent = config->exponent;

    return FWK_SUCCESS;
}

int sensor_get_timestamp_config(
    fwk_id_t id,
    struct mod_sensor_timestamp_info *config)
{
    struct sensor_dev_ctx *ctx;

    fwk_assert(!fwk_id_is_equal(id, FWK_ID_NONE));
    if (config == NULL) {
        return FWK_E_PARAM;
    }

    ctx = sensor_get_ctx(id);

    if (!ctx->timestamp.timestamp_support) {
        return FWK_E_SUPPORT;
    }

    config->timestamp_support = ctx->timestamp.timestamp_support;
    config->enabled = ctx->timestamp.enabled;
    config->exponent = ctx->timestamp.exponent;

    return FWK_SUCCESS;
}

/* Power function implemented for positive integers */
static uint64_t pow_unsigned(uint64_t base, uint64_t exponent)
{
    uint64_t result = 1;
    for (; exponent > 0; exponent--) {
        result *= base;
    }
    return result;
}

uint64_t sensor_get_timestamp(fwk_id_t id)
{
    struct sensor_dev_ctx *ctx;
    uint64_t timestamp;
    int64_t normalized_exponent;

    ctx = sensor_get_ctx(id);

    if (!ctx->timestamp.enabled) {
        return 0;
    }

    timestamp = FWK_NS(fwk_time_current());
    /*
     * Exponent is the power-of-10 multiplier that is applied to the
     * sensor timestamps (timestamp x 10 [timestamp exponent] ) to
     * represent it in seconds. Since `fwk_time_current()` base time is in
     * nano seconds, base unit should be changed.
     */
    normalized_exponent = ctx->timestamp.exponent + 9;
    if (normalized_exponent > 0) {
        return timestamp / pow_unsigned(10, normalized_exponent);
    }
    return timestamp / pow_unsigned(10, -normalized_exponent);
}

#endif

#ifdef BUILD_HAS_SENSOR_MULTI_AXIS

int sensor_axis_start(fwk_id_t id)
{
    struct sensor_dev_ctx *ctx;
    struct mod_sensor_axis_info axis_info;
    enum mod_sensor_type type;
    unsigned int i;
    int status;

    ctx = sensor_get_ctx(id);

    if (ctx->driver_api->get_axis_count) {
        ctx->axis_count =
            ctx->driver_api->get_axis_count(ctx->config->driver_id);
    } else {
        ctx->axis_count = 1;
    }
    ctx->last_read.axis_value =
        fwk_mm_calloc(ctx->axis_count, sizeof(uint64_t));
    ctx->last_read.axis_count = ctx->axis_count;

    if (ctx->config->trip_point.count > 0) {
        for (i = 0; i < ctx->axis_count; i++) {
            status = sensor_get_axis_info(id, i, &axis_info);
            if (status != FWK_SUCCESS) {
                return status;
            }

            /* Save value of first axis type */
            if (i == 0) {
                type = axis_info.type;
            } else if (axis_info.type != type) {
                ctx->trip_point_ctx->enabled = false;

                /*
                 * Valid situation where axis have different types, for that
                 * reason trip points are disabled for this particular sensor.
                 */
                FWK_LOG_INFO(
                    "[Sensor] Trip points are disable. Different axis type");

                return FWK_SUCCESS;
            }
        }

        ctx->trip_point_ctx->enabled = true;
    }

    return FWK_SUCCESS;
}

int sensor_get_axis_info(
    fwk_id_t id,
    uint32_t axis,
    struct mod_sensor_axis_info *info)
{
    struct sensor_dev_ctx *ctx;

    if (info == NULL) {
        return FWK_E_PARAM;
    }

    ctx = sensor_get_ctx(id);

    if (axis > ctx->axis_count) {
        return FWK_E_PARAM;
    }

    ctx->driver_api->get_axis_info(ctx->config->driver_id, axis, info);
    return FWK_SUCCESS;
}

#endif

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sensor.h"

#include <mod_sensor.h>

#include <fwk_assert.h>
#include <fwk_id.h>
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
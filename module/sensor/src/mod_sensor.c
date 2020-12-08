/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sensor.h>

#include <mod_scmi_sensor.h>
#include <mod_sensor.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static struct sensor_dev_ctx *ctx_table;
static struct sensor_mod_ctx sensor_mod_ctx;

static int get_ctx_if_valid_call(fwk_id_t id,
                                 void *data,
                                 struct sensor_dev_ctx **ctx)
{
    fwk_assert(ctx != NULL);

    if (!fwk_expect(data != NULL))
        return FWK_E_PARAM;

    *ctx = ctx_table + fwk_id_get_element_idx(id);

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_SCMI_SENSOR_EVENTS
static bool trip_point_evaluate(
    struct sensor_trip_point_ctx *ctx,
    uint64_t value)
{
    uint64_t threshold;
    bool new_above_threshold, trigger = false;

    threshold = (((uint64_t)ctx->params.high_value << 32) & ~0U) |
        (((uint64_t)ctx->params.low_value) & ~0U);
    new_above_threshold = value > threshold;

    switch (ctx->params.mode) {
    case MOD_SENSOR_TRIP_POINT_MODE_POSITIVE:
        if (!ctx->above_threshold && value > threshold)
            trigger = true;
        break;

    case MOD_SENSOR_TRIP_POINT_MODE_NEGATIVE:
        if (ctx->above_threshold && (value <= threshold))
            trigger = true;
        break;

    case MOD_SENSOR_TRIP_POINT_MODE_TRANSITION:
        if ((!ctx->above_threshold && value > threshold) ||
            (ctx->above_threshold && value <= threshold))
            trigger = true;
        break;

    default:
        break;
    }
    ctx->above_threshold = new_above_threshold;
    return trigger;
}
#endif

#ifdef BUILD_HAS_SCMI_SENSOR_EVENTS
static void trip_point_process(fwk_id_t id, uint64_t value)
{
    struct sensor_dev_ctx *ctx;
    unsigned int i;

    fwk_check(!fwk_id_is_equal(id, FWK_ID_NONE));
    ctx = ctx_table + fwk_id_get_element_idx(id);

    for (i = 0; i < ctx->config->trip_point.count; i++) {
        if (trip_point_evaluate(&(ctx->trip_point_ctx[i]), value)) {
            /* Handle trip point event*/
            if (sensor_mod_ctx.sensor_trip_point_api != NULL)
                sensor_mod_ctx.sensor_trip_point_api->notify_sensor_trip_point(
                    id, ctx->trip_point_ctx->above_threshold, i);
        }
    }
}
#endif

/*
 * Module API
 */
static int get_value(fwk_id_t id, uint64_t *value)
{
    int status;
    struct sensor_dev_ctx *ctx;
    struct fwk_event req;

    status = get_ctx_if_valid_call(id, value, &ctx);
    if (status != FWK_SUCCESS)
        return status;

    /* Concurrent readings are not supported */
    if (ctx->read_busy)
        return FWK_E_BUSY;

    status = ctx->driver_api->get_value(ctx->config->driver_id, value);
    if (status == FWK_PENDING) {
        req = (struct fwk_event) {
            .target_id = id,
            .id = mod_sensor_event_id_read_request,
            .response_requested = true,
        };

        status = fwk_thread_put_event(&req);
        if (status == FWK_SUCCESS) {
            ctx->read_busy = true;

             /*
              * We return FWK_PENDING here to indicate to the caller that the
              * result of the request is pending and will arrive later through
              * an event.
              */
            return FWK_PENDING;
        } else
            return status;
    } else if (status == FWK_SUCCESS) {
#ifdef BUILD_HAS_SCMI_SENSOR_EVENTS
        trip_point_process(id, *value);
#endif
        return FWK_SUCCESS;
    }

    else
        return FWK_E_DEVICE;
}

static int get_info(fwk_id_t id, struct mod_sensor_scmi_info *info)
{
    int status;
    struct sensor_dev_ctx *ctx;

    status = get_ctx_if_valid_call(id, info, &ctx);
    if (status != FWK_SUCCESS)
        return status;

    status = ctx->driver_api->get_info(ctx->config->driver_id, &info->hal_info);
    if (!fwk_expect(status == FWK_SUCCESS))
        return FWK_E_DEVICE;
    info->trip_point = ctx->config->trip_point;

    return FWK_SUCCESS;
}

static int sensor_get_trip_point(
    fwk_id_t id,
    uint32_t trip_point_idx,
    struct mod_sensor_trip_point_params *params)
{
    struct sensor_dev_ctx *ctx;

    fwk_check(params != NULL);

    ctx = ctx_table + fwk_id_get_element_idx(id);

    if (trip_point_idx >= ctx->config->trip_point.count)
        return FWK_E_PARAM;

    *params = ctx->trip_point_ctx[trip_point_idx].params;

    return FWK_SUCCESS;
}

static int sensor_set_trip_point(
    fwk_id_t id,
    uint32_t trip_point_idx,
    struct mod_sensor_trip_point_params *params)
{
    struct sensor_dev_ctx *ctx;

    if (params == NULL)
        return FWK_E_PARAM;

    ctx = ctx_table + fwk_id_get_element_idx(id);

    if (trip_point_idx >= ctx->config->trip_point.count)
        return FWK_E_PARAM;

    ctx->trip_point_ctx[trip_point_idx].params = *params;

    /* Clear the trip point flag */
    ctx->trip_point_ctx[trip_point_idx].above_threshold = false;
    return FWK_SUCCESS;
}

static struct mod_sensor_api sensor_api = { .get_value = get_value,
                                            .get_info = get_info,
                                            .get_trip_point =
                                                sensor_get_trip_point,
                                            .set_trip_point =
                                                sensor_set_trip_point };

/*
 * Driver response API.
 */
static void reading_complete(fwk_id_t dev_id,
                             struct mod_sensor_driver_resp_params *response)
{
    int status = FWK_SUCCESS;
    struct fwk_event event;
    struct sensor_dev_ctx *ctx;
    struct mod_sensor_event_params *event_params =
        (struct mod_sensor_event_params *)(void *)event.params;

    if (!fwk_expect(fwk_id_get_module_idx(dev_id) == FWK_MODULE_IDX_SENSOR))
        return;

    ctx = &ctx_table[fwk_id_get_element_idx(dev_id)];

    event = (struct fwk_event) {
        .id = mod_sensor_event_id_read_complete,
        .source_id = ctx->config->driver_id,
        .target_id = dev_id,
    };

    if (response != NULL) {
        event_params->status = response->status;
        event_params->value = response->value;
#ifdef BUILD_HAS_SCMI_SENSOR_EVENTS
        trip_point_process(dev_id, response->value);
#endif
    } else
        event_params->status = FWK_E_DEVICE;

    status = fwk_thread_put_event(&event);
    fwk_assert(status == FWK_SUCCESS);
}

static struct mod_sensor_driver_response_api sensor_driver_response_api = {
    .reading_complete = reading_complete,
};

/*
 * Framework handlers
 */
static int sensor_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    struct mod_sensor_config *config;

    ctx_table = fwk_mm_calloc(element_count, sizeof(ctx_table[0]));
    config = (struct mod_sensor_config *)data;

    sensor_mod_ctx.config = config;
    return FWK_SUCCESS;
}

static int sensor_dev_init(fwk_id_t element_id,
                           unsigned int unused,
                           const void *data)
{
    struct sensor_dev_ctx *ctx;
    struct mod_sensor_dev_config *config;

    ctx = ctx_table + fwk_id_get_element_idx(element_id);

    fwk_check(data != NULL);
    config = (struct mod_sensor_dev_config*)data;

    ctx->config = config;

    if (config->trip_point.count > 0) {
        ctx->trip_point_ctx = fwk_mm_calloc(
            config->trip_point.count, sizeof(struct sensor_trip_point_ctx));
    } else
        ctx->trip_point_ctx = NULL;
    return FWK_SUCCESS;
}

static int sensor_bind(fwk_id_t id, unsigned int round)
{
    struct sensor_dev_ctx *ctx;
    int status;
    struct mod_sensor_driver_api *driver = NULL;

    if (round > 0) {
        /*
         * Only bind in first round of calls
         */
        return FWK_SUCCESS;
    }
    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        if (sensor_mod_ctx.config == NULL)
            return FWK_SUCCESS;

        if (fwk_id_is_equal(
                sensor_mod_ctx.config->notification_id, FWK_ID_NONE))
            return FWK_SUCCESS;

        return fwk_module_bind(
            sensor_mod_ctx.config->notification_id,
            sensor_mod_ctx.config->trip_point_api_id,
            &sensor_mod_ctx.sensor_trip_point_api);
    }
    ctx = ctx_table + fwk_id_get_element_idx(id);
    /* Bind to driver */
    status = fwk_module_bind(ctx->config->driver_id,
        ctx->config->driver_api_id,
        &driver);
    if (status != FWK_SUCCESS)
        return status;

    /* Validate driver API */
    if ((driver == NULL) || (driver->get_value == NULL))
        return FWK_E_DATA;

    ctx->driver_api = driver;

    return FWK_SUCCESS;
}

static int sensor_process_bind_request(fwk_id_t source_id,
                                       fwk_id_t target_id,
                                       fwk_id_t api_id,
                                       const void **api)
{
    struct sensor_dev_ctx *ctx;
    fwk_id_t driver_id;

    if (fwk_id_is_equal(api_id, mod_sensor_api_id_sensor)) {
        *api = &sensor_api;

        return FWK_SUCCESS;
    }

    if (fwk_id_is_equal(api_id, mod_sensor_api_id_driver_response)) {
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT))
            return FWK_E_PARAM;

        ctx = ctx_table + fwk_id_get_element_idx(target_id);
        driver_id = ctx->config->driver_id;

        /* Allow element to sub-element binding */
        if ((fwk_id_get_module_idx(driver_id) ==
            fwk_id_get_module_idx(source_id)) &&
            (fwk_id_get_element_idx(driver_id) ==
            fwk_id_get_element_idx(source_id))) {

            *api = &sensor_driver_response_api;

            return FWK_SUCCESS;
        } else
            return FWK_E_ACCESS;
    }

    return FWK_E_PARAM;
}

static int sensor_process_event(const struct fwk_event *event,
                                struct fwk_event *resp_event)
{
    int status;
    struct sensor_dev_ctx *ctx;
    struct fwk_event read_req_event;
    struct mod_sensor_event_params *event_params =
        (struct mod_sensor_event_params *)(void *)event->params;
    struct mod_sensor_event_params *resp_params =
        (struct mod_sensor_event_params *)(void *)read_req_event.params;

    if (!fwk_module_is_valid_element_id(event->target_id))
        return FWK_E_PARAM;

    ctx = ctx_table + fwk_id_get_element_idx(event->target_id);

    switch (fwk_id_get_event_idx(event->id)) {

    case SENSOR_EVENT_IDX_READ_REQUEST:
        ctx->cookie = event->cookie;
        resp_event->is_delayed_response = true;

        return FWK_SUCCESS;

    case SENSOR_EVENT_IDX_READ_COMPLETE:
        ctx->read_busy = false;

        status = fwk_thread_get_delayed_response(event->target_id,
                                                 ctx->cookie,
                                                 &read_req_event);
        if (status != FWK_SUCCESS)
            return status;

        *resp_params = *event_params;
        return fwk_thread_put_event(&read_req_event);

    default:
        return FWK_E_PARAM;
    }
}

const struct fwk_module module_sensor = {
    .name = "SENSOR",
    .api_count = MOD_SENSOR_API_IDX_COUNT,
    .event_count = SENSOR_EVENT_IDX_COUNT,
    .type = FWK_MODULE_TYPE_HAL,
    .init = sensor_init,
    .element_init = sensor_dev_init,
    .bind = sensor_bind,
    .process_bind_request = sensor_process_bind_request,
    .process_event = sensor_process_event,
};

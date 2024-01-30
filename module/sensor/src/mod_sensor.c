/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sensor.h"

#ifdef BUILD_HAS_SCMI_SENSOR_EVENTS
#    include <mod_scmi_sensor.h>
#endif
#include <mod_sensor.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_string.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static struct sensor_dev_ctx *ctx_table;
static struct mod_sensor_ctx sensor_mod_ctx;

struct sensor_dev_ctx *sensor_get_ctx(fwk_id_t id)
{
    return ctx_table + fwk_id_get_element_idx(id);
}

static int get_ctx_if_valid_call(fwk_id_t id,
                                 void *data,
                                 struct sensor_dev_ctx **ctx)
{
    fwk_assert(ctx != NULL);

    if (!fwk_expect(data != NULL)) {
        return FWK_E_PARAM;
    }

    *ctx = ctx_table + fwk_id_get_element_idx(id);

    return FWK_SUCCESS;
}

static inline void sensor_data_copy(
    struct mod_sensor_data *dest,
    const struct mod_sensor_data *origin)
{
#ifdef BUILD_HAS_SENSOR_MULTI_AXIS
    mod_sensor_value_t *value = dest->axis_value;
#endif

    fwk_str_memcpy(dest, origin, sizeof(struct mod_sensor_data));

#ifdef BUILD_HAS_SENSOR_MULTI_AXIS
    if (dest->axis_count > 1) {
        dest->axis_value = value;
        fwk_str_memcpy(
            dest->axis_value,
            origin->axis_value,
            sizeof(uint64_t) * dest->axis_count);
    }
#endif
}

#ifdef BUILD_HAS_SCMI_SENSOR_EVENTS
static bool trip_point_evaluate(
    struct sensor_trip_point_ctx *ctx,
    uint64_t value)
{
    uint64_t threshold;
    bool new_above_threshold, trigger = false;

    threshold = ctx->params.tp_value;
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
static void trip_point_process(fwk_id_t id, struct mod_sensor_data *data)
{
    struct sensor_dev_ctx *ctx;
    unsigned int i;

    fwk_check(!fwk_id_is_equal(id, FWK_ID_NONE));
    ctx = ctx_table + fwk_id_get_element_idx(id);

    if (!ctx->trip_point_ctx->enabled) {
        return;
    }

    for (i = 0; i < ctx->config->trip_point.count; i++) {
        if (trip_point_evaluate(&(ctx->trip_point_ctx[i]), data->value)) {
            /* Handle trip point event*/
            if (sensor_mod_ctx.sensor_trip_point_api != NULL)
                sensor_mod_ctx.sensor_trip_point_api->notify_sensor_trip_point(
                    id, ctx->trip_point_ctx->above_threshold, i);
        }
    }
}
#endif

static int is_sensor_enabled(fwk_id_t id, bool *sensor_is_enabled)
{
    int status;

    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        struct mod_sensor_complete_info complete_info;
        struct sensor_dev_ctx *ctx;

        ctx = &ctx_table[fwk_id_get_element_idx(id)];

        status = ctx->driver_api->get_info(
            ctx->config->driver_id, &(complete_info.hal_info));

        if (status != FWK_SUCCESS) {
            return status;
        }

        *sensor_is_enabled = !complete_info.hal_info.disabled;

        return FWK_SUCCESS;
    }

    return FWK_E_PARAM;
}

/*
 * Module API
 */
static int get_data(fwk_id_t id, struct mod_sensor_data *data)
{
    int status;
    bool sensor_enabled;
    struct sensor_dev_ctx *ctx;
    struct fwk_event req;
    struct mod_sensor_event_params *event_params =
        (struct mod_sensor_event_params *)req.params;

    status = get_ctx_if_valid_call(id, data, &ctx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = is_sensor_enabled(id, &sensor_enabled);
    if (status != FWK_SUCCESS) {
        return status;
    }

    if (!sensor_enabled) {
        return FWK_E_SUPPORT;
    }

    if (ctx->concurrency_readings.dequeuing) {
        /* Prevent new reading request while dequeuing pending readings
         * cached data is returned
         */
        sensor_data_copy(data, &ctx->last_read);
        return ctx->last_read.status;
    }

    if (ctx->concurrency_readings.pending_requests == 0) {
        status = ctx->driver_api->get_value(
            ctx->config->driver_id, &ctx->last_read.value);
        ctx->last_read.status = status;
        if (status == FWK_SUCCESS) {
#ifdef BUILD_HAS_SCMI_SENSOR_EVENTS
            trip_point_process(id, &ctx->last_read);
#endif
#ifdef BUILD_HAS_SENSOR_TIMESTAMP
            ctx->last_read.timestamp = sensor_get_timestamp(id);
#endif
            sensor_data_copy(data, &ctx->last_read);

            return status;
        } else if (status != FWK_PENDING) {
            return status;
        }
    }

    if (ctx->concurrency_readings.pending_requests >=
        SENSOR_MAX_PENDING_REQUESTS) {
        return FWK_E_BUSY;
    }

    req = (struct fwk_event){
        .target_id = id,
        .id = mod_sensor_event_id_read_request,
        .response_requested = true,
    };

    /* Save data address to copy return values in there */
    event_params->sensor_data = data;

    status = fwk_put_event(&req);
    if (status != FWK_SUCCESS) {
        return status;
    }

    ctx->concurrency_readings.pending_requests++;
    /*
     * We return FWK_PENDING here to indicate to the caller that the
     * result of the request is pending and will arrive later through
     * an event.
     */
    return FWK_PENDING;
}

static int get_info(fwk_id_t id, struct mod_sensor_complete_info *info)
{
    int status;
    struct sensor_dev_ctx *ctx;

    status = get_ctx_if_valid_call(id, info, &ctx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = ctx->driver_api->get_info(ctx->config->driver_id, &info->hal_info);
    if (!fwk_expect(status == FWK_SUCCESS)) {
        return FWK_E_DEVICE;
    }
    info->trip_point = ctx->config->trip_point;

#ifdef BUILD_HAS_SENSOR_TIMESTAMP
    status = sensor_get_timestamp_config(id, &info->timestamp);
    if (status == FWK_E_SUPPORT) {
        info->timestamp.timestamp_support = false;
    } else {
        return status;
    }
#endif
#ifdef BUILD_HAS_SENSOR_MULTI_AXIS
    if (ctx->axis_count > 1) {
        info->multi_axis.support = true;
        info->multi_axis.axis_count = ctx->axis_count;
    }
#endif

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

    if (trip_point_idx >= ctx->config->trip_point.count) {
        return FWK_E_PARAM;
    }

    *params = ctx->trip_point_ctx[trip_point_idx].params;

    return FWK_SUCCESS;
}

static int sensor_set_trip_point(
    fwk_id_t id,
    uint32_t trip_point_idx,
    struct mod_sensor_trip_point_params *params)
{
    struct sensor_dev_ctx *ctx;

    if (params == NULL) {
        return FWK_E_PARAM;
    }

    ctx = ctx_table + fwk_id_get_element_idx(id);

    if (trip_point_idx >= ctx->config->trip_point.count) {
        return FWK_E_PARAM;
    }

    ctx->trip_point_ctx[trip_point_idx].params = *params;

    /* Clear the trip point flag */
    ctx->trip_point_ctx[trip_point_idx].above_threshold = false;
    return FWK_SUCCESS;
}

static int sensor_enable(fwk_id_t id)
{
    int status;

    struct sensor_dev_ctx *ctx;
    struct mod_sensor_complete_info info;

    status = get_ctx_if_valid_call(id, &info, &ctx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    ctx = &ctx_table[fwk_id_get_element_idx(id)];

    if (ctx->driver_api->enable != NULL) {
        return ctx->driver_api->enable(id);
    }

    return FWK_E_SUPPORT;
}

static int sensor_disable(fwk_id_t id)
{
    int status;

    struct sensor_dev_ctx *ctx;
    struct mod_sensor_complete_info info;

    status = get_ctx_if_valid_call(id, &info, &ctx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    ctx = &ctx_table[fwk_id_get_element_idx(id)];

    if (ctx->driver_api->disable != NULL) {
        return ctx->driver_api->disable(id);
    }

    return FWK_E_SUPPORT;
}

static int sensor_set_update_interval(
    fwk_id_t id,
    unsigned int time_interval,
    int time_interval_multiplier)
{
    struct sensor_dev_ctx *ctx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_E_PARAM;
    }

    ctx = &ctx_table[fwk_id_get_element_idx(id)];

    if (ctx->driver_api->set_update_interval == NULL) {
        return FWK_E_SUPPORT;
    }

    return ctx->driver_api->set_update_interval(
        id, time_interval, time_interval_multiplier);
}

static int sensor_get_update_interval(
    fwk_id_t id,
    unsigned int *time_interval,
    int *time_interval_multiplier)
{
    int status;
    struct sensor_dev_ctx *ctx;
    struct mod_sensor_complete_info complete_info;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_E_PARAM;
    }

    ctx = &ctx_table[fwk_id_get_element_idx(id)];

    status = ctx->driver_api->get_info(
        ctx->config->driver_id, &(complete_info.hal_info));

    if (status != FWK_SUCCESS) {
        return status;
    }

    *time_interval = complete_info.hal_info.update_interval;
    *time_interval_multiplier =
        complete_info.hal_info.update_interval_multiplier;

    return FWK_SUCCESS;
}

static struct mod_sensor_api sensor_api = {
    .get_data = get_data,
    .get_info = get_info,
    .get_trip_point = sensor_get_trip_point,
    .set_trip_point = sensor_set_trip_point,
    .enable = sensor_enable,
    .disable = sensor_disable,
    .set_update_interval = sensor_set_update_interval,
    .get_update_interval = sensor_get_update_interval,
#ifdef BUILD_HAS_SENSOR_TIMESTAMP
    .set_timestamp_config = sensor_set_timestamp_config,
    .get_timestamp_config = sensor_get_timestamp_config,
#endif
#ifdef BUILD_HAS_SENSOR_MULTI_AXIS
    .get_axis_info = sensor_get_axis_info,
#endif
};

/*
 * Driver response API.
 */
static void reading_complete(fwk_id_t dev_id,
                             struct mod_sensor_driver_resp_params *response)
{
    int status = FWK_SUCCESS;
    struct fwk_event event;
    struct sensor_dev_ctx *ctx;

    if (!fwk_expect(fwk_id_get_module_idx(dev_id) == FWK_MODULE_IDX_SENSOR)) {
        return;
    }

    ctx = &ctx_table[fwk_id_get_element_idx(dev_id)];
    event = (struct fwk_event) {
        .id = mod_sensor_event_id_read_complete,
        .source_id = ctx->config->driver_id,
        .target_id = dev_id,
    };

    if (response != NULL) {
        ctx->last_read.status = response->status;

#ifdef BUILD_HAS_SENSOR_TIMESTAMP
        ctx->last_read.timestamp = sensor_get_timestamp(dev_id);
#endif
#ifdef BUILD_HAS_SENSOR_MULTI_AXIS
        if (ctx->axis_count > 1) {
            fwk_str_memcpy(
                ctx->last_read.axis_value,
                response->axis_value,
                sizeof(uint64_t) * ctx->axis_count);
        } else {
            ctx->last_read.value = response->value;
        }
#else
        ctx->last_read.value = response->value;
#endif

#ifdef BUILD_HAS_SCMI_SENSOR_EVENTS
        trip_point_process(dev_id, &ctx->last_read);
#endif
    } else {
        ctx->last_read.status = FWK_E_DEVICE;
    }

    ctx->concurrency_readings.dequeuing = true;

    status = fwk_put_event(&event);
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
    fwk_str_memset(&sensor_mod_ctx, 0, sizeof(sensor_mod_ctx));
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
        ctx->trip_point_ctx->enabled = true;
    } else {
        ctx->trip_point_ctx = NULL;
    }

    /* Pre-init last read with an invalid status */
    ctx->last_read.status = FWK_E_DEVICE;

#ifndef BUILD_HAS_SENSOR_MULTI_AXIS
    ctx->axis_count = 1;
#endif
#ifdef BUILD_HAS_SENSOR_TIMESTAMP
    return sensor_timestamp_dev_init(element_id, ctx);
#else
    return FWK_SUCCESS;
#endif
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
        if (sensor_mod_ctx.config == NULL) {
            return FWK_SUCCESS;
        }

#ifdef BUILD_HAS_NOTIFICATION
        if (fwk_id_is_equal(
                sensor_mod_ctx.config->notification_id, FWK_ID_NONE)) {
            return FWK_SUCCESS;
        }

        return fwk_module_bind(
            sensor_mod_ctx.config->notification_id,
            sensor_mod_ctx.config->trip_point_api_id,
            &sensor_mod_ctx.sensor_trip_point_api);
#else
        return FWK_SUCCESS;
#endif
    }
    ctx = ctx_table + fwk_id_get_element_idx(id);
    /* Bind to driver */
    status = fwk_module_bind(ctx->config->driver_id,
        ctx->config->driver_api_id,
        &driver);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* Validate driver API */
    if ((driver == NULL) || (driver->get_value == NULL)) {
        return FWK_E_DATA;
    }

    ctx->driver_api = driver;

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_SENSOR_MULTI_AXIS
int sensor_start(fwk_id_t id)
{
    int status;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }
    status = sensor_axis_start(id);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}
#endif

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
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT)) {
            return FWK_E_PARAM;
        }

        ctx = ctx_table + fwk_id_get_element_idx(target_id);
        driver_id = ctx->config->driver_id;

        /* Allow element to sub-element binding */
        if ((fwk_id_get_module_idx(driver_id) ==
            fwk_id_get_module_idx(source_id)) &&
            (fwk_id_get_element_idx(driver_id) ==
            fwk_id_get_element_idx(source_id))) {

            *api = &sensor_driver_response_api;

            return FWK_SUCCESS;
        } else {
            return FWK_E_ACCESS;
        }
    }

    return FWK_E_PARAM;
}

static int process_pending_requests(
    fwk_id_t dev_id,
    const struct mod_sensor_data *event_params)
{
    int status;
    bool list_is_empty;
    struct fwk_event delayed_response;
    struct mod_sensor_event_params *response_params =
        (struct mod_sensor_event_params *)delayed_response.params;
    struct sensor_dev_ctx *ctx;

    status = fwk_is_delayed_response_list_empty(dev_id, &list_is_empty);
    if (status != FWK_SUCCESS) {
        return status;
    }

    ctx = &ctx_table[fwk_id_get_element_idx(dev_id)];

    for (; !list_is_empty && ctx->concurrency_readings.pending_requests > 0;
         ctx->concurrency_readings.pending_requests--) {
        status = fwk_get_first_delayed_response(dev_id, &delayed_response);
        if (status != FWK_SUCCESS) {
            return status;
        }

        sensor_data_copy(response_params->sensor_data, &ctx->last_read);

        status = fwk_put_event(&delayed_response);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = fwk_is_delayed_response_list_empty(dev_id, &list_is_empty);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    ctx->concurrency_readings.pending_requests = 0;
    ctx->concurrency_readings.dequeuing = false;

    return FWK_SUCCESS;
}

static int sensor_process_event(const struct fwk_event *event,
                                struct fwk_event *resp_event)
{
    int status;
    struct sensor_dev_ctx *ctx;
    struct fwk_event read_req_event;
    struct mod_sensor_event_params *event_params =
        (struct mod_sensor_event_params *)read_req_event.params;
    enum mod_sensor_event_idx event_id_type;

    if (!fwk_module_is_valid_element_id(event->target_id)) {
        return FWK_E_PARAM;
    }

    ctx = ctx_table + fwk_id_get_element_idx(event->target_id);

    event_id_type = (enum mod_sensor_event_idx)fwk_id_get_event_idx(event->id);

    switch (event_id_type) {
    case SENSOR_EVENT_IDX_READ_REQUEST:
        if (ctx->concurrency_readings.pending_requests == 1) {
            /*
             * We keep the cookie event of the request that triggers the
             * reading.
             */
            ctx->cookie = event->cookie;
        }
        resp_event->is_delayed_response = true;

        return FWK_SUCCESS;

    case SENSOR_EVENT_IDX_READ_COMPLETE:
        status = fwk_get_delayed_response(
            event->target_id, ctx->cookie, &read_req_event);
        if (status != FWK_SUCCESS) {
            return status;
        }

        sensor_data_copy(
            (struct mod_sensor_data *)event_params->sensor_data,
            &ctx->last_read);

        status = fwk_put_event(&read_req_event);
        if (status != FWK_SUCCESS) {
            return status;
        }

        /*
         * After a read complete event all pending requests are processed.
         * We are processing pending events until it reaches a new reading
         * or the event queue is empty.
         */
        return process_pending_requests(
            event->target_id, (const struct mod_sensor_data *)event->params);

    default:
        return FWK_E_PARAM;
    }
}

const struct fwk_module module_sensor = {
    .api_count = (unsigned int)MOD_SENSOR_API_IDX_COUNT,
    .event_count = (unsigned int)SENSOR_EVENT_IDX_COUNT,
    .type = FWK_MODULE_TYPE_HAL,
    .init = sensor_init,
    .element_init = sensor_dev_init,
    .bind = sensor_bind,
#ifdef BUILD_HAS_SENSOR_MULTI_AXIS
    .start = sensor_start,
#endif
    .process_bind_request = sensor_process_bind_request,
    .process_event = sensor_process_event,
};

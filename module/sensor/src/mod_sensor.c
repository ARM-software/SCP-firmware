/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <sensor.h>

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
    } else if (status == FWK_SUCCESS)
        return FWK_SUCCESS;
    else
        return FWK_E_DEVICE;
}

static int get_info(fwk_id_t id, struct mod_sensor_info *info)
{
    int status;
    struct sensor_dev_ctx *ctx;

    status = get_ctx_if_valid_call(id, info, &ctx);
    if (status != FWK_SUCCESS)
        return status;

    status = ctx->driver_api->get_info(ctx->config->driver_id, info);
    if (!fwk_expect(status == FWK_SUCCESS))
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}

static struct mod_sensor_api sensor_api = {
    .get_value = get_value,
    .get_info  = get_info,
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
    struct mod_sensor_event_params *event_params =
        (struct mod_sensor_event_params *)event.params;

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
static int sensor_init(fwk_id_t module_id,
                       unsigned int element_count,
                       const void *unused)
{
    ctx_table = fwk_mm_calloc(element_count, sizeof(ctx_table[0]));

    return FWK_SUCCESS;
}

static int sensor_dev_init(fwk_id_t element_id,
                           unsigned int unused,
                           const void *data)
{
    struct sensor_dev_ctx *ctx;
    struct mod_sensor_dev_config *config;

    ctx = ctx_table + fwk_id_get_element_idx(element_id);
    config = (struct mod_sensor_dev_config*)data;

    ctx->config = config;

    return FWK_SUCCESS;
}

static int sensor_bind(fwk_id_t id, unsigned int round)
{
    struct sensor_dev_ctx *ctx;
    int status;
    struct mod_sensor_driver_api *driver = NULL;

    if ((round > 0) || fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        /*
         * Only bind in first round of calls
         * Nothing to do for module
         */
        return FWK_SUCCESS;
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
        (struct mod_sensor_event_params *)(event->params);
    struct mod_sensor_event_params *resp_params =
        (struct mod_sensor_event_params *)(read_req_event.params);

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

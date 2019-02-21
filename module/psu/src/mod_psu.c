/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_mm.h>
#include <fwk_thread.h>
#include <mod_psu.h>

/* "Set enabled" event */
struct mod_psu_event_params_set_enabled {
    bool enable;
};

/* "Set voltage" event */
struct mod_psu_event_params_set_voltage {
    uintmax_t voltage;
};

/* Device context */
struct mod_psu_device_ctx {
    /* Device configuration */
    const struct mod_psu_device_config *config;

    struct {
        /* Driver API */
        const struct mod_psu_driver_api *driver;
    } apis;
};

/* Device context table */
static struct mod_psu_device_ctx (*device_ctx)[];

static struct mod_psu_device_ctx *mod_psu_get_device_ctx(fwk_id_t device_id)
{
    unsigned int element_idx = fwk_id_get_element_idx(device_id);

    return &(*device_ctx)[element_idx];
}

static struct mod_psu_device_ctx *mod_psu_get_valid_device_ctx(
    fwk_id_t device_id)
{
    if (fwk_module_check_call(device_id) != FWK_SUCCESS)
        return NULL;

    return mod_psu_get_device_ctx(device_id);
}

int mod_psu_event_set_enabled(
    const struct fwk_event *event,
    struct fwk_event *response)
{
    const struct mod_psu_device_ctx *ctx;
    const struct mod_psu_event_params_set_enabled *params;
    struct mod_psu_event_params_set_enabled_response *response_params;

    /* These conditions were checked when we submitted the event */
    assert(fwk_id_get_module_idx(event->target_id) == FWK_MODULE_IDX_PSU);
    assert(fwk_module_is_valid_element_id(event->target_id));

    /* Explicitly cast to our parameter types */
    params = (void *)&event->params;
    response_params = (void *)&response->params;

    ctx = mod_psu_get_device_ctx(event->target_id);

    /* Set the enabled state through the driver */
    response_params->status = ctx->apis.driver->set_enabled(
        ctx->config->driver_id,
        params->enable);

    return FWK_SUCCESS;
}

int mod_psu_event_set_voltage(
    const struct fwk_event *event,
    struct fwk_event *response)
{
    const struct mod_psu_device_ctx *ctx;
    const struct mod_psu_event_params_set_voltage *params;
    struct mod_psu_event_params_set_voltage_response *response_params;

    /* These conditions were checked when we submitted the event */
    assert(fwk_id_get_module_idx(event->target_id) == FWK_MODULE_IDX_PSU);
    assert(fwk_module_is_valid_element_id(event->target_id));

    /* Explicitly cast to our parameter types */
    params = (void *)&event->params;
    response_params = (void *)&response->params;

    ctx = mod_psu_get_device_ctx(event->target_id);

    /* Set the voltage through the driver */
    response_params->status = ctx->apis.driver->set_voltage(
        ctx->config->driver_id,
        params->voltage);

    return FWK_SUCCESS;
}

static int mod_psu_process_event(
    const struct fwk_event *event,
    struct fwk_event *response)
{
    typedef int (*handler_t)(
        const struct fwk_event *event,
        struct fwk_event *response);

    static const handler_t handlers[] = {
        [MOD_PSU_EVENT_IDX_SET_ENABLED] = mod_psu_event_set_enabled,
        [MOD_PSU_EVENT_IDX_SET_VOLTAGE] = mod_psu_event_set_voltage,
    };

    unsigned int event_idx;
    handler_t handler;

    /* We only handle the events defined by us */
    if (fwk_id_get_module_idx(event->id) != FWK_MODULE_IDX_PSU)
        return FWK_E_PARAM;

    /* Ensure the event index is within bounds we can handle */
    event_idx = fwk_id_get_event_idx(event->id);
    if (event_idx >= FWK_ARRAY_SIZE(handlers))
        return FWK_E_PARAM;

    /* Ensure we have an implemented handler for this event */
    handler = handlers[event_idx];
    if (handler == NULL)
        return FWK_E_PARAM;

    /* Delegate event handling to the relevant handler */
    return handler(event, response);
}

static int api_get_enabled(fwk_id_t device_id, bool *enabled)
{
    int status;
    const struct mod_psu_device_ctx *ctx;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_PSU)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to a valid element */
    if (!fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    /* Validate the API call */
    status = fwk_module_check_call(device_id);
    if (status != FWK_SUCCESS)
        return FWK_E_STATE;

    ctx = mod_psu_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    /* Get the enabled state from the driver */
    status = ctx->apis.driver->get_enabled(ctx->config->driver_id, enabled);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int api_set_enabled(fwk_id_t device_id, bool enable)
{
    int status;
    const struct mod_psu_device_ctx *ctx;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_PSU)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to a valid element */
    if (!fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    /* Validate the API call */
    status = fwk_module_check_call(device_id);
    if (status != FWK_SUCCESS)
        return FWK_E_STATE;

    ctx = mod_psu_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    /* Set the enabled state through the driver */
    status = ctx->apis.driver->set_enabled(ctx->config->driver_id, enable);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int api_set_enabled_async(fwk_id_t device_id, bool enable)
{
    int status;
    struct fwk_event event;
    struct mod_psu_event_params_set_enabled *params;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_PSU)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to an existing element */
    if (fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    /* Validate the API call */
    status = fwk_module_check_call(device_id);
    if (status != FWK_SUCCESS)
        return FWK_E_STATE;

    /* Build and submit the event */
    event = (struct fwk_event) {
        .id = mod_psu_event_id_set_enabled,
        .target_id = device_id,
        .response_requested = true,
    };

    params = (void *)&event.params;
    *params = (struct mod_psu_event_params_set_enabled) {
        .enable = enable,
    };

    /* Submit the event for processing */
    status = fwk_thread_put_event(&event);
    if (status == FWK_E_NOMEM)
        return FWK_E_NOMEM;
    else if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

static int api_get_voltage(fwk_id_t device_id, uintmax_t *voltage)
{
    int status;
    const struct mod_psu_device_ctx *ctx;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_PSU)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to a valid element */
    if (!fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    /* Validate the API call */
    status = fwk_module_check_call(device_id);
    if (status != FWK_SUCCESS)
        return FWK_E_STATE;

    ctx = mod_psu_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    /* Get the voltage from the driver */
    status = ctx->apis.driver->get_voltage(ctx->config->driver_id, voltage);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int api_set_voltage(fwk_id_t device_id, uintmax_t voltage)
{
    int status;
    const struct mod_psu_device_ctx *ctx;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_PSU)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to a valid element */
    if (!fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    /* Validate the API call */
    status = fwk_module_check_call(device_id);
    if (status != FWK_SUCCESS)
        return FWK_E_STATE;

    ctx = mod_psu_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    /* Set the voltage state through the driver */
    status = ctx->apis.driver->set_voltage(ctx->config->driver_id, voltage);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int api_set_voltage_async(fwk_id_t device_id, uintmax_t voltage)
{
    int status;
    struct fwk_event event;
    struct mod_psu_event_params_set_voltage *params;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_PSU)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to an existing element */
    if (fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    /* Validate the API call */
    status = fwk_module_check_call(device_id);
    if (status != FWK_SUCCESS)
        return FWK_E_STATE;

    /* Build and submit the event */
    event = (struct fwk_event) {
        .id = mod_psu_event_id_set_enabled,
        .target_id = device_id,
        .response_requested = true,
    };

    params = (void *)&event.params;
    *params = (struct mod_psu_event_params_set_voltage) {
        .voltage = voltage,
    };

    status = fwk_thread_put_event(&event);
    if (status == FWK_E_NOMEM)
        return FWK_E_NOMEM;
    else if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

/* Module API implementation */
static const struct mod_psu_device_api mod_psu_device_api = {
    .get_enabled = api_get_enabled,
    .set_enabled = api_set_enabled,
    .set_enabled_async = api_set_enabled_async,
    .get_voltage = api_get_voltage,
    .set_voltage = api_set_voltage,
    .set_voltage_async = api_set_voltage_async,
};

static int psu_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    device_ctx = fwk_mm_calloc(
        element_count,
        sizeof((*device_ctx)[0]));
    if (device_ctx == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int psu_element_init(
    fwk_id_t device_id,
    unsigned int sub_element_count,
    const void *data)
{
    assert(sub_element_count == 0);

    mod_psu_get_device_ctx(device_id)->config = data;

    return FWK_SUCCESS;
}

static int psu_bind_element(fwk_id_t device_id, unsigned int round)
{
    int status;
    const struct mod_psu_device_ctx *ctx;

    /* Only handle the first round */
    if (round > 0)
        return FWK_SUCCESS;

    ctx = mod_psu_get_device_ctx(device_id);

    /* Bind to the driver */
    status = fwk_module_bind(
        ctx->config->driver_id,
        ctx->config->driver_api_id,
        &ctx->apis.driver);
    if (status != FWK_SUCCESS) {
        assert(false);

        return FWK_E_PANIC;
    }

    assert(ctx->apis.driver->set_enabled != NULL);
    assert(ctx->apis.driver->get_enabled != NULL);
    assert(ctx->apis.driver->set_voltage != NULL);
    assert(ctx->apis.driver->get_voltage != NULL);

    return FWK_SUCCESS;
}

static int psu_bind(fwk_id_t id, unsigned int round)
{
    /* We only need to handle element binding */
    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return psu_bind_element(id, round);

    return FWK_SUCCESS;
}

static int psu_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    /* Only accept binds to the elements */
    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_PARAM;

    /* Only expose the device API */
    if (!fwk_id_is_equal(api_id, mod_psu_api_id_psu_device))
        return FWK_E_PARAM;

    *api = &mod_psu_device_api;

    return FWK_SUCCESS;
}

/* Module description */
const struct fwk_module module_psu = {
    .name = "PSU",
    .type = FWK_MODULE_TYPE_HAL,
    .init = psu_init,
    .element_init = psu_element_init,
    .bind = psu_bind,
    .process_bind_request = psu_process_bind_request,
    .process_event = mod_psu_process_event,
    .api_count = MOD_PSU_API_IDX_COUNT,
    .event_count = MOD_PSU_EVENT_IDX_COUNT,
};

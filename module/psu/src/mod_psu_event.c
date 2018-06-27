/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_psu_private.h>

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

    ctx = __mod_psu_get_device_ctx(event->target_id);

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

    ctx = __mod_psu_get_device_ctx(event->target_id);

    /* Set the voltage through the driver */
    response_params->status = ctx->apis.driver->set_voltage(
        ctx->config->driver_id,
        params->voltage);

    return FWK_SUCCESS;
}

int __mod_psu_process_event(
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

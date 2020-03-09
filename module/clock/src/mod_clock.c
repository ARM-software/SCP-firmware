/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clock.h>

#include <mod_clock.h>
#include <mod_power_domain.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Device context */
struct clock_dev_ctx {
    /* Pointer to the element configuration data */
    const struct mod_clock_dev_config *config;

    /* Driver API */
    struct mod_clock_drv_api *api;

    /* Cookie for the pre-transition notification response */
    unsigned int pd_pre_power_transition_notification_cookie;

    /* Counter of notifications sent */
    unsigned int transition_pending_notifications_sent;

    /* Status of the pending transition */
    unsigned int transition_pending_response_status;

    /* A request is on-going */
    bool is_request_ongoing;

    /* Cookie for the response event */
    uint32_t cookie;
};

/* Module context */
struct clock_ctx {
    /* Pointer to the module configuration data */
    const struct mod_clock_config *config;

    /* Table of elements context */
    struct clock_dev_ctx *dev_ctx_table;
};

static struct clock_ctx module_ctx;

/*
 * Utility functions
 */

static int process_response_event(const struct fwk_event *event)
{
    int status;
    struct fwk_event resp_event;
    struct clock_dev_ctx *ctx;
    struct mod_clock_driver_resp_params *event_params =
        (struct mod_clock_driver_resp_params *)event->params;
    struct mod_clock_resp_params *resp_params =
        (struct mod_clock_resp_params *)resp_event.params;

    ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(event->target_id)];

    status = fwk_thread_get_delayed_response(event->target_id,
                                             ctx->cookie,
                                             &resp_event);
    if (status != FWK_SUCCESS)
        return status;

    resp_params->status = event_params->status;
    resp_params->value = event_params->value;
    ctx->is_request_ongoing = false;

    return fwk_thread_put_event(&resp_event);
}

static int process_request_event(const struct fwk_event *event,
                                 struct fwk_event *resp_event)
{
    struct clock_dev_ctx *ctx;

    ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(event->target_id)];

    ctx->cookie = event->cookie;
    resp_event->is_delayed_response = true;

    return FWK_SUCCESS;
}

static int create_async_request(struct clock_dev_ctx *ctx, fwk_id_t clock_id)
{
    int status;
    struct fwk_event request_event;

    request_event = (struct fwk_event) {
        .target_id = clock_id,
        .id = mod_clock_event_id_request,
        .response_requested = true,
    };

    status = fwk_thread_put_event(&request_event);
    if (status != FWK_SUCCESS)
        return status;

    ctx->is_request_ongoing = true;

     /*
      * Signal the result of the request is pending and will arrive later
      * through an event.
      */
    return FWK_PENDING;
}

static int get_ctx(fwk_id_t clock_id, struct clock_dev_ctx **ctx)
{
    fwk_assert(fwk_module_is_valid_element_id(clock_id));

    *ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(clock_id)];

    return FWK_SUCCESS;
}

/*
 * Driver response API.
 */

void request_complete(fwk_id_t dev_id,
                      struct mod_clock_driver_resp_params *response)
{
    int status;
    struct fwk_event event;
    struct clock_dev_ctx *ctx;
    struct mod_clock_driver_resp_params *event_params =
        (struct mod_clock_driver_resp_params *)event.params;

    fwk_assert(fwk_module_is_valid_element_id(dev_id));

    ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(dev_id)];

    event = (struct fwk_event) {
        .id = mod_clock_event_id_response,
        .source_id = ctx->config->driver_id,
        .target_id = dev_id,
    };

    if (response != NULL) {
        event_params->status = response->status;
        event_params->value = response->value;
    } else
        event_params->status = FWK_E_PARAM;

    status = fwk_thread_put_event(&event);
    fwk_assert(status == FWK_SUCCESS);
}

static struct mod_clock_driver_response_api clock_driver_response_api = {
    .request_complete = request_complete,
};

/*
 * Module API functions
 */

static int clock_set_rate(fwk_id_t clock_id, uint64_t rate,
                          enum mod_clock_round_mode round_mode)
{
    int status;
    struct clock_dev_ctx *ctx;

    status = get_ctx(clock_id, &ctx);
    if (status != FWK_SUCCESS)
        return status;

    /* Concurrency is not supported */
    if (ctx->is_request_ongoing)
        return FWK_E_BUSY;

    status = ctx->api->set_rate(ctx->config->driver_id, rate, round_mode);
    if (status == FWK_PENDING)
        return create_async_request(ctx, clock_id);
    else
        return status;
}

static int clock_get_rate(fwk_id_t clock_id, uint64_t *rate)
{
    int status;
    struct clock_dev_ctx *ctx;

    status = get_ctx(clock_id, &ctx);
    if (status != FWK_SUCCESS)
        return status;

    if (rate == NULL)
        return FWK_E_PARAM;

    /* Concurrency is not supported */
    if (ctx->is_request_ongoing)
        return FWK_E_BUSY;

    status = ctx->api->get_rate(ctx->config->driver_id, rate);
    if (status == FWK_PENDING)
        return create_async_request(ctx, clock_id);
    else
        return status;
}

static int clock_get_rate_from_index(fwk_id_t clock_id, unsigned int rate_index,
                                     uint64_t *rate)
{
    int status;
    struct clock_dev_ctx *ctx;

    status = get_ctx(clock_id, &ctx);
    if (status != FWK_SUCCESS)
        return status;

    if (rate == NULL)
        return FWK_E_PARAM;

    return ctx->api->get_rate_from_index(ctx->config->driver_id, rate_index,
                                         rate);
}

static int clock_set_state(fwk_id_t clock_id, enum mod_clock_state state)
{
    int status;
    struct clock_dev_ctx *ctx;

    status = get_ctx(clock_id, &ctx);
    if (status != FWK_SUCCESS)
        return status;

    /* Concurrency is not supported */
    if (ctx->is_request_ongoing)
        return FWK_E_BUSY;

    status = ctx->api->set_state(ctx->config->driver_id, state);
    if (status == FWK_PENDING)
        return create_async_request(ctx, clock_id);
    else
        return status;
}

static int clock_get_state(fwk_id_t clock_id, enum mod_clock_state *state)
{
    int status;
    struct clock_dev_ctx *ctx;

    status = get_ctx(clock_id, &ctx);
    if (status != FWK_SUCCESS)
        return status;

    if (state == NULL)
        return FWK_E_PARAM;

    /* Concurrency is not supported */
    if (ctx->is_request_ongoing)
        return FWK_E_BUSY;

    status = ctx->api->get_state(ctx->config->driver_id, state);
    if (status == FWK_PENDING)
        return create_async_request(ctx, clock_id);
    else
        return status;
}

static int clock_get_info(fwk_id_t clock_id, struct mod_clock_info *info)
{
    int status;
    struct clock_dev_ctx *ctx;

    status = get_ctx(clock_id, &ctx);
    if (status != FWK_SUCCESS)
        return status;

    if (info == NULL)
        return FWK_E_PARAM;

    status = ctx->api->get_range(ctx->config->driver_id, &info->range);
    if (status != FWK_SUCCESS)
        return status;

    info->name = fwk_module_get_name(clock_id);

    return FWK_SUCCESS;
}

static const struct mod_clock_api clock_api = {
    .set_rate = clock_set_rate,
    .get_rate = clock_get_rate,
    .get_rate_from_index = clock_get_rate_from_index,
    .set_state = clock_set_state,
    .get_state = clock_get_state,
    .get_info = clock_get_info,
};

/*
 * Framework handler functions
 */

static int clock_init(fwk_id_t module_id, unsigned int element_count,
                      const void *data)
{
    const struct mod_clock_config *config = data;

    if (element_count == 0)
        return FWK_SUCCESS;

    if (config == NULL)
        return FWK_E_PARAM;

    module_ctx.config = config;
    module_ctx.dev_ctx_table = fwk_mm_calloc(element_count,
                                             sizeof(struct clock_dev_ctx));
    return FWK_SUCCESS;
}

static int clock_dev_init(fwk_id_t element_id, unsigned int sub_element_count,
                          const void *data)
{
    struct clock_dev_ctx *ctx;
    const struct mod_clock_dev_config *dev_config = data;

    ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(element_id)];
    ctx->config = dev_config;

    return FWK_SUCCESS;
}

static int clock_bind(fwk_id_t id, unsigned int round)
{
    struct clock_dev_ctx *ctx;

    if (round == 1)
        return FWK_SUCCESS;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        /* Only element binding is supported */
        return FWK_SUCCESS;
    }

    ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(id)];

    return fwk_module_bind(ctx->config->driver_id,
                           ctx->config->api_id,
                           &ctx->api);
}

static int clock_start(fwk_id_t id)
{
    int status;
    struct clock_dev_ctx *ctx;

    /* Nothing to be done at the module level */
    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return FWK_SUCCESS;

    ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(id)];

    if (fwk_id_is_type(ctx->config->pd_source_id, FWK_ID_TYPE_NONE))
         return FWK_SUCCESS;

    if ((ctx->api->process_power_transition != NULL) &&
        (fwk_id_is_type(
            module_ctx.config->pd_transition_notification_id,
            FWK_ID_TYPE_NOTIFICATION))) {
        status = fwk_notification_subscribe(
            module_ctx.config->pd_transition_notification_id,
            ctx->config->pd_source_id,
            id);
        if (status != FWK_SUCCESS)
            return status;
    }

    if ((ctx->api->process_pending_power_transition != NULL) &&
        (fwk_id_is_type(
            module_ctx.config->pd_pre_transition_notification_id,
            FWK_ID_TYPE_NOTIFICATION))) {
        status = fwk_notification_subscribe(
            module_ctx.config->pd_pre_transition_notification_id,
            ctx->config->pd_source_id,
            id);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int clock_process_bind_request(fwk_id_t source_id, fwk_id_t target_id,
                                      fwk_id_t api_id, const void **api)
{
    enum mod_clock_api_type api_type = fwk_id_get_api_idx(api_id);
    struct clock_dev_ctx *ctx;

    switch (api_type) {
    case MOD_CLOCK_API_TYPE_HAL:
        *api = &clock_api;

        return FWK_SUCCESS;

    case MOD_CLOCK_API_TYPE_DRIVER_RESPONSE:
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT))
            return FWK_E_PARAM;

        ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(target_id)];

        if (fwk_id_is_equal(source_id, ctx->config->driver_id))
            *api = &clock_driver_response_api;
        else
            return FWK_E_ACCESS;

        return FWK_SUCCESS;

    default:
        return FWK_E_ACCESS;
    }
}

static int clock_process_pd_pre_transition_notification(
    struct clock_dev_ctx *ctx,
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status;
    struct mod_pd_power_state_pre_transition_notification_params *pd_params;
    struct mod_pd_power_state_pre_transition_notification_resp_params
        *pd_resp_params;
    struct clock_notification_params *out_params;
    struct fwk_event outbound_event = {
        .response_requested = true,
        .id = mod_clock_notification_id_state_change_pending,
        .source_id = FWK_ID_NONE
    };

    pd_params = (struct mod_pd_power_state_pre_transition_notification_params *)
        event->params;
    pd_resp_params =
        (struct mod_pd_power_state_pre_transition_notification_resp_params *)
            resp_event->params;

    fwk_assert(ctx->api->process_pending_power_transition != NULL);
    status = ctx->api->process_pending_power_transition(
        ctx->config->driver_id,
        pd_params->current_state,
        pd_params->target_state);

    /*
     * The response to the notification should initially be the overall result
     * of the downwards propagation of the state change through the driver(s).
     */
    pd_resp_params->status = status;

    if (status != FWK_SUCCESS) {
        ctx->transition_pending_notifications_sent = 0;
        return status;
    }

    ctx->transition_pending_response_status = FWK_SUCCESS;
    out_params =
        (struct clock_notification_params *)outbound_event.params;

    /*
     * For now it is sufficient to assume that a PD ON state implies that
     * the clock is running and any other state implies that the clock has
     * stopped. This will likely need to be revisited so that the clock
     * driver can influence the resulting state that is propagated via the
     * outgoing notification.
     */
    out_params->new_state = (pd_params->target_state == MOD_PD_STATE_ON)
        ? MOD_CLOCK_STATE_RUNNING
        : MOD_CLOCK_STATE_STOPPED;

    /* Notify subscribers of the pending clock state change */
    status = fwk_notification_notify(
        &outbound_event,
        &(ctx->transition_pending_notifications_sent));
    if (status != FWK_SUCCESS) {
        pd_resp_params->status = status;
        return status;
    }

    if (ctx->transition_pending_notifications_sent > 0) {
        /* There are one or more subscribers that must respond */
        resp_event->is_delayed_response = true;
        ctx->pd_pre_power_transition_notification_cookie = event->cookie;
    }

    return status;
}

static int clock_process_pd_transition_notification(
    struct clock_dev_ctx *ctx,
    const struct fwk_event *event)
{
    int status;
    unsigned int transition_notifications_sent;
    struct mod_pd_power_state_transition_notification_params *pd_params;
    struct clock_notification_params* out_params;
    struct fwk_event outbound_event = {
        .response_requested = false,
        .id = mod_clock_notification_id_state_changed,
        .source_id = FWK_ID_NONE
    };

    pd_params =
        (struct mod_pd_power_state_transition_notification_params *)event
            ->params;

    fwk_assert(ctx->api->process_power_transition != NULL);
    status = ctx->api->process_power_transition(
        ctx->config->driver_id, pd_params->state);

    if (status != FWK_SUCCESS)
        return status;

    /* Notify subscribers of the clock state change */
    out_params = (struct clock_notification_params *)outbound_event.params;
    if (pd_params->state == MOD_PD_STATE_ON)
        out_params->new_state = MOD_CLOCK_STATE_RUNNING;
    else
        out_params->new_state = MOD_CLOCK_STATE_STOPPED;

    status = fwk_notification_notify(
        &outbound_event, &(transition_notifications_sent));

    return status;
}

static int clock_process_notification_response(
    struct clock_dev_ctx *ctx,
    const struct fwk_event *event)
{
    struct clock_state_change_pending_resp_params *resp_params;
    struct mod_pd_power_state_pre_transition_notification_resp_params
        *pd_resp_params;
    struct fwk_event pd_response_event = {
        .id = module_ctx.config->pd_pre_transition_notification_id,
        .target_id = ctx->config->pd_source_id,
        .cookie = ctx->pd_pre_power_transition_notification_cookie,
        .is_notification = true,
        .is_response = true,
        .is_delayed_response = true,
    };

    fwk_assert(fwk_id_is_equal(event->id,
                               mod_clock_notification_id_state_change_pending));

    /* At least one notification response must be outstanding */
    if (!fwk_expect(ctx->transition_pending_notifications_sent != 0))
        return FWK_E_PANIC;

    resp_params =
        (struct clock_state_change_pending_resp_params *)event->params;

    /*
     * Change the status used in the response to the power domain notification
     * if this response has a non-success status code, otherwise leave the
     * existing value alone. By default the status is FWK_SUCCESS and changing
     * it to an error status code will veto the power domain state transition
     * that is pending.
     */
    if (resp_params->status != FWK_SUCCESS)
        ctx->transition_pending_response_status = resp_params->status;

    if ((--(ctx->transition_pending_notifications_sent)) == 0) {
        /*
         * If this is the final response then the response to the power domain
         * notification can be sent.
         */
        pd_resp_params =
            (struct mod_pd_power_state_pre_transition_notification_resp_params
                 *)pd_response_event.params;
        pd_resp_params->status = ctx->transition_pending_response_status;
        fwk_thread_put_event(&pd_response_event);
    }

    return FWK_SUCCESS;
}

static int clock_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_dev_ctx *ctx;

    if (!fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT))
        /* Only elements should be registered for notifications */
        return FWK_E_PARAM;

    ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(event->target_id)];

    if (event->is_response)
        return clock_process_notification_response(ctx, event);

    if (fwk_id_is_equal(
            event->id, module_ctx.config->pd_transition_notification_id))
        return clock_process_pd_transition_notification(ctx, event);
    else if (fwk_id_is_equal(event->id,
                 module_ctx.config->pd_pre_transition_notification_id))
        return clock_process_pd_pre_transition_notification(
            ctx, event, resp_event);
    else
        return FWK_E_HANDLER;
}

static int clock_process_event(const struct fwk_event *event,
                               struct fwk_event *resp_event)
{
    enum clock_event_idx event_idx;

    if (!fwk_module_is_valid_element_id(event->target_id))
        return FWK_E_PARAM;

    event_idx = fwk_id_get_event_idx(event->id);

    switch (event_idx) {
    case CLOCK_EVENT_IDX_REQUEST:
        return process_request_event(event, resp_event);
    case CLOCK_EVENT_IDX_RESPONSE:
        return process_response_event(event);
    default:
        return FWK_E_PANIC;
    }
}

const struct fwk_module module_clock = {
    .name = "Clock HAL",
    .type = FWK_MODULE_TYPE_HAL,
    .api_count = MOD_CLOCK_API_COUNT,
    .event_count = CLOCK_EVENT_IDX_COUNT,
    .notification_count = MOD_CLOCK_NOTIFICATION_IDX_COUNT,
    .init = clock_init,
    .element_init = clock_dev_init,
    .bind = clock_bind,
    .start = clock_start,
    .process_bind_request = clock_process_bind_request,
    .process_notification = clock_process_notification,
    .process_event = clock_process_event,
};

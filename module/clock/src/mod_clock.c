/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock.h"

#include <mod_clock.h>
#include <mod_power_domain.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_list.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static struct clock_ctx mod_clock_ctx;

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

    ctx =
        &mod_clock_ctx.dev_ctx_table[fwk_id_get_element_idx(event->target_id)];

    status = fwk_get_delayed_response(
        event->target_id, ctx->request.cookie, &resp_event);
    if (status != FWK_SUCCESS) {
        return status;
    }

    resp_params->status = event_params->status;
    resp_params->value = event_params->value;
    ctx->request.is_ongoing = false;

    return fwk_put_event(&resp_event);
}

static int process_request_event(const struct fwk_event *event,
                                 struct fwk_event *resp_event)
{
    struct clock_dev_ctx *ctx;

    ctx =
        &mod_clock_ctx.dev_ctx_table[fwk_id_get_element_idx(event->target_id)];

    ctx->request.cookie = event->cookie;
    resp_event->is_delayed_response = true;

    return FWK_SUCCESS;
}

static int create_async_request(
    struct clock_dev_ctx *ctx,
    fwk_id_t clock_id,
    fwk_id_t event_id)
{
    int status;
    struct fwk_event request_event;

    request_event = (struct fwk_event) {
        .target_id = clock_id,
        .id = event_id,
        .response_requested = true,
    };

    status = fwk_put_event(&request_event);
    if (status != FWK_SUCCESS) {
        return status;
    }

    ctx->request.is_ongoing = true;

    /*
     * Signal the result of the request is pending and will arrive later
     * through an event.
     */
    return FWK_PENDING;
}

void clock_get_ctx(fwk_id_t clock_id, struct clock_dev_ctx **ctx)
{
    fwk_assert(fwk_module_is_valid_element_id(clock_id));

    *ctx = &mod_clock_ctx.dev_ctx_table[fwk_id_get_element_idx(clock_id)];
}

/*
 * Driver response API.
 */

void clock_request_complete(
    fwk_id_t dev_id,
    struct mod_clock_driver_resp_params *response)
{
    int status;
    struct fwk_event event;
    struct clock_dev_ctx *ctx;
    struct mod_clock_driver_resp_params *event_params =
        (struct mod_clock_driver_resp_params *)event.params;

    fwk_assert(fwk_module_is_valid_element_id(dev_id));

    ctx = &mod_clock_ctx.dev_ctx_table[fwk_id_get_element_idx(dev_id)];

    event = (struct fwk_event) {
        .id = mod_clock_event_id_response,
        .source_id = ctx->config->driver_id,
        .target_id = dev_id,
    };

    if (response != NULL) {
        event_params->status = response->status;
        event_params->value = response->value;
    } else {
        event_params->status = FWK_E_PARAM;
    }

    status = fwk_put_event(&event);
    fwk_assert(status == FWK_SUCCESS);
}

static struct mod_clock_driver_response_api clock_driver_response_api = {
    .request_complete = clock_request_complete,
};

/*
 * Module API functions
 */

static int clock_set_rate(fwk_id_t clock_id, uint64_t rate,
                          enum mod_clock_round_mode round_mode)
{
    int status;
    struct clock_dev_ctx *ctx;

#ifdef BUILD_HAS_CLOCK_TREE_MGMT
    struct fwk_event event;
    struct clock_set_rate_params *event_params;
#endif

    clock_get_ctx(clock_id, &ctx);

    /* Concurrency is not supported */
    if (ctx->request.is_ongoing) {
        return FWK_E_BUSY;
    }

#ifdef BUILD_HAS_CLOCK_TREE_MGMT

    status = ctx->api->set_rate(ctx->config->driver_id, rate, round_mode);
    if (status == FWK_PENDING) {
        return create_async_request(
            ctx, clock_id, mod_clock_event_id_set_rate_request);
    }
    if (clock_is_single_node(ctx)) {
        return status;
    }

    event = (struct fwk_event){
        .target_id = clock_id,
        .id = mod_clock_event_id_set_rate_pre_request,
    };
    event_params = (struct clock_set_rate_params *)event.params;
    event_params->input_rate = rate;

    return fwk_put_event(&event);
#else
    status = ctx->api->set_rate(ctx->config->driver_id, rate, round_mode);
    if (status == FWK_PENDING) {
        return create_async_request(
            ctx,
            clock_id,
            mod_clock_event_id_set_rate_request);
    }
    return status;
#endif
}

static int clock_get_rate(fwk_id_t clock_id, uint64_t *rate)
{
    int status;
    struct clock_dev_ctx *ctx;

    clock_get_ctx(clock_id, &ctx);

    if (rate == NULL) {
        return FWK_E_PARAM;
    }

    /* Concurrency is not supported */
    if (ctx->request.is_ongoing) {
        return FWK_E_BUSY;
    }

    status = ctx->api->get_rate(ctx->config->driver_id, rate);
    if (status == FWK_PENDING) {
        return create_async_request(
            ctx,
            clock_id,
            mod_clock_event_id_get_rate_request);
    } else {
        return status;
    }
}

static int clock_get_rate_from_index(fwk_id_t clock_id, unsigned int rate_index,
                                     uint64_t *rate)
{
    struct clock_dev_ctx *ctx;

    clock_get_ctx(clock_id, &ctx);

    if (rate == NULL) {
        return FWK_E_PARAM;
    }

    return ctx->api->get_rate_from_index(ctx->config->driver_id, rate_index,
                                         rate);
}

static int clock_set_state(fwk_id_t clock_id, enum mod_clock_state state)
{
    int status;
    struct clock_dev_ctx *ctx;

#ifdef BUILD_HAS_CLOCK_TREE_MGMT
    struct fwk_event event;
    struct clock_set_state_params *event_params;
#endif

    clock_get_ctx(clock_id, &ctx);
    /* Concurrency is not supported */
    if (ctx->request.is_ongoing) {
        return FWK_E_BUSY;
    }

#ifdef BUILD_HAS_CLOCK_TREE_MGMT
    if (ctx->state_transition.state != CLOCK_STATE_TRANSITION_IDLE) {
        return FWK_E_BUSY;
    }

    if (clock_is_single_node(ctx)) {
        status = ctx->api->set_state(ctx->config->driver_id, state);
        if (status == FWK_PENDING) {
            return create_async_request(
                ctx, clock_id, mod_clock_event_id_set_state_request);
        }
        return status;
    }

    status = create_async_request(
        ctx, clock_id, mod_clock_event_id_set_state_request);
    if (status != FWK_PENDING) {
        return status;
    }

    event = (struct fwk_event){
        .target_id = clock_id,
        .id = mod_clock_event_id_set_state_pre_request,
    };
    event_params = (struct clock_set_state_params *)event.params;
    event_params->target_state = state;
    ctx->state_transition.is_transition_initiator = true;
    status = fwk_put_event(&event);
    if (status != FWK_SUCCESS) {
        return status;
    }
    return FWK_PENDING;
#else
    status = ctx->api->set_state(ctx->config->driver_id, state);
    if (status == FWK_PENDING) {
        return create_async_request(
            ctx, clock_id, mod_clock_event_id_set_state_request);
    }
    return status;
#endif
}

static int clock_get_state(fwk_id_t clock_id, enum mod_clock_state *state)
{
    int status;
    struct clock_dev_ctx *ctx;

    clock_get_ctx(clock_id, &ctx);

    if (state == NULL) {
        return FWK_E_PARAM;
    }

    /* Concurrency is not supported */
    if (ctx->request.is_ongoing) {
        return FWK_E_BUSY;
    }

    status = ctx->api->get_state(ctx->config->driver_id, state);
    if (status == FWK_PENDING) {
        return create_async_request(
            ctx,
            clock_id,
            mod_clock_event_id_get_state_request);
    } else {
        return status;
    }
}

static int clock_get_info(fwk_id_t clock_id, struct mod_clock_info *info)
{
    int status;
    struct clock_dev_ctx *ctx;

    clock_get_ctx(clock_id, &ctx);

    if (info == NULL) {
        return FWK_E_PARAM;
    }

    status = ctx->api->get_range(ctx->config->driver_id, &info->range);
    if (status != FWK_SUCCESS) {
        return status;
    }

    info->name = fwk_module_get_element_name(clock_id);

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

    if (element_count == 0) {
        return FWK_SUCCESS;
    }

#ifdef BUILD_HAS_NOTIFICATION
    if (config == NULL) {
        return FWK_E_PARAM;
    }
#endif

    mod_clock_ctx.config = config;
    mod_clock_ctx.dev_ctx_table =
        fwk_mm_calloc(element_count, sizeof(struct clock_dev_ctx));
    mod_clock_ctx.dev_count = element_count;

    return FWK_SUCCESS;
}

static int clock_dev_init(fwk_id_t element_id, unsigned int sub_element_count,
                          const void *data)
{
    struct clock_dev_ctx *ctx;
    const struct mod_clock_dev_config *dev_config = data;

    ctx = &mod_clock_ctx.dev_ctx_table[fwk_id_get_element_idx(element_id)];
    ctx->config = dev_config;

#ifdef BUILD_HAS_CLOCK_TREE_MGMT
    fwk_list_init(&ctx->children_list);
    ctx->id = element_id;
#endif

    return FWK_SUCCESS;
}

static int clock_bind(fwk_id_t id, unsigned int round)
{
    struct clock_dev_ctx *ctx;
    int status;

    if (round == 1) {
        return FWK_SUCCESS;
    }

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        /* Only element binding is supported */
        return FWK_SUCCESS;
    }

    ctx = &mod_clock_ctx.dev_ctx_table[fwk_id_get_element_idx(id)];

    status =
        fwk_module_bind(ctx->config->driver_id, ctx->config->api_id, &ctx->api);
    if (status != FWK_SUCCESS) {
        return status;
    }

#ifdef FWK_MODULE_ID_POWER_DOMAIN
    if (ctx->config->default_on) {
        status = fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_POWER_DOMAIN),
            FWK_ID_API(FWK_MODULE_IDX_POWER_DOMAIN, MOD_PD_API_IDX_RESTRICTED),
            &ctx->pd_restricted_api);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }
#endif

    return FWK_SUCCESS;
}

static int clock_start(fwk_id_t id)
{
#if defined(BUILD_HAS_NOTIFICATION) || defined(FWK_MODULE_ID_POWER_DOMAIN)
    int status;
#endif
    struct clock_dev_ctx *ctx;
#ifdef FWK_MODULE_ID_POWER_DOMAIN
    unsigned int pd_state;
#endif

    /* Clock tree is initialized */
    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
#ifdef BUILD_HAS_CLOCK_TREE_MGMT
        return clock_connect_tree(&mod_clock_ctx);
#else
        return FWK_SUCCESS;
#endif
    }

    ctx = &mod_clock_ctx.dev_ctx_table[fwk_id_get_element_idx(id)];

    if (fwk_id_is_type(ctx->config->pd_source_id, FWK_ID_TYPE_NONE)) {
        return FWK_SUCCESS;
    }

#ifdef FWK_MODULE_ID_POWER_DOMAIN
    if (ctx->config->default_on && ctx->api->process_power_transition != NULL) {
        status = ctx->pd_restricted_api->get_state(
            ctx->config->pd_source_id, &pd_state);
        if (status != FWK_SUCCESS) {
            return status;
        }

        if (pd_state == MOD_PD_STATE_ON) {
            status = ctx->api->process_power_transition(
                ctx->config->driver_id, MOD_PD_STATE_ON);
            if (status != FWK_SUCCESS) {
                return status;
            }
        }
    }
#endif

#ifdef BUILD_HAS_NOTIFICATION
    if ((ctx->api->process_power_transition != NULL) &&
        (fwk_id_is_type(
            mod_clock_ctx.config->pd_transition_notification_id,
            FWK_ID_TYPE_NOTIFICATION))) {
        status = fwk_notification_subscribe(
            mod_clock_ctx.config->pd_transition_notification_id,
            ctx->config->pd_source_id,
            id);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    if ((ctx->api->process_pending_power_transition != NULL) &&
        (fwk_id_is_type(
            mod_clock_ctx.config->pd_pre_transition_notification_id,
            FWK_ID_TYPE_NOTIFICATION))) {
        status = fwk_notification_subscribe(
            mod_clock_ctx.config->pd_pre_transition_notification_id,
            ctx->config->pd_source_id,
            id);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }
#endif

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
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT)) {
            return FWK_E_PARAM;
        }

        ctx = &mod_clock_ctx.dev_ctx_table[fwk_id_get_element_idx(target_id)];

        if (fwk_id_is_equal(source_id, ctx->config->driver_id)) {
            *api = &clock_driver_response_api;
        } else {
            return FWK_E_ACCESS;
        }

        return FWK_SUCCESS;

    default:
        return FWK_E_ACCESS;
    }
}

#ifdef BUILD_HAS_NOTIFICATION

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
        ctx->pd_notif.transition_pending_sent = 0;
        return status;
    }

    ctx->pd_notif.transition_pending_response_status = FWK_SUCCESS;
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
        &outbound_event, &(ctx->pd_notif.transition_pending_sent));
    if (status != FWK_SUCCESS) {
        pd_resp_params->status = status;
        return status;
    }

    if (ctx->pd_notif.transition_pending_sent > 0) {
        /* There are one or more subscribers that must respond */
        resp_event->is_delayed_response = true;
        ctx->pd_notif.pre_power_transition_cookie = event->cookie;
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

    if (status != FWK_SUCCESS) {
        return status;
    }

    /* Notify subscribers of the clock state change */
    out_params = (struct clock_notification_params *)outbound_event.params;
    if (pd_params->state == MOD_PD_STATE_ON) {
        out_params->new_state = MOD_CLOCK_STATE_RUNNING;
    } else {
        out_params->new_state = MOD_CLOCK_STATE_STOPPED;
    }

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
        .id = mod_clock_ctx.config->pd_pre_transition_notification_id,
        .target_id = ctx->config->pd_source_id,
        .cookie = ctx->pd_notif.pre_power_transition_cookie,
        .is_notification = true,
        .is_response = true,
        .is_delayed_response = true,
    };

    fwk_assert(fwk_id_is_equal(event->id,
                               mod_clock_notification_id_state_change_pending));

    /* At least one notification response must be outstanding */
    if (!fwk_expect(ctx->pd_notif.transition_pending_sent != 0)) {
        return FWK_E_PANIC;
    }

    resp_params =
        (struct clock_state_change_pending_resp_params *)event->params;

    /*
     * Change the status used in the response to the power domain notification
     * if this response has a non-success status code, otherwise leave the
     * existing value alone. By default the status is FWK_SUCCESS and changing
     * it to an error status code will veto the power domain state transition
     * that is pending.
     */
    if (resp_params->status != FWK_SUCCESS) {
        ctx->pd_notif.transition_pending_response_status =
            (unsigned int)resp_params->status;
    }

    if ((--(ctx->pd_notif.transition_pending_sent)) == 0) {
        /*
         * If this is the final response then the response to the power domain
         * notification can be sent.
         */
        pd_resp_params =
            (struct mod_pd_power_state_pre_transition_notification_resp_params
                 *)pd_response_event.params;
        pd_resp_params->status =
            (int)ctx->pd_notif.transition_pending_response_status;
        return fwk_put_event(&pd_response_event);
    }

    return FWK_SUCCESS;
}

static int clock_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_dev_ctx *ctx;

    if (!fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT)) {
        /* Only elements should be registered for notifications */
        return FWK_E_PARAM;
    }

    ctx =
        &mod_clock_ctx.dev_ctx_table[fwk_id_get_element_idx(event->target_id)];

    if (event->is_response) {
        return clock_process_notification_response(ctx, event);
    }

    if (fwk_id_is_equal(
            event->id, mod_clock_ctx.config->pd_transition_notification_id)) {
        return clock_process_pd_transition_notification(ctx, event);
    } else if (fwk_id_is_equal(
                   event->id,
                   mod_clock_ctx.config->pd_pre_transition_notification_id)) {
        return clock_process_pd_pre_transition_notification(
            ctx, event, resp_event);
    } else {
        return FWK_E_HANDLER;
    }
}
#endif /* BUILD_HAS_NOTIFICATION */

static int clock_process_event(const struct fwk_event *event,
                               struct fwk_event *resp_event)
{
    if (!fwk_module_is_valid_element_id(event->target_id)) {
        return FWK_E_PARAM;
    }

    switch (fwk_id_get_event_idx(event->id)) {
    case (unsigned int)MOD_CLOCK_EVENT_IDX_SET_RATE_REQUEST:
    case (unsigned int)MOD_CLOCK_EVENT_IDX_GET_RATE_REQUEST:
    case (unsigned int)MOD_CLOCK_EVENT_IDX_SET_STATE_REQUEST:
    case (unsigned int)MOD_CLOCK_EVENT_IDX_GET_STATE_REQUEST:
        return process_request_event(event, resp_event);

#ifdef BUILD_HAS_CLOCK_TREE_MGMT
    case (unsigned int)CLOCK_EVENT_IDX_SET_STATE_PRE_REQUEST:
        return clock_management_process_state(event);

    case (unsigned int)CLOCK_EVENT_IDX_SET_RATE_PRE_REQUEST:
        return clock_management_process_rate(event);
#endif

    case (unsigned int)CLOCK_EVENT_IDX_RESPONSE:
        return process_response_event(event);

    default:
        return FWK_E_PANIC;
    }
}

const struct fwk_module module_clock = {
    .type = FWK_MODULE_TYPE_HAL,
    .api_count = (unsigned int)MOD_CLOCK_API_COUNT,
    .event_count = (unsigned int)CLOCK_EVENT_IDX_COUNT,
#ifdef BUILD_HAS_NOTIFICATION
    .notification_count = (unsigned int)MOD_CLOCK_NOTIFICATION_IDX_COUNT,
#endif
    .init = clock_init,
    .element_init = clock_dev_init,
    .bind = clock_bind,
    .start = clock_start,
    .process_bind_request = clock_process_bind_request,
#ifdef BUILD_HAS_NOTIFICATION
    .process_notification = clock_process_notification,
#endif
    .process_event = clock_process_event,
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_errno.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_thread.h>
#include <fwk_notification.h>
#include <mod_clock.h>
#include <mod_power_domain.h>

/* Device context */
struct clock_dev_ctx {
    const struct mod_clock_dev_config *config;
    struct mod_clock_drv_api *api;
    unsigned int pd_pre_power_transition_notification_cookie;
    unsigned int transition_pending_notifications_sent;
    unsigned int transition_pending_response_status;
};

/* Module context */
struct clock_ctx {
    const struct mod_clock_config *config;
    struct clock_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
};

struct clock_ctx module_ctx;

/*
 * Module API functions
 */

static int clock_set_rate(fwk_id_t clock_id, uint64_t rate,
                          enum mod_clock_round_mode round_mode)
{
    int status;
    struct clock_dev_ctx *ctx;

    status = fwk_module_check_call(clock_id);
    if (status != FWK_SUCCESS)
        return status;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(clock_id);

    return ctx->api->set_rate(ctx->config->driver_id, rate, round_mode);
}

static int clock_get_rate(fwk_id_t clock_id, uint64_t *rate)
{
    int status;
    struct clock_dev_ctx *ctx;

    status = fwk_module_check_call(clock_id);
    if (status != FWK_SUCCESS)
        return status;

    if (rate == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(clock_id);

    return ctx->api->get_rate(ctx->config->driver_id, rate);
}

static int clock_get_rate_from_index(fwk_id_t clock_id, unsigned int rate_index,
                                     uint64_t *rate)
{
    int status;
    struct clock_dev_ctx *ctx;

    status = fwk_module_check_call(clock_id);
    if (status != FWK_SUCCESS)
        return status;

    if (rate == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(clock_id);

    return ctx->api->get_rate_from_index(ctx->config->driver_id, rate_index,
                                         rate);
}

static int clock_set_state(fwk_id_t clock_id, enum mod_clock_state state)
{
    int status;
    struct clock_dev_ctx *ctx;

    status = fwk_module_check_call(clock_id);
    if (status != FWK_SUCCESS)
        return status;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(clock_id);

    return ctx->api->set_state(ctx->config->driver_id, state);
}

static int clock_get_state(fwk_id_t clock_id, enum mod_clock_state *state)
{
    int status;
    struct clock_dev_ctx *ctx;

    status = fwk_module_check_call(clock_id);
    if (status != FWK_SUCCESS)
        return status;

    if (state == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(clock_id);

    return ctx->api->get_state(ctx->config->driver_id, state);
}

static int clock_get_info(fwk_id_t clock_id, struct mod_clock_info *info)
{
    int status;
    struct clock_dev_ctx *ctx;

    status = fwk_module_check_call(clock_id);
    if (status != FWK_SUCCESS)
        return status;

    if (info == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(clock_id);

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

    module_ctx.dev_count = element_count;

    if (element_count == 0)
        return FWK_SUCCESS;

    if (config == NULL)
        return FWK_E_PARAM;

    module_ctx.config = config;
    module_ctx.dev_ctx_table = fwk_mm_calloc(element_count,
                                             sizeof(struct clock_dev_ctx));
    if (module_ctx.dev_ctx_table == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int clock_dev_init(fwk_id_t element_id, unsigned int sub_element_count,
                          const void *data)
{
    struct clock_dev_ctx *ctx;
    const struct mod_clock_dev_config *dev_config = data;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(element_id);
    ctx->config = dev_config;

    return FWK_SUCCESS;
}

static int clock_bind(fwk_id_t id, unsigned int round)
{
    struct clock_dev_ctx *ctx;

    if (round == 1)
        return FWK_SUCCESS;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        /* Only element binding is supported */
        return FWK_SUCCESS;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(id);

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

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(id);

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
    if (fwk_id_get_api_idx(api_id) != MOD_CLOCK_API_TYPE_HAL)
        /* The requested API is not supported. */
        return FWK_E_ACCESS;

    *api = &clock_api;
    return FWK_SUCCESS;
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
    };

    pd_params = (struct mod_pd_power_state_pre_transition_notification_params *)
        event->params;
    pd_resp_params =
        (struct mod_pd_power_state_pre_transition_notification_resp_params *)
            resp_event->params;

    assert(ctx->api->process_pending_power_transition != NULL);
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

    if (ctx->transition_pending_notifications_sent > 0) {
        /* There are one or more subscribers that must respond */
        resp_event->is_delayed_response = true;
        ctx->pd_pre_power_transition_notification_cookie = event->cookie;
    }

    return FWK_SUCCESS;
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
    };

    pd_params =
        (struct mod_pd_power_state_transition_notification_params *)event
            ->params;

    assert(ctx->api->process_power_transition != NULL);
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

    return FWK_SUCCESS;
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

    assert(fwk_id_is_equal(event->id,
                           mod_clock_notification_id_state_change_pending));

    /* At least one notification response must be outstanding */
    if (ctx->transition_pending_notifications_sent == 0) {
        assert(false);
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

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(event->target_id);

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

const struct fwk_module module_clock = {
    .name = "Clock HAL",
    .type = FWK_MODULE_TYPE_HAL,
    .api_count = MOD_CLOCK_API_COUNT,
    .event_count = 0,
    .notification_count = MOD_CLOCK_NOTIFICATION_IDX_COUNT,
    .init = clock_init,
    .element_init = clock_dev_init,
    .bind = clock_bind,
    .start = clock_start,
    .process_bind_request = clock_process_bind_request,
    .process_notification = clock_process_notification,
};

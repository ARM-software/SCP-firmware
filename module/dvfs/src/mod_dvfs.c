/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <mod_clock.h>
#include <mod_dvfs.h>
#include <mod_power_domain.h>
#include <mod_psu.h>

/* Domain context */
struct mod_dvfs_domain_ctx {
    /* Domain configuration */
    const struct mod_dvfs_domain_config *config;

    struct {
        /* Power supply API */
        const struct mod_psu_device_api *psu;

        /* Clock API */
        const struct mod_clock_api *clock;
    } apis;

    /* Number of operating points */
    size_t opp_count;

    /* Operating point prior to domain suspension */
    struct mod_dvfs_opp suspended_opp;

    /* Current operating point limits */
    struct mod_dvfs_frequency_limits frequency_limits;
};

static struct mod_dvfs_domain_ctx (*domain_ctx)[];

static struct mod_dvfs_domain_ctx *get_domain_ctx(fwk_id_t domain_id)
{
    return &(*domain_ctx)[fwk_id_get_element_idx(domain_id)];
}

struct mod_dvfs_domain_ctx *__mod_dvfs_get_valid_domain_ctx(fwk_id_t domain_id)
{
    return get_domain_ctx(domain_id);
}


/*
 * DVFS Helper Functions
 */
static int
count_opps(const struct mod_dvfs_opp *opps)
{
    const struct mod_dvfs_opp *opp = &opps[0];

    while ((opp->voltage != 0) && (opp->frequency != 0))
        opp++;

    return opp - &opps[0];
}

static const struct mod_dvfs_opp *get_opp_for_values(
    const struct mod_dvfs_domain_ctx *ctx,
    uint64_t frequency,
    uint64_t voltage)
{
    size_t opp_idx;
    const struct mod_dvfs_opp *opp;

    /* A value of zero indicates the parameter should be ignored */
    assert((frequency != 0) || (voltage != 0));

    for (opp_idx = 0; opp_idx < ctx->opp_count; opp_idx++) {
        opp = &ctx->config->opps[opp_idx];

        /* Only check the frequency if requested */
        if ((frequency != 0) && (opp->frequency != frequency))
            continue;

        /* Only check the voltage if requested */
        if ((voltage != 0) && (opp->voltage != voltage))
            continue;

        return opp;
    }

    return NULL;
}

static bool
is_opp_within_limits(const struct mod_dvfs_opp *opp,
    const struct mod_dvfs_frequency_limits *limits)
{
    return (opp->frequency >= limits->minimum) &&
           (opp->frequency <= limits->maximum);
}

static bool
are_limits_valid(const struct mod_dvfs_domain_ctx *ctx,
    const struct mod_dvfs_frequency_limits *limits)
{
    if (limits->minimum > limits->maximum)
        return false;

    if (get_opp_for_values(ctx, limits->minimum, 0) == NULL)
        return false;

    if (get_opp_for_values(ctx, limits->maximum, 0) == NULL)
        return false;

    return true;
}

static const struct
mod_dvfs_opp *adjust_opp_for_new_limits(
    const struct mod_dvfs_domain_ctx *ctx,
    const struct mod_dvfs_opp *opp,
    const struct mod_dvfs_frequency_limits *limits)
{
    uint64_t needle;

    if (opp->frequency < limits->minimum)
        needle = limits->minimum;
    else if (opp->frequency > limits->maximum)
        needle = limits->maximum;
    else {
        /* No transition necessary */
        return opp;
    }

    return get_opp_for_values(ctx, needle, 0);
}

/*
 * DVFS Utilities
 */
static int
__mod_dvfs_get_current_opp(const struct mod_dvfs_domain_ctx *ctx,
    struct mod_dvfs_opp *opp)
{
    int status;

    status = ctx->apis.clock->get_rate(
        ctx->config->clock_id,
        &opp->frequency);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    status = ctx->apis.psu->get_voltage(
        ctx->config->psu_id,
        &opp->voltage);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}

static int
__mod_dvfs_set_opp(const struct mod_dvfs_domain_ctx *ctx,
    const struct mod_dvfs_opp *new_opp)
{
    int status;
    struct mod_dvfs_opp current_opp;

    status = __mod_dvfs_get_current_opp(ctx, &current_opp);
    if (status != FWK_SUCCESS)
        return status;

    if (new_opp->voltage > current_opp.voltage) {
        /* Raise the voltage before raising the frequency */
        status = ctx->apis.psu->set_voltage(
            ctx->config->psu_id,
            new_opp->voltage);
        if (status != FWK_SUCCESS)
            return FWK_E_DEVICE;
    }

    if (new_opp->frequency != current_opp.frequency) {
        status = ctx->apis.clock->set_rate(
            ctx->config->clock_id,
            new_opp->frequency,
            MOD_CLOCK_ROUND_MODE_NONE);
        if (status != FWK_SUCCESS)
            return FWK_E_DEVICE;
    }

    if (new_opp->voltage < current_opp.voltage) {
        /* Lower the voltage after lowering the frequency */
        status = ctx->apis.psu->set_voltage(
            ctx->config->psu_id,
            new_opp->voltage);
        if (status != FWK_SUCCESS)
            return FWK_E_DEVICE;
    }

    return FWK_SUCCESS;
}
/*
 * DVFS Domain API Support
 */
static int
api_get_current_opp(fwk_id_t domain_id, struct mod_dvfs_opp *opp)
{
    int status;
    const struct mod_dvfs_domain_ctx *ctx;

    fwk_assert(opp != NULL);

    ctx = __mod_dvfs_get_valid_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    status = __mod_dvfs_get_current_opp(ctx, opp);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

static int
api_get_sustained_opp(fwk_id_t domain_id, struct mod_dvfs_opp *opp)
{
    const struct mod_dvfs_domain_ctx *ctx;

    fwk_assert(opp != NULL);

    ctx = __mod_dvfs_get_valid_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    if (ctx->config->sustained_idx  >= ctx->opp_count)
        return FWK_E_PARAM;

    *opp = ctx->config->opps[ctx->config->sustained_idx];

    return FWK_SUCCESS;
}

static int
api_get_nth_opp(fwk_id_t domain_id,
    size_t n,
    struct mod_dvfs_opp *opp)
{
    const struct mod_dvfs_domain_ctx *ctx;

    fwk_assert(opp != NULL);

    ctx = __mod_dvfs_get_valid_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    if (n >= ctx->opp_count)
        return FWK_E_PARAM;

    *opp = ctx->config->opps[n];

    return FWK_SUCCESS;
}

static int
api_get_opp_count(fwk_id_t domain_id, size_t *opp_count)
{
    const struct mod_dvfs_domain_ctx *ctx;

    fwk_assert(opp_count != NULL);

    ctx = __mod_dvfs_get_valid_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *opp_count = ctx->opp_count;

    return FWK_SUCCESS;
}

static int
api_get_latency(fwk_id_t domain_id, uint16_t *latency)
{
    const struct mod_dvfs_domain_ctx *ctx;

    fwk_assert(latency != NULL);

    ctx = __mod_dvfs_get_valid_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *latency = ctx->config->latency;

    return FWK_SUCCESS;
}

static int
api_set_frequency(fwk_id_t domain_id, uint64_t frequency)
{
    int status;
    const struct mod_dvfs_domain_ctx *ctx;
    const struct mod_dvfs_opp *new_opp;

    ctx = __mod_dvfs_get_valid_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    /* Only accept frequencies that exist in the operating point table */
    new_opp = get_opp_for_values(ctx, frequency, 0);
    if (new_opp == NULL)
        return FWK_E_RANGE;

    if (!is_opp_within_limits(new_opp, &ctx->frequency_limits))
        return FWK_E_RANGE;

    status = __mod_dvfs_set_opp(ctx, new_opp);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

static int
api_get_frequency_limits(fwk_id_t domain_id,
    struct mod_dvfs_frequency_limits *limits)
{
    const struct mod_dvfs_domain_ctx *ctx;

    ctx = __mod_dvfs_get_valid_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *limits = ctx->frequency_limits;

    return FWK_SUCCESS;
}

static int
api_set_frequency_limits(fwk_id_t domain_id,
    const struct mod_dvfs_frequency_limits *limits)
{
    int status;
    struct mod_dvfs_domain_ctx *ctx;
    struct mod_dvfs_opp current_opp;
    const struct mod_dvfs_opp *new_opp;

    ctx = __mod_dvfs_get_valid_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    if (!are_limits_valid(ctx, limits))
        return FWK_E_PARAM;

    status = __mod_dvfs_get_current_opp(ctx, &current_opp);
    if (status != FWK_SUCCESS)
        return status;

    new_opp = adjust_opp_for_new_limits(ctx, &current_opp, limits);
    status = __mod_dvfs_set_opp(ctx, new_opp);
    if (status != FWK_SUCCESS)
        return status;

    ctx->frequency_limits = *limits;

    return FWK_SUCCESS;
}

const struct mod_dvfs_domain_api __mod_dvfs_domain_api = {
    .get_current_opp = api_get_current_opp,
    .get_sustained_opp = api_get_sustained_opp,
    .get_nth_opp = api_get_nth_opp,
    .get_opp_count = api_get_opp_count,
    .get_latency = api_get_latency,
    .set_frequency = api_set_frequency,
    .get_frequency_limits = api_get_frequency_limits,
    .set_frequency_limits = api_set_frequency_limits,
};
/*
 * DVFS Event Handling
 */
static int
event_set_frequency(const struct fwk_event *event,
    struct fwk_event *response)
{
    return FWK_E_SUPPORT;
}

static int
event_set_frequency_limits(const struct fwk_event *event,
    struct fwk_event *response)
{
    return FWK_E_SUPPORT;
}

static int
__mod_dvfs_process_event(const struct fwk_event *event,
    struct fwk_event *response)
{
    typedef int (*handler_t)(
        const struct fwk_event *event,
        struct fwk_event *response);

    static const handler_t handlers[] = {
        [MOD_DVFS_EVENT_IDX_SET_FREQUENCY] = event_set_frequency,
        [MOD_DVFS_EVENT_IDX_SET_FREQUENCY_LIMITS] = event_set_frequency_limits,
    };

    handler_t handler;

    /* Ensure we have a handler implemented for this event */
    handler = handlers[fwk_id_get_event_idx(event->id)];
    if (handler == NULL)
        return FWK_E_PARAM;

    /* Delegate event handling to the relevant handler */
    return handler(event, response);
}

/*
 * DVFS Module Framework Support
 */
static int
dvfs_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    domain_ctx = fwk_mm_calloc(element_count, sizeof((*domain_ctx)[0]));
    if (domain_ctx == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int
dvfs_element_init(fwk_id_t domain_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_dvfs_domain_ctx *ctx = get_domain_ctx(domain_id);

    fwk_assert(sub_element_count == 0);

    /* Initialize the configuration */
    ctx->config = data;
    fwk_assert(ctx->config->opps != NULL);

    /* Initialize the context */
    ctx->opp_count = count_opps(ctx->config->opps);
    fwk_assert(ctx->opp_count > 0);

    /* Frequency limits default to the minimum and maximum available */
    ctx->frequency_limits = (struct mod_dvfs_frequency_limits) {
        .minimum = ctx->config->opps[0].frequency,
        .maximum = ctx->config->opps[ctx->opp_count - 1].frequency,
    };

    ctx->suspended_opp = ctx->config->opps[ctx->config->sustained_idx];

    return FWK_SUCCESS;
}

static int
dvfs_bind_element(fwk_id_t domain_id, unsigned int round)
{
    int status;
    const struct mod_dvfs_domain_ctx *ctx = get_domain_ctx(domain_id);

    /* Only handle the first round */
    if (round > 0)
        return FWK_SUCCESS;

    /* Bind to the power supply module */
    status = fwk_module_bind(
        ctx->config->psu_id, mod_psu_api_id_device, &ctx->apis.psu);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    /* Bind to the clock module */
    status = fwk_module_bind(
        ctx->config->clock_id,
        FWK_ID_API(FWK_MODULE_IDX_CLOCK, 0),
        &ctx->apis.clock);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    return FWK_SUCCESS;
}

static int
dvfs_bind(fwk_id_t id, unsigned int round)
{
    /* We only need to handle binding our elements */
    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return dvfs_bind_element(id, round);

    return FWK_SUCCESS;
}

static int
dvfs_process_bind_request_module(fwk_id_t source_id,
    fwk_id_t api_id,
    const void **api)
{
    /* Only expose the module API */
    if (!fwk_id_is_equal(api_id, mod_dvfs_api_id_dvfs))
        return FWK_E_PARAM;

    /* We don't do any permissions management */
    *api = &__mod_dvfs_domain_api;

    return FWK_SUCCESS;
}

static int
dvfs_process_bind_request(fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    /* Only allow binding to the module */
    if (!fwk_id_is_equal(target_id, fwk_module_id_dvfs))
        return FWK_E_PARAM;

    return dvfs_process_bind_request_module(source_id, api_id, api);
}

static int
dvfs_start(fwk_id_t id)
{
    int status;
    const struct mod_dvfs_domain_ctx *ctx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return FWK_SUCCESS;

    ctx = get_domain_ctx(id);

    /* Register for clock state notifications */
    status = fwk_notification_subscribe(
        mod_clock_notification_id_state_changed,
        ctx->config->clock_id,
        id);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_notification_subscribe(
        mod_clock_notification_id_state_change_pending,
        ctx->config->clock_id,
        id);
}

static int
dvfs_notify_system_state_transition_suspend(fwk_id_t domain_id)
{
    struct mod_dvfs_domain_ctx *ctx =
        get_domain_ctx(domain_id);

    return __mod_dvfs_get_current_opp(ctx, &ctx->suspended_opp);
}

static int
dvfs_notify_system_state_transition_resume(fwk_id_t domain_id)
{
    const struct mod_dvfs_domain_ctx *ctx =
        get_domain_ctx(domain_id);

    return __mod_dvfs_set_opp(ctx, &ctx->suspended_opp);
}

static int
dvfs_process_notification(const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params;
    struct clock_state_change_pending_resp_params *resp_params;

    fwk_assert(
        fwk_id_is_equal(
            event->id,
            mod_clock_notification_id_state_changed) ||
        fwk_id_is_equal(
            event->id,
            mod_clock_notification_id_state_change_pending));
    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    params = (struct clock_notification_params *)event->params;

    if (fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed)) {
        if (params->new_state == MOD_CLOCK_STATE_RUNNING)
            return dvfs_notify_system_state_transition_resume(event->target_id);
    } else if (params->new_state == MOD_CLOCK_STATE_STOPPED) {
        /* DVFS has received the pending change notification */
        resp_params =
            (struct clock_state_change_pending_resp_params *)resp_event->params;
        resp_params->status = FWK_SUCCESS;

        return dvfs_notify_system_state_transition_suspend(event->target_id);
    }

    return FWK_SUCCESS;
}
/* Module description */
const struct fwk_module module_dvfs = {
    .name = "DVFS",
    .type = FWK_MODULE_TYPE_HAL,
    .init = dvfs_init,
    .element_init = dvfs_element_init,
    .bind = dvfs_bind,
    .process_bind_request = dvfs_process_bind_request,
    .process_event = __mod_dvfs_process_event,
    .start = dvfs_start,
    .process_notification = dvfs_process_notification,
    .api_count = MOD_DVFS_API_IDX_COUNT,
    .event_count = MOD_DVFS_EVENT_IDX_COUNT,
};

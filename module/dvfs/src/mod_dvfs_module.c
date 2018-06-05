/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <mod_clock.h>
#include <mod_dvfs_private.h>
#include <mod_power_domain.h>
#include <mod_psu.h>

static struct mod_dvfs_domain_ctx (*domain_ctx)[];

static int count_opps(const struct mod_dvfs_opp *opps)
{
    const struct mod_dvfs_opp *opp = &opps[0];

    while ((opp->voltage != 0) && (opp->frequency != 0))
        opp++;

    return opp - &opps[0];
}

static struct mod_dvfs_domain_ctx *get_domain_ctx(fwk_id_t domain_id)
{
    unsigned int element_idx = fwk_id_get_element_idx(domain_id);

    return &(*domain_ctx)[element_idx];
}

static int dvfs_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    domain_ctx = fwk_mm_calloc(
        element_count,
        sizeof((*domain_ctx)[0]));
    if (domain_ctx == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int dvfs_element_init(
    fwk_id_t domain_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_dvfs_domain_ctx *ctx = get_domain_ctx(domain_id);

    assert(sub_element_count == 0);

    /* Initialize the configuration */
    ctx->config = data;
    assert(ctx->config->opps != NULL);

    /* Initialize the context */
    ctx->opp_count = count_opps(ctx->config->opps);
    assert(ctx->opp_count > 0);

    /* Frequency limits default to the minimum and maximum available */
    ctx->frequency_limits = (struct mod_dvfs_frequency_limits) {
        .minimum = ctx->config->opps[0].frequency,
        .maximum = ctx->config->opps[ctx->opp_count - 1].frequency,
    };

    ctx->suspended_opp = ctx->config->opps[ctx->config->sustained_idx];

    return FWK_SUCCESS;
}

static int dvfs_bind_element(fwk_id_t domain_id, unsigned int round)
{
    int status;
    const struct mod_dvfs_domain_ctx *ctx =
        get_domain_ctx(domain_id);

    /* Only handle the first round */
    if (round > 0)
        return FWK_SUCCESS;

    /* Bind to the power supply module */
    status = fwk_module_bind(
        ctx->config->psu_id,
        mod_psu_api_id_psu_device,
        &ctx->apis.psu);
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

static int dvfs_bind(fwk_id_t id, unsigned int round)
{
    /* We only need to handle binding our elements */
    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return dvfs_bind_element(id, round);

    return FWK_SUCCESS;
}

static int dvfs_process_bind_request_module(
    fwk_id_t source_id,
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

static int dvfs_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    /* Only allow binding to the module */
    if (!fwk_id_is_equal(target_id, fwk_module_id_dvfs))
        return FWK_E_PARAM;

    return dvfs_process_bind_request_module(source_id, api_id, api);
}

static int dvfs_start(fwk_id_t id)
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

static int dvfs_notify_system_state_transition_suspend(fwk_id_t domain_id)
{
    struct mod_dvfs_domain_ctx *ctx =
        get_domain_ctx(domain_id);

    return __mod_dvfs_get_current_opp(ctx, &ctx->suspended_opp);
}

static int dvfs_notify_system_state_transition_resume(fwk_id_t domain_id)
{
    const struct mod_dvfs_domain_ctx *ctx =
        get_domain_ctx(domain_id);

    return __mod_dvfs_set_opp(ctx, &ctx->suspended_opp);
}

static int dvfs_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params;
    struct clock_state_change_pending_resp_params *resp_params;

    assert(
        fwk_id_is_equal(
            event->id,
            mod_clock_notification_id_state_changed) ||
        fwk_id_is_equal(
            event->id,
            mod_clock_notification_id_state_change_pending));
    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

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

struct mod_dvfs_domain_ctx *__mod_dvfs_get_valid_domain_ctx(fwk_id_t domain_id)
{
    if (fwk_module_check_call(domain_id) != FWK_SUCCESS)
        return NULL;

    return get_domain_ctx(domain_id);
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

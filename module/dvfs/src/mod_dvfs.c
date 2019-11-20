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
#ifdef BUILD_HAS_MULTITHREADING
#include <fwk_multi_thread.h>
#endif
#include <fwk_notification.h>
#include <mod_clock.h>
#include <mod_dvfs.h>
#include <mod_power_domain.h>
#include <mod_psu.h>

/*!
 * \brief Domain states for GET_OPP/SET_OPP.
 */
enum mod_dvfs_domain_state {
    DVFS_DOMAIN_STATE_IDLE = 0,

    /* Waiting for GET_OPP request */
    DVFS_DOMAIN_GET_OPP,

    /* Waiting for SET_OPP request */
    DVFS_DOMAIN_SET_OPP,

    /* set_rate() in progress, set_voltage() next */
    DVFS_DOMAIN_SET_VOLTAGE,

    /* set_voltage() in progress, set_frequency() next */
    DVFS_DOMAIN_SET_FREQUENCY,

    /* waiting for SET_OPP to complete */
    DVFS_DOMAIN_SET_OPP_DONE,
};

/*!
 * \brief Request for SET_OPP.
 */
struct mod_dvfs_request {
    /* New operating point data for the request */
    struct mod_dvfs_opp new_opp;

    /* New operating point limits for the active request */
    struct mod_dvfs_frequency_limits frequency_limits;
};

/*!
 * \brief Domain context.
 */
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

    /* Current operating point */
    struct mod_dvfs_opp current_opp;

    /* Current frequency limits */
    struct mod_dvfs_frequency_limits frequency_limits;

    /* Current request details */
    struct mod_dvfs_request request;

    /* next state */
    enum mod_dvfs_domain_state state;

    /* cookie for deferred request response */
    uint32_t cookie;
};

static uint32_t dvfs_domain_element_count = 0;
static struct mod_dvfs_domain_ctx (*domain_ctx)[];

/*
 * DVFS Helper Functions
 */
static struct mod_dvfs_domain_ctx *get_domain_ctx(fwk_id_t domain_id)
{
    uint32_t idx = fwk_id_get_element_idx(domain_id);

    if (idx < dvfs_domain_element_count)
        return &(*domain_ctx)[idx];
    else
        return NULL;
}

static int count_opps(const struct mod_dvfs_opp *opps)
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
    fwk_assert((frequency != 0) || (voltage != 0));

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

static bool is_opp_within_limits(const struct mod_dvfs_opp *opp,
    const struct mod_dvfs_frequency_limits *limits)
{
    return (opp->frequency >= limits->minimum) &&
           (opp->frequency <= limits->maximum);
}

static bool are_limits_valid(const struct mod_dvfs_domain_ctx *ctx,
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

static const struct mod_dvfs_opp *adjust_opp_for_new_limits(
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
 * Helper to create events to process requests asynchronously
 */
static int put_event_request(fwk_id_t domain_id,
    struct mod_dvfs_domain_ctx *ctx,
    fwk_id_t event_id,
    enum mod_dvfs_domain_state state,
    bool response_required)
{
    struct fwk_event req;

    ctx->state = state;

    req = (struct fwk_event) {
        .target_id = domain_id,
        .id = event_id,
        .response_requested = response_required,
    };

    return fwk_thread_put_event(&req);
}

/*
 * DVFS module synchronous API functions
 */
static int dvfs_get_sustained_opp(fwk_id_t domain_id, struct mod_dvfs_opp *opp)
{
    const struct mod_dvfs_domain_ctx *ctx;

    fwk_assert(opp != NULL);

    ctx = get_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    if (ctx->config->sustained_idx  >= ctx->opp_count)
        return FWK_E_PARAM;

    *opp = ctx->config->opps[ctx->config->sustained_idx];

    return FWK_SUCCESS;
}

static int dvfs_get_nth_opp(fwk_id_t domain_id,
    size_t n,
    struct mod_dvfs_opp *opp)
{
    const struct mod_dvfs_domain_ctx *ctx;

    fwk_assert(opp != NULL);

    ctx = get_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    if (n >= ctx->opp_count)
        return FWK_E_PARAM;

    *opp = ctx->config->opps[n];

    return FWK_SUCCESS;
}

static int dvfs_get_opp_count(fwk_id_t domain_id, size_t *opp_count)
{
    const struct mod_dvfs_domain_ctx *ctx;

    fwk_assert(opp_count != NULL);

    ctx = get_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *opp_count = ctx->opp_count;

    return FWK_SUCCESS;
}

static int dvfs_get_latency(fwk_id_t domain_id, uint16_t *latency)
{
    const struct mod_dvfs_domain_ctx *ctx;

    fwk_assert(latency != NULL);

    ctx = get_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *latency = ctx->config->latency;

    return FWK_SUCCESS;
}

static int dvfs_get_frequency_limits(fwk_id_t domain_id,
    struct mod_dvfs_frequency_limits *limits)
{
    struct mod_dvfs_domain_ctx *ctx;

    ctx = get_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *limits = ctx->frequency_limits;
    return FWK_SUCCESS;
}

/*
 * dvfs_get_current_opp() may be either synchronous or asynchronous
 */
static int dvfs_get_current_opp(fwk_id_t domain_id, struct mod_dvfs_opp *opp)
{
    int status;
    struct mod_dvfs_domain_ctx *ctx;

    fwk_assert(opp != NULL);

    ctx = get_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    if (ctx->current_opp.frequency != 0) {
        opp->frequency = ctx->current_opp.frequency;
        opp->voltage = ctx->current_opp.voltage;
        return FWK_SUCCESS;
    }

    if (ctx->state != DVFS_DOMAIN_STATE_IDLE)
        return FWK_E_BUSY;

    status = put_event_request(domain_id, ctx,
        mod_dvfs_event_id_get_opp, DVFS_DOMAIN_GET_OPP, true);
    if (status == FWK_SUCCESS) {
         /*
          * We return FWK_PENDING here to indicate to the caller that the
          * result of the request is pending and will arrive later through
          * an event.
          */
        return FWK_PENDING;
    }

    return status;
}

/*
 * DVFS module asynchronous API functions
 */
static int dvfs_set_frequency(fwk_id_t domain_id, uint64_t frequency)
{
    struct mod_dvfs_domain_ctx *ctx;
    const struct mod_dvfs_opp *new_opp;

    ctx = get_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    if (ctx->state != DVFS_DOMAIN_STATE_IDLE)
        return FWK_E_BUSY;

    /* Only accept frequencies that exist in the operating point table */
    new_opp = get_opp_for_values(ctx, frequency, 0);
    if (new_opp == NULL)
        return FWK_E_RANGE;

    if ((new_opp->frequency == ctx->current_opp.frequency) &&
        (new_opp->voltage == ctx->current_opp.voltage))
        return FWK_SUCCESS;

    if (!is_opp_within_limits(new_opp, &ctx->frequency_limits))
        return FWK_E_RANGE;

    ctx->request.new_opp.frequency = new_opp->frequency;
    ctx->request.new_opp.voltage = new_opp->voltage;
    ctx->request.frequency_limits.minimum = 0;
    ctx->request.frequency_limits.maximum = 0;

    /*
     * Note that we do not return FWK_PENDING here. If an event
     * is successfully queued we return FWK_SUCCESS to indicate
     * that the caller should not expect a deferred response.
     */
    return put_event_request(domain_id, ctx, mod_dvfs_event_id_set,
        DVFS_DOMAIN_SET_OPP, false);
}

static int dvfs_set_frequency_limits(fwk_id_t domain_id,
    const struct mod_dvfs_frequency_limits *limits)
{
    struct mod_dvfs_domain_ctx *ctx;
    const struct mod_dvfs_opp *new_opp;

    ctx = get_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    if (ctx->state != DVFS_DOMAIN_STATE_IDLE)
        return FWK_E_BUSY;

    if (!are_limits_valid(ctx, limits))
        return FWK_E_PARAM;

    new_opp = adjust_opp_for_new_limits(ctx, &ctx->current_opp, limits);

    if ((new_opp->frequency == ctx->current_opp.frequency) &&
        (new_opp->voltage == ctx->current_opp.voltage)) {
        ctx->frequency_limits = *limits;
        return FWK_SUCCESS;
    }

    ctx->request.new_opp.frequency = new_opp->frequency;
    ctx->request.new_opp.voltage = new_opp->voltage;
    ctx->request.frequency_limits.minimum = limits->minimum;
    ctx->request.frequency_limits.maximum = limits->maximum;

    /*
     * Note that we do not return FWK_PENDING here. If an event
     * is successfully queued we return FWK_SUCCESS to indicate
     * that the caller should not expect a deferred response.
     */
    return put_event_request(domain_id, ctx, mod_dvfs_event_id_set,
        DVFS_DOMAIN_SET_OPP, false);
}

static const struct mod_dvfs_domain_api mod_dvfs_domain_api = {
    .get_current_opp = dvfs_get_current_opp,
    .get_sustained_opp = dvfs_get_sustained_opp,
    .get_nth_opp = dvfs_get_nth_opp,
    .get_opp_count = dvfs_get_opp_count,
    .get_latency = dvfs_get_latency,
    .set_frequency = dvfs_set_frequency,
    .get_frequency_limits = dvfs_get_frequency_limits,
    .set_frequency_limits = dvfs_set_frequency_limits,
};

/*
 * DVFS utility functions
 */
static void dvfs_set_op_done(struct mod_dvfs_domain_ctx *ctx)
{
    /*
     * SET_OPP() successful, store the new values
     */
    ctx->current_opp.voltage = ctx->request.new_opp.voltage;
    ctx->current_opp.frequency = ctx->request.new_opp.frequency;

    if ((ctx->request.frequency_limits.minimum != 0) ||
        (ctx->request.frequency_limits.maximum != 0)) {
        ctx->frequency_limits = ctx->request.frequency_limits;
    }
}

static int dvfs_complete(struct mod_dvfs_domain_ctx *ctx,
    const struct fwk_event *event,
    struct fwk_event *resp_event,
    int req_status,
    bool return_opp)
{
    int status = req_status;
    struct fwk_event read_req_event;
    struct mod_dvfs_params_response *resp_params;

    if (ctx->cookie != 0) {
        resp_params = (struct mod_dvfs_params_response *)
            &read_req_event.params;
        status = fwk_thread_get_delayed_response(event->target_id,
                                                 ctx->cookie,
                                                 &read_req_event);

        if (status == FWK_SUCCESS) {
            resp_params->status = req_status;
            if (return_opp)
                resp_params->performance_level = ctx->current_opp.frequency;
            status = fwk_thread_put_event(&read_req_event);
        }
        ctx->cookie = 0;
    } else if (resp_event != NULL) {
        resp_params =
            (struct mod_dvfs_params_response *)resp_event->params;
        resp_params->status = req_status;
        if (return_opp)
            resp_params->performance_level = ctx->current_opp.frequency;
    }

    ctx->state = DVFS_DOMAIN_STATE_IDLE;

    return status;
}

/*
 * The SET_OPP() request has successfully completed the first step,
 * reading the voltage.
 */
static int dvfs_handle_set_opp(struct mod_dvfs_domain_ctx *ctx,
    const struct fwk_event *event,
    uint64_t voltage)
{
    int status = FWK_SUCCESS;

    if (ctx->request.new_opp.voltage > voltage) {
        /*
         * Current < request, increase voltage then set frequency
         */
        status = ctx->apis.psu->set_voltage(
            ctx->config->psu_id, ctx->request.new_opp.voltage);

        if (status == FWK_PENDING) {
            ctx->state = DVFS_DOMAIN_SET_FREQUENCY;
            return status;
        }

        if (status != FWK_SUCCESS)
            return dvfs_complete(ctx, event, NULL, status, false);

        /*
         * Voltage set successsfully, continue to set the frequency
         */
        status = ctx->apis.clock->set_rate(
            ctx->config->clock_id,
            ctx->request.new_opp.frequency,
            MOD_CLOCK_ROUND_MODE_NONE);

        if (status == FWK_PENDING) {
            ctx->state = DVFS_DOMAIN_SET_OPP_DONE;
            return status;
        }
    }

    if (ctx->request.new_opp.voltage < voltage) {
        /*
         * Current > request, decrease frequency then set voltage
         */
        status = ctx->apis.clock->set_rate(
            ctx->config->clock_id,
            ctx->request.new_opp.frequency,
            MOD_CLOCK_ROUND_MODE_NONE);

        if (status == FWK_PENDING) {
            ctx->state = DVFS_DOMAIN_SET_VOLTAGE;
            return status;
        }

        if (status != FWK_SUCCESS)
            return dvfs_complete(ctx, event, NULL, status, false);

        /*
         * Clock set_rate() completed successfully, continue to set_voltage()
         */
        status = ctx->apis.psu->set_voltage(
            ctx->config->psu_id, ctx->request.new_opp.voltage);

        if (status == FWK_PENDING) {
            ctx->state = DVFS_DOMAIN_SET_OPP_DONE;
            return status;
        }
    }

    /*
     * SET_OPP() completed, return to caller.
     */
    if (status == FWK_SUCCESS)
        dvfs_set_op_done(ctx);

    return dvfs_complete(ctx, event, NULL, status, false);
}

/*
 * The current voltage has been read. This is the first step of a SET_OPP()
 * request and the only step of a GET_OPP() request. It may have been handled
 * synchronously or asynchronously. Note that resp_event will only be set
 * by a GET_OPP(), it will always be NULL for SET_OPP().
 */
static int dvfs_handle_psu_get_voltage_resp(struct mod_dvfs_domain_ctx *ctx,
    const struct fwk_event *event,
    struct fwk_event *resp_event,
    int req_status,
    uint64_t voltage)
{
    const struct mod_dvfs_opp *opp;

    if (req_status != FWK_SUCCESS)
        return dvfs_complete(ctx, event, resp_event, req_status, false);

    if (ctx->state == DVFS_DOMAIN_SET_OPP)
        return dvfs_handle_set_opp(ctx, event, voltage);

    /*
     * We have the actual voltage, get the frequency from the
     * corresponding OPP in the domain context table.
     */
    opp = get_opp_for_values(ctx, 0, voltage);
    if (opp == NULL)
        return dvfs_complete(ctx, event, resp_event, FWK_E_DEVICE, false);

    /*
     * We have successfully found the frequency, save it in the domain context.
     */
    ctx->current_opp.voltage = voltage;
    ctx->current_opp.frequency = opp->frequency;

    /*
     * This is a GET_OPP(), we are done, return the frequency to caller
     */
    return dvfs_complete(ctx, event, resp_event, FWK_SUCCESS, true);
}

/*
 * Note that dvfs_handle_psu_set_voltage_resp() is only called after an
 * asynchronous set_voltage() operation, we have already saved the
 * cookie for the event.
 */
static int dvfs_handle_psu_set_voltage_resp(struct mod_dvfs_domain_ctx *ctx,
    const struct fwk_event *event)
{
    int status = FWK_SUCCESS;
    struct mod_psu_driver_response *psu_response =
        (struct mod_psu_driver_response *)event->params;

    if (psu_response->status != FWK_SUCCESS)
        return dvfs_complete(ctx, event, NULL, psu_response->status, false);

    if (ctx->state == DVFS_DOMAIN_SET_FREQUENCY) {
        status = ctx->apis.clock->set_rate(
           ctx->config->clock_id,
           ctx->request.new_opp.frequency,
           MOD_CLOCK_ROUND_MODE_NONE);
        if (status == FWK_PENDING) {
            ctx->state = DVFS_DOMAIN_SET_OPP_DONE;
            return status;
        }
    } else if (ctx->state == DVFS_DOMAIN_SET_OPP_DONE) {
        status = FWK_SUCCESS;
    } else
        status = FWK_E_DEVICE;

    /*
     * SET_OPP() completed, return to caller.
     */
    if (status == FWK_SUCCESS)
        dvfs_set_op_done(ctx);

    return dvfs_complete(ctx, event, NULL, status, false);
}

/*
 * Note that dvfs_handle_clk_set_freq_resp() is only called after an
 * asynchronous set_rate() operation, we have already saved the
 * cookie for the event.
 */
static int dvfs_handle_clk_set_freq_resp(struct mod_dvfs_domain_ctx *ctx,
    const struct fwk_event *event)
{
    int status;
    struct mod_clock_driver_resp_params *clock_response =
        (struct mod_clock_driver_resp_params *)event->params;

    if (clock_response->status != FWK_SUCCESS)
        return dvfs_complete(ctx, event, NULL, clock_response->status, false);

    if (ctx->state == DVFS_DOMAIN_SET_VOLTAGE) {
        /*
         * Clock set_rate() completed successfully, continue to set_voltage()
         */
        status = ctx->apis.psu->set_voltage(
            ctx->config->psu_id,
            ctx->request.new_opp.voltage);
        if (status == FWK_PENDING) {
            ctx->state = DVFS_DOMAIN_SET_OPP_DONE;
            return status;
        }

    } else if (ctx->state == DVFS_DOMAIN_SET_OPP_DONE) {
        status = FWK_SUCCESS;
    } else
        status = FWK_E_DEVICE;

    /*
     * SET_OPP() completed, return to caller.
     */
    if (status == FWK_SUCCESS)
        dvfs_set_op_done(ctx);

    return dvfs_complete(ctx, event, NULL, status, false);
}

/*
 * DVFS Module Framework Support
 */
static int mod_dvfs_process_event(const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status = FWK_SUCCESS;
    struct mod_dvfs_domain_ctx *ctx;
    struct mod_psu_driver_response *psu_response;
    uint64_t voltage;

    ctx = get_domain_ctx(event->target_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    /*
     * local DVFS event from dvfs_get_current_opp()
     */
    if (fwk_id_is_equal(event->id, mod_dvfs_event_id_get_opp)) {
        status = ctx->apis.psu->get_voltage(
            ctx->config->psu_id,
            &ctx->request.new_opp.voltage);
        if (status == FWK_PENDING) {
            ctx->cookie = event->cookie;
            resp_event->is_delayed_response = true;
            return FWK_SUCCESS;
        }

        /*
         * Handle get_voltage() synchronously
         */
        return dvfs_handle_psu_get_voltage_resp(ctx, event,
            resp_event, status, ctx->request.new_opp.voltage);
    }

    /*
     * local DVFS event from dvfs_set_frequency()
     */
    if (fwk_id_is_equal(event->id, mod_dvfs_event_id_set)) {
        if (ctx->current_opp.voltage != 0) {
            voltage = ctx->current_opp.voltage;
            status = FWK_SUCCESS;
        } else {
            status = ctx->apis.psu->get_voltage(
                ctx->config->psu_id,
                &voltage);
            if (status == FWK_PENDING)
                return FWK_SUCCESS;
        }

        /*
         * Handle get_voltage() synchronously
         */
        return dvfs_handle_psu_get_voltage_resp(ctx, event, NULL,
            status, voltage);
    }

    /*
     * response event from PSU get_voltage()
     */
    if (fwk_id_is_equal(event->id, mod_psu_event_id_get_voltage)) {
        /*
         * Handle get_voltage() asynchronously
         */
        psu_response = (struct mod_psu_driver_response *)event->params;
        return dvfs_handle_psu_get_voltage_resp(ctx, event, NULL,
            psu_response->status, psu_response->voltage);
    }

    /*
     * response event from PSU set_voltage()
     */
    if (fwk_id_is_equal(event->id, mod_psu_event_id_set_voltage)) {
        /*
         * Handle set_voltage() asynchronously
         */
        return dvfs_handle_psu_set_voltage_resp(ctx, event);
    }

    /*
     * response event from Clock set_rate()
     */
    if (fwk_id_is_equal(event->id, mod_clock_event_id_request)) {
        /*
         * Handle set_frequency() asynchronously
         */
        return dvfs_handle_clk_set_freq_resp(ctx, event);
    }

    return FWK_E_PARAM;
}

static int dvfs_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    domain_ctx = fwk_mm_calloc(element_count, sizeof((*domain_ctx)[0]));
    if (domain_ctx == NULL)
        return FWK_E_NOMEM;
    dvfs_domain_element_count = element_count;
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
    *api = &mod_dvfs_domain_api;

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

/* Module description */
const struct fwk_module module_dvfs = {
    .name = "DVFS",
    .type = FWK_MODULE_TYPE_HAL,
    .init = dvfs_init,
    .element_init = dvfs_element_init,
    .bind = dvfs_bind,
    .process_bind_request = dvfs_process_bind_request,
    .process_event = mod_dvfs_process_event,
    .api_count = MOD_DVFS_API_IDX_COUNT,
    .event_count = MOD_DVFS_EVENT_IDX_COUNT,
};

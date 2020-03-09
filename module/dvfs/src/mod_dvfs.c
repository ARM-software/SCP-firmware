/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_clock.h>
#include <mod_dvfs.h>
#include <mod_psu.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>

/*
 * Maximum number of attempts to complete a request
 */
#define DVFS_MAX_RETRIES    4

enum mod_dvfs_internal_event_idx {
    /* retry request */
    MOD_DVFS_INTERNAL_EVENT_IDX_RETRY = MOD_DVFS_EVENT_IDX_COUNT,
    MOD_DVFS_INTERNAL_EVENT_IDX_COUNT,
};

/* Retry request event identifier */
static const fwk_id_t mod_dvfs_event_id_retry =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_DVFS,
        MOD_DVFS_INTERNAL_EVENT_IDX_RETRY);

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

    /* waiting for alarm callback to start a retry */
    DVFS_DOMAIN_STATE_RETRY,
};

/*!
 * \brief Request for SET_OPP.
 */
struct mod_dvfs_request {
    /* New operating point data for the request */
    struct mod_dvfs_opp new_opp;

    /* Response expected for this request */
    bool response_required;

    /* Retry the request on failure */
    bool retry_request;

    /* This request requires the event source_id to be set */
    bool set_source_id;

    /* Retry count */
    uint8_t num_retries;
};

/*!
 * \brief Domain context.
 */
struct mod_dvfs_domain_ctx {
    /* Context Domain ID */
    fwk_id_t domain_id;

    /* Domain configuration */
    const struct mod_dvfs_domain_config *config;

    struct {
        /* Power supply API */
        const struct mod_psu_device_api *psu;

        /* Clock API */
        const struct mod_clock_api *clock;

        /* Alarm API for pending requests */
        const struct mod_timer_alarm_api *alarm_api;

    } apis;

    /* Number of operating points */
    size_t opp_count;

    /* Current operating point */
    struct mod_dvfs_opp current_opp;

    /* Current frequency limits */
    struct mod_dvfs_frequency_limits frequency_limits;

    /* Current request details */
    struct mod_dvfs_request request;

    /* State */
    enum mod_dvfs_domain_state state;

    /* Cookie for deferred request response */
    uint32_t cookie;

    /* Pending request details */
    struct mod_dvfs_request pending_request;

    /* SET_OPP Request is pending for this domain */
    bool request_pending;
};

struct mod_dvfs_ctx {
    /* Number of DVFS domains */
    uint32_t dvfs_domain_element_count;

    /* DVFS device context table */
    struct mod_dvfs_domain_ctx (*domain_ctx)[];
} dvfs_ctx;

/*
 * DVFS Helper Functions
 */
static struct mod_dvfs_domain_ctx *get_domain_ctx(fwk_id_t domain_id)
{
    uint32_t idx = fwk_id_get_element_idx(domain_id);

    if (idx < dvfs_ctx.dvfs_domain_element_count)
        return &(*dvfs_ctx.domain_ctx)[idx];
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

    /*
     * If both parameters are zero we have nothing to do.
     */
    if ((frequency == 0) && (voltage == 0))
        return NULL;

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
static int put_event_request(struct mod_dvfs_domain_ctx *ctx,
    fwk_id_t event_id,
    enum mod_dvfs_domain_state state)
{
    struct fwk_event req;

    req = (struct fwk_event) {
        .target_id = ctx->domain_id,
        .id = event_id,
        .response_requested = ctx->request.response_required,
    };

    if (ctx->request.set_source_id)
        req.source_id = ctx->domain_id;

    ctx->state = state;

    return fwk_thread_put_event(&req);
}

static int dvfs_set_frequency_start(struct mod_dvfs_domain_ctx *ctx,
    const struct mod_dvfs_opp *new_opp,
    bool retry_request,
    uint8_t num_retries)
{
    ctx->request.new_opp = *new_opp;
    ctx->request.retry_request = retry_request;
    ctx->request.response_required = false;
    ctx->request.num_retries = num_retries;

    return put_event_request(ctx, mod_dvfs_event_id_set,
        DVFS_DOMAIN_SET_OPP);
}

/*
 * Handle pending requests
 */
static void dvfs_flush_pending_request(struct mod_dvfs_domain_ctx *ctx)
{
    if (ctx->request_pending) {
        ctx->request_pending = false;
        dvfs_set_frequency_start(ctx, &ctx->pending_request.new_opp,
            ctx->pending_request.retry_request,
            ctx->pending_request.num_retries);
    }
    ctx->pending_request = (struct mod_dvfs_request){ 0 };
}

static void alarm_callback(uintptr_t param)
{
    struct fwk_event req;
    struct mod_dvfs_domain_ctx *ctx =
        (struct mod_dvfs_domain_ctx *)param;

    req = (struct fwk_event) {
        .target_id = ctx->domain_id,
        .source_id = ctx->domain_id,
        .id = mod_dvfs_event_id_retry,
        .response_requested = ctx->pending_request.response_required,
    };

    fwk_thread_put_event(&req);
}

static int dvfs_handle_pending_request(struct mod_dvfs_domain_ctx *ctx)
{
    int status = FWK_SUCCESS;

    if (ctx->state == DVFS_DOMAIN_STATE_RETRY)
        return FWK_SUCCESS;

    if (ctx->config->retry_ms > 0) {
        status = ctx->apis.alarm_api->start(ctx->config->alarm_id,
            ctx->config->retry_ms, MOD_TIMER_ALARM_TYPE_ONCE,
            alarm_callback, (uintptr_t)ctx);
        if (status == FWK_SUCCESS)
            ctx->state = DVFS_DOMAIN_STATE_RETRY;
    } else {
        /*
         * If this domain does not have a timeout configured we start
         * processing the request immediately.
         */
        dvfs_flush_pending_request(ctx);
    }
    return status;
}

static int dvfs_create_pending_level_request(struct mod_dvfs_domain_ctx *ctx,
    const struct mod_dvfs_opp *new_opp, bool retry_request)
{

    if (ctx->request_pending) {
        if ((new_opp->frequency == ctx->pending_request.new_opp.frequency) &&
            (new_opp->voltage == ctx->pending_request.new_opp.voltage))
            return FWK_SUCCESS;
    } else {
        if ((new_opp->frequency == ctx->current_opp.frequency) &&
            (new_opp->voltage == ctx->current_opp.voltage))
                return FWK_SUCCESS;

        ctx->pending_request.num_retries = 0;
        ctx->pending_request.set_source_id = false;
        ctx->pending_request.response_required = false;
        ctx->request_pending = true;
    }

    ctx->pending_request.new_opp = *new_opp;

    /*
     * Set the retry flag only if we have to retry the request. Otherwise don't
     * overwrite it.
     */
    if (retry_request)
        ctx->pending_request.retry_request = retry_request;

    return FWK_SUCCESS;
}

/*
 * DVFS module synchronous API functions
 */
static int dvfs_get_sustained_opp(fwk_id_t domain_id, struct mod_dvfs_opp *opp)
{
    const struct mod_dvfs_domain_ctx *ctx;

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

    if (opp == NULL)
        return FWK_E_PARAM;

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

    if (opp_count == NULL)
        return FWK_E_PARAM;

    ctx = get_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *opp_count = ctx->opp_count;

    return FWK_SUCCESS;
}

static int dvfs_get_latency(fwk_id_t domain_id, uint16_t *latency)
{
    const struct mod_dvfs_domain_ctx *ctx;

    if (latency == NULL)
        return FWK_E_PARAM;

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

    if (limits == NULL)
        return FWK_E_PARAM;

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

    if (opp == NULL)
        return FWK_E_PARAM;

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

    ctx->request.response_required = true;
    status = put_event_request(ctx, mod_dvfs_event_id_get_opp,
        DVFS_DOMAIN_GET_OPP);
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

    /* Only accept frequencies that exist in the operating point table */
    new_opp = get_opp_for_values(ctx, frequency, 0);
    if (new_opp == NULL)
        return FWK_E_RANGE;

    if (!is_opp_within_limits(new_opp, &ctx->frequency_limits))
        return FWK_E_RANGE;

    if (ctx->state != DVFS_DOMAIN_STATE_IDLE)
        return dvfs_create_pending_level_request(ctx, new_opp, false);

    if ((new_opp->frequency == ctx->current_opp.frequency) &&
        (new_opp->voltage == ctx->current_opp.voltage))
        return FWK_SUCCESS;

    return dvfs_set_frequency_start(ctx, new_opp, false, 0);
}

static int dvfs_set_frequency_limits(fwk_id_t domain_id,
    const struct mod_dvfs_frequency_limits *limits)
{
    struct mod_dvfs_domain_ctx *ctx;
    const struct mod_dvfs_opp *new_opp;

    ctx = get_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    if (!are_limits_valid(ctx, limits))
        return FWK_E_PARAM;

    new_opp = adjust_opp_for_new_limits(ctx, &ctx->current_opp, limits);
    if (new_opp == NULL)
        return FWK_E_PARAM;

    ctx->frequency_limits = *limits;

    if ((new_opp->frequency == ctx->current_opp.frequency) &&
        (new_opp->voltage == ctx->current_opp.voltage)) {
        return FWK_SUCCESS;
    }

    if (ctx->state != DVFS_DOMAIN_STATE_IDLE)
        return dvfs_create_pending_level_request(ctx, new_opp, true);

    return dvfs_set_frequency_start(ctx, new_opp, true, 0);
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

/*
 * DVFS Request Complete handling.
 */
static void dvfs_complete_respond(struct mod_dvfs_domain_ctx *ctx,
    struct fwk_event *resp_event,
    int req_status)
{
    int status;
    struct fwk_event read_req_event;
    struct mod_dvfs_params_response *resp_params;
    bool return_opp;

    if ((ctx->state == DVFS_DOMAIN_GET_OPP) && (req_status == FWK_SUCCESS))
        return_opp = true;

    if (ctx->cookie != 0) {
        /*
         * The request was handled asynchronously, retrieve
         * the delayed_response and return it to the caller
         * with the data.
         */
        resp_params = (struct mod_dvfs_params_response *)
            &read_req_event.params;
        status = fwk_thread_get_delayed_response(ctx->domain_id,
                                                 ctx->cookie,
                                                 &read_req_event);

        if (status == FWK_SUCCESS) {
            resp_params->status = req_status;
            if (return_opp)
                resp_params->performance_level = ctx->current_opp.frequency;
            fwk_thread_put_event(&read_req_event);
        }
        ctx->cookie = 0;
    } else if (resp_event != NULL) {
        /*
         * The request is being handled synchronously, return
         * the data to the caller in the resp_event.
         */
        resp_params =
            (struct mod_dvfs_params_response *)resp_event->params;
        resp_params->status = req_status;
        if (return_opp)
            resp_params->performance_level = ctx->current_opp.frequency;
    }
}

static int dvfs_complete(struct mod_dvfs_domain_ctx *ctx,
    struct fwk_event *resp_event,
    int req_status)
{
    int status = req_status;

    if (ctx->request.response_required) {
        /*
         * If the DVFS request requires a response we send it now, no retries
         * are attempted.
         */
        dvfs_complete_respond(ctx, resp_event, req_status);

    } else if ((req_status != FWK_SUCCESS) && ctx->request.retry_request) {
        /*
         * No response required, request has failed, a retry is necessary.
         */
        if (ctx->request.num_retries++ < DVFS_MAX_RETRIES) {
            ctx->pending_request.retry_request = ctx->request.retry_request;
            ctx->pending_request.num_retries = ctx->request.num_retries;
            if (!ctx->request_pending) {
                ctx->pending_request.new_opp = ctx->request.new_opp;
                ctx->request_pending = true;
            }
        }
    }

    /*
     * Now we need to start processing the pending request if any,
     * note that we do not set the state to DOMAIN_STATE_IDLE
     * here to prevent another request being processed ahead of this one.
     */
    if (ctx->request_pending) {
        dvfs_handle_pending_request(ctx);
    } else {
        /*
         * The request has completed and there are no pending requests.
         * Clean up the domain ctx here.
         */
        ctx->pending_request = (struct mod_dvfs_request){ 0 };
        ctx->request = (struct mod_dvfs_request){ 0 };
        ctx->state = DVFS_DOMAIN_STATE_IDLE;
    }

    return status;
}

/*
 * The SET_OPP() request has successfully completed the first step,
 * reading the voltage.
 */
static int dvfs_handle_set_opp(struct mod_dvfs_domain_ctx *ctx,
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
            return dvfs_complete(ctx, NULL, status);

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
    } else if (ctx->request.new_opp.voltage < voltage) {
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
            return dvfs_complete(ctx, NULL, status);

        /*
         * Clock set_rate() completed successfully, continue to set_voltage()
         */
        status = ctx->apis.psu->set_voltage(
            ctx->config->psu_id, ctx->request.new_opp.voltage);

        if (status == FWK_PENDING) {
            ctx->state = DVFS_DOMAIN_SET_OPP_DONE;
            return status;
        }
    } else if (ctx->current_opp.frequency == 0) {
        /*
         * At startup the voltage may be set without the frequency having
         * been set. In this case we must set the frequency regardless of
         * the voltage.
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

    /*
     * SET_OPP() completed, return to caller.
     */
    if (status == FWK_SUCCESS)
        ctx->current_opp = ctx->request.new_opp;

    return dvfs_complete(ctx, NULL, status);
}

/*
 * The current voltage has been read. This is the first step of a SET_OPP()
 * request and the only step of a GET_OPP() request. It may have been handled
 * synchronously or asynchronously. Note that resp_event will only be set
 * by a GET_OPP(), it will always be NULL for SET_OPP().
 */
static int dvfs_handle_psu_get_voltage_resp(struct mod_dvfs_domain_ctx *ctx,
    struct fwk_event *resp_event,
    int req_status,
    uint64_t voltage)
{
    const struct mod_dvfs_opp *opp;

    if (req_status != FWK_SUCCESS)
        return dvfs_complete(ctx, resp_event, req_status);

    if (ctx->state == DVFS_DOMAIN_SET_OPP)
        return dvfs_handle_set_opp(ctx, voltage);

    /*
     * We have the actual voltage, get the frequency from the
     * corresponding OPP in the domain context table.
     */
    opp = get_opp_for_values(ctx, 0, voltage);
    if (opp == NULL)
        return dvfs_complete(ctx, resp_event, FWK_E_DEVICE);

    /*
     * We have successfully found the frequency, save it in the domain context.
     */
    ctx->current_opp.voltage = voltage;
    ctx->current_opp.frequency = opp->frequency;

    /*
     * This is a GET_OPP(), we are done, return the frequency to caller
     */
    return dvfs_complete(ctx, resp_event, FWK_SUCCESS);
}

/*
 * Note that dvfs_handle_psu_set_voltage_resp() is only called after an
 * asynchronous set_voltage() operation.
 */
static int dvfs_handle_psu_set_voltage_resp(struct mod_dvfs_domain_ctx *ctx,
    const struct fwk_event *event)
{
    int status = FWK_SUCCESS;
    struct mod_psu_driver_response *psu_response =
        (struct mod_psu_driver_response *)event->params;

    if (psu_response->status != FWK_SUCCESS)
        return dvfs_complete(ctx, NULL, psu_response->status);

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
        ctx->current_opp = ctx->request.new_opp;

    return dvfs_complete(ctx, NULL, status);
}

/*
 * Note that dvfs_handle_clk_set_freq_resp() is only called after an
 * asynchronous set_rate() operation.
 */
static int dvfs_handle_clk_set_freq_resp(struct mod_dvfs_domain_ctx *ctx,
    const struct fwk_event *event)
{
    int status;
    struct mod_clock_driver_resp_params *clock_response =
        (struct mod_clock_driver_resp_params *)event->params;

    if (clock_response->status != FWK_SUCCESS)
        return dvfs_complete(ctx, NULL, clock_response->status);

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
        ctx->current_opp = ctx->request.new_opp;

    return dvfs_complete(ctx, NULL, status);
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
        status = ctx->apis.psu->get_voltage(ctx->config->psu_id,
            &ctx->request.new_opp.voltage);
        if (status == FWK_PENDING) {
            ctx->cookie = event->cookie;
            resp_event->is_delayed_response = true;
            return FWK_SUCCESS;
        }

        /*
         * Handle get_voltage() synchronously
         */
        status = dvfs_handle_psu_get_voltage_resp(ctx,
            resp_event, status, ctx->request.new_opp.voltage);
        if (status == FWK_PENDING)
            return FWK_SUCCESS;
        return status;
    }

    /*
     * local DVFS event from dvfs_set_frequency()
     */
    if (fwk_id_is_equal(event->id, mod_dvfs_event_id_set)) {
        if (ctx->current_opp.voltage != 0) {
            voltage = ctx->current_opp.voltage;
            status = FWK_SUCCESS;
        } else {
            status = ctx->apis.psu->get_voltage(ctx->config->psu_id,
                &voltage);
            if (status == FWK_PENDING)
                return FWK_SUCCESS;
        }

        /*
         * Handle get_voltage() synchronously
         */
        status = dvfs_handle_psu_get_voltage_resp(ctx, NULL,
            status, voltage);
        if (status == FWK_PENDING)
            return FWK_SUCCESS;
        return status;
    }

    /*
     * local DVFS event from alarm_callback() generated when alarm set by
     * dvfs_handle_pending_request() fires
     */
    if (fwk_id_is_equal(event->id, mod_dvfs_event_id_retry)) {
        ctx->request.set_source_id = false;
        ctx->request_pending = false;
        status = dvfs_set_frequency_start(ctx, &ctx->pending_request.new_opp,
            ctx->pending_request.retry_request,
            ctx->pending_request.num_retries);
        ctx->pending_request = (struct mod_dvfs_request){ 0 };
        return status;
    }

    /*
     * response event from PSU get_voltage()
     */
    if (fwk_id_is_equal(event->id, mod_psu_event_id_get_voltage)) {
        /*
         * Handle get_voltage() asynchronously, cookie will have been saved
         * above so we can safely discard the resp_event.
         */
        psu_response = (struct mod_psu_driver_response *)event->params;
        status = dvfs_handle_psu_get_voltage_resp(ctx, NULL,
            psu_response->status, psu_response->voltage);
        if (status == FWK_PENDING)
            return FWK_SUCCESS;
        return status;
    }

    /*
     * response event from SET_OPP() PSU set_voltage()
     */
    if (fwk_id_is_equal(event->id, mod_psu_event_id_set_voltage)) {
        /*
         * Handle set_voltage() asynchronously, no response required for
         * a SET_OPP() request so resp_event discarded.
         */
        status = dvfs_handle_psu_set_voltage_resp(ctx, event);
        if (status == FWK_PENDING)
            return FWK_SUCCESS;
        return status;
    }

    /*
     * response event from SET_OPP() Clock set_rate()
     */
    if (fwk_id_is_equal(event->id, mod_clock_event_id_request)) {
        /*
         * Handle set_frequency() asynchronously, no response required for
         * a SET_OPP() request so resp_event discarded.
         */
        status = dvfs_handle_clk_set_freq_resp(ctx, event);
        if (status == FWK_PENDING)
            return FWK_SUCCESS;
        return status;
    }

    return FWK_E_PARAM;
}

/*
 * Module framework support
 */

/*
 * The DVFS voltage/frequency must be set to the sustained OPP at startup
 * for each domain.
 */
static int dvfs_start(fwk_id_t id)
{
    int status;
    struct mod_dvfs_opp sustained_opp;
    struct mod_dvfs_domain_ctx *ctx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return FWK_SUCCESS;

    status = dvfs_get_sustained_opp(id, &sustained_opp);
    if (status == FWK_SUCCESS) {
        ctx = get_domain_ctx(id);
        ctx->request.set_source_id = true;
        dvfs_set_frequency_start(ctx, &sustained_opp, true, 0);
    }

    return status;
}

static int dvfs_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    dvfs_ctx.domain_ctx = fwk_mm_calloc(element_count,
        sizeof((*dvfs_ctx.domain_ctx)[0]));

    dvfs_ctx.dvfs_domain_element_count = element_count;

    return FWK_SUCCESS;
}

static int
dvfs_element_init(fwk_id_t domain_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_dvfs_domain_ctx *ctx = get_domain_ctx(domain_id);

    fwk_assert(sub_element_count == 0);

    ctx->domain_id = domain_id;

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
    status = fwk_module_bind(ctx->config->psu_id, mod_psu_api_id_device,
        &ctx->apis.psu);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    /* Bind to the clock module */
    status = fwk_module_bind(ctx->config->clock_id,
        FWK_ID_API(FWK_MODULE_IDX_CLOCK, 0), &ctx->apis.clock);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    /* Bind to the alarm HAL if required */
    if (ctx->config->retry_ms > 0) {
        status = fwk_module_bind(ctx->config->alarm_id, MOD_TIMER_API_ID_ALARM,
            &ctx->apis.alarm_api);
        if (status != FWK_SUCCESS)
            return FWK_E_PANIC;
    }

    return FWK_SUCCESS;
}

static int
dvfs_bind(fwk_id_t id, unsigned int round)
{
    /* Bind our elements */
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
    .start = dvfs_start,
    .bind = dvfs_bind,
    .process_bind_request = dvfs_process_bind_request,
    .process_event = mod_dvfs_process_event,
    .api_count = MOD_DVFS_API_IDX_COUNT,
    .event_count = MOD_DVFS_INTERNAL_EVENT_IDX_COUNT,
};

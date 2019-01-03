/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_clock.h>
#include <mod_dvfs_private.h>
#include <mod_psu.h>

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

static bool is_opp_within_limits(
    const struct mod_dvfs_opp *opp,
    const struct mod_dvfs_frequency_limits *limits)
{
    return (opp->frequency >= limits->minimum) &&
           (opp->frequency <= limits->maximum);
}

static bool are_limits_valid(
    const struct mod_dvfs_domain_ctx *ctx,
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

static int api_get_current_opp(fwk_id_t domain_id, struct mod_dvfs_opp *opp)
{
    int status;
    const struct mod_dvfs_domain_ctx *ctx;

    assert(opp != NULL);

    ctx = __mod_dvfs_get_valid_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    status = __mod_dvfs_get_current_opp(ctx, opp);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

int api_get_sustained_opp(fwk_id_t domain_id, struct mod_dvfs_opp *opp)
{
    const struct mod_dvfs_domain_ctx *ctx;

    assert(opp != NULL);

    ctx = __mod_dvfs_get_valid_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *opp = ctx->config->opps[ctx->config->sustained_idx];

    return FWK_SUCCESS;
}

int api_get_nth_opp(fwk_id_t domain_id,
    size_t n,
    struct mod_dvfs_opp *opp)
{
    const struct mod_dvfs_domain_ctx *ctx;

    assert(opp != NULL);

    ctx = __mod_dvfs_get_valid_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    if (n >= ctx->opp_count)
        return FWK_E_PARAM;

    *opp = ctx->config->opps[n];

    return FWK_SUCCESS;
}

static int api_get_opp_count(fwk_id_t domain_id, size_t *opp_count)
{
    const struct mod_dvfs_domain_ctx *ctx;

    assert(opp_count != NULL);

    ctx = __mod_dvfs_get_valid_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *opp_count = ctx->opp_count;

    return FWK_SUCCESS;
}

int api_get_latency(fwk_id_t domain_id, uint16_t *latency)
{
    const struct mod_dvfs_domain_ctx *ctx;

    assert(latency != NULL);

    ctx = __mod_dvfs_get_valid_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *latency = ctx->config->latency;

    return FWK_SUCCESS;
}

static int api_set_frequency(fwk_id_t domain_id, uint64_t frequency)
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

static int api_set_frequency_async(fwk_id_t domain_id, uint64_t frequency)
{
    return FWK_E_SUPPORT;
}

int api_get_frequency_limits(
    fwk_id_t domain_id,
    struct mod_dvfs_frequency_limits *limits)
{
    const struct mod_dvfs_domain_ctx *ctx;

    ctx = __mod_dvfs_get_valid_domain_ctx(domain_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *limits = ctx->frequency_limits;

    return FWK_SUCCESS;
}

static int api_set_frequency_limits(
    fwk_id_t domain_id,
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

static int api_set_frequency_limits_async(
    fwk_id_t domain_id,
    const struct mod_dvfs_frequency_limits *limits)
{
    return FWK_E_SUPPORT;
}

const struct mod_dvfs_domain_api __mod_dvfs_domain_api = {
    .get_current_opp = api_get_current_opp,
    .get_sustained_opp = api_get_sustained_opp,
    .get_nth_opp = api_get_nth_opp,
    .get_opp_count = api_get_opp_count,
    .get_latency = api_get_latency,
    .set_frequency = api_set_frequency,
    .set_frequency_async = api_set_frequency_async,
    .get_frequency_limits = api_get_frequency_limits,
    .set_frequency_limits = api_set_frequency_limits,
    .set_frequency_limits_async = api_set_frequency_limits_async,
};

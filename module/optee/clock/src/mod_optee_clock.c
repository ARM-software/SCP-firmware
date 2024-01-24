/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Interface SCP-firmware clock module with OP-TEE clock resources.
 */

#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_log.h>

#include <mod_clock.h>
#include <mod_optee_clock.h>

#include <compiler.h>
#include <drivers/clk.h>
#include <tee_api_types.h>

#include <stdbool.h>

#define MOD_NAME "[SCMI CLOCK] "

/* OP-TEE clock device context */
struct optee_clock_dev_ctx {
    struct clk *clk;
    bool enabled;
};

/* OP-TEE clock module context */
struct optee_clock_module_ctx {
    struct optee_clock_dev_ctx *dev_ctx;
    unsigned int dev_count;
};

static struct optee_clock_module_ctx module_ctx;

static struct optee_clock_dev_ctx *elt_id_to_ctx(fwk_id_t dev_id)
{
    if (!fwk_module_is_valid_element_id(dev_id)) {
        return NULL;
    }

    return module_ctx.dev_ctx + fwk_id_get_element_idx(dev_id);
}

static bool is_exposed(struct optee_clock_dev_ctx *ctx)
{
    return ctx->clk != NULL;
}

/*
 * Clock driver API functions
 */
static int get_rate(fwk_id_t dev_id, uint64_t *rate)
{
    struct optee_clock_dev_ctx *ctx = elt_id_to_ctx(dev_id);

    if ((ctx == NULL) || (rate == NULL)) {
        return FWK_E_PARAM;
    }

    if (!is_exposed(ctx)) {
        *rate = 0;
        return FWK_SUCCESS;
    }

    *rate = clk_get_rate(ctx->clk);

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI optee_clock (%u/\"%s\"): clk_get_rate() = %" PRIu64,
        fwk_id_get_element_idx(dev_id),
        clk_get_name(ctx->clk),
        *rate);

    return FWK_SUCCESS;
}

static int set_state(fwk_id_t dev_id, enum mod_clock_state state)
{
    struct optee_clock_dev_ctx *ctx = elt_id_to_ctx(dev_id);
    TEE_Result res;

    if (ctx == NULL) {
        return FWK_E_PARAM;
    }

    switch (state) {
    case MOD_CLOCK_STATE_STOPPED:
    case MOD_CLOCK_STATE_RUNNING:
        break;
    default:
        return FWK_E_PARAM;
    }

    if (!is_exposed(ctx)) {
        if (state == MOD_CLOCK_STATE_STOPPED) {
            return FWK_SUCCESS;
        } else {
            return FWK_E_ACCESS;
        }
    }

    if (state == MOD_CLOCK_STATE_STOPPED) {
        if (ctx->enabled) {
            FWK_LOG_DEBUG(
                MOD_NAME "SCMI optee_clock (%u/\"%s\") disable",
                fwk_id_get_element_idx(dev_id),
                clk_get_name(ctx->clk));

            clk_disable(ctx->clk);
            ctx->enabled = false;
        } else {
            FWK_LOG_DEBUG(
                MOD_NAME "SCMI optee_clock (%u/\"%s\") is already OFF",
                fwk_id_get_element_idx(dev_id),
                clk_get_name(ctx->clk));
        }
    } else {
        if (!ctx->enabled) {
            FWK_LOG_DEBUG(
                MOD_NAME "SCMI optee_clock (%u/\"%s\") enable",
                fwk_id_get_element_idx(dev_id),
                clk_get_name(ctx->clk));

            res = clk_enable(ctx->clk);
            if (res != TEE_SUCCESS) {
                return FWK_E_DEVICE;
            }

            ctx->enabled = true;
        } else {
            FWK_LOG_DEBUG(
                MOD_NAME "SCMI optee_clock (%u/\"%s\") is already ON",
                fwk_id_get_element_idx(dev_id),
                clk_get_name(ctx->clk));
        }
    }

    return FWK_SUCCESS;
}

static int get_state(fwk_id_t dev_id, enum mod_clock_state *state)
{
    struct optee_clock_dev_ctx *ctx = elt_id_to_ctx(dev_id);

    if ((ctx == NULL) || (state == NULL)) {
        return FWK_E_PARAM;
    }

    if (!is_exposed(ctx)) {
        *state = MOD_CLOCK_STATE_STOPPED;
        return FWK_SUCCESS;
    }

    if (ctx->enabled) {
        *state = MOD_CLOCK_STATE_RUNNING;
    } else {
        *state = MOD_CLOCK_STATE_STOPPED;
    }

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI optee_clock (%u/\"%s\") is %s",
        fwk_id_get_element_idx(dev_id),
        clk_get_name(ctx->clk),
        *state == MOD_CLOCK_STATE_STOPPED ? "off" : "on");

    return FWK_SUCCESS;
}

static int get_range(fwk_id_t dev_id, struct mod_clock_range *range)
{
    struct optee_clock_dev_ctx *ctx = elt_id_to_ctx(dev_id);
    unsigned long rate;
    size_t rate_count;
    size_t rate_idx;
    TEE_Result res;

    if ((ctx == NULL) || (range == NULL)) {
        return FWK_E_PARAM;
    }

    range->rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE;

    if (!is_exposed(ctx)) {
        range->min = 0;
        range->max = 0;
        range->rate_count = 1;

        return FWK_SUCCESS;
    }

    res = clk_get_rates_array(ctx->clk, 0, NULL, &rate_count);
    if (res == TEE_ERROR_NOT_SUPPORTED) {
        range->min = clk_get_rate(ctx->clk);
        range->max = range->min;
        range->rate_count = 1;

        return FWK_SUCCESS;
    } else if (res != TEE_SUCCESS) {
        return FWK_E_DEVICE;
    }

    range->rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE;
    range->rate_count = rate_count;
    range->min = UINT64_MAX;
    range->max = 0;

    for (rate_idx = 0; rate_idx < rate_count; rate_idx++) {
        size_t count = 1;

        res = clk_get_rates_array(ctx->clk, rate_idx, &rate, &count);
        fwk_assert(!res && count == 1);

        if (rate > range->max) {
            range->max = rate;
        } else if (rate < range->min) {
            range->min = rate;
        }
    }

    return FWK_SUCCESS;
}

static int set_rate(fwk_id_t dev_id, uint64_t rate,
                    enum mod_clock_round_mode round_mode)
{
    struct optee_clock_dev_ctx *ctx = elt_id_to_ctx(dev_id);
    TEE_Result res;

    if (ctx == NULL) {
        return FWK_E_PARAM;
    }

    if (!is_exposed(ctx)) {
        return FWK_E_ACCESS;
    }

    res = clk_set_rate(ctx->clk, rate);
    if (res == TEE_ERROR_NOT_SUPPORTED) {
        return FWK_E_SUPPORT;
    } else if (res != TEE_SUCCESS) {
        return FWK_E_DEVICE;
    }

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI optee_clock (%u/\"%s\"): rate = %" PRIu64,
        fwk_id_get_element_idx(dev_id),
        clk_get_name(ctx->clk),
        rate);

    return FWK_SUCCESS;
}

static int get_rate_from_index(fwk_id_t dev_id,
                               unsigned int rate_index, uint64_t *rate)
{
    struct optee_clock_dev_ctx *ctx = elt_id_to_ctx(dev_id);
    unsigned long rate_ul;
    size_t rate_count;
    TEE_Result res;

    if ((ctx == NULL) || (rate == NULL)) {
        return FWK_E_PARAM;
    }

    if (!is_exposed(ctx)) {
        *rate = 0;
        return FWK_SUCCESS;
    }

    res = clk_get_rates_array(ctx->clk, 0, NULL, &rate_count);
    if (res == TEE_ERROR_NOT_SUPPORTED) {
        if (rate_index > 0) {
            return FWK_E_PARAM;
        }

        *rate = clk_get_rate(ctx->clk);
        return FWK_SUCCESS;
    } else if (res != TEE_SUCCESS) {
        return FWK_E_DEVICE;
    }

    if (rate_index > rate_count) {
        return FWK_E_PARAM;
    }

    rate_count = 1;
    res = clk_get_rates_array(ctx->clk, rate_index, &rate_ul, &rate_count);
        fwk_assert(!res && rate_count == 1);
    *rate = rate_ul;

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI optee_clock (%u/\"%s\"): rate(index %u) = %lu",
        fwk_id_get_element_idx(dev_id),
        clk_get_name(ctx->clk),
        rate_index,
        rate_ul);

    return FWK_SUCCESS;
}

static int stub_process_power_transition(fwk_id_t dev_id, unsigned int state)
{
    return FWK_E_SUPPORT;
}

static int stub_pending_power_transition(fwk_id_t dev_id,
                                         unsigned int current_state,
                                         unsigned int next_state)
{
    return FWK_E_SUPPORT;
}

static const struct mod_clock_drv_api api_optee_clock = {
    .get_rate = get_rate,
    .set_state = set_state,
    .get_state = get_state,
    .get_range = get_range,
    .get_rate_from_index = get_rate_from_index,
    .set_rate = set_rate,
    /* Not supported */
    .process_power_transition = stub_process_power_transition,
    .process_pending_power_transition = stub_pending_power_transition,
};

/*
 * Framework handler functions
 */

static int optee_clock_init(fwk_id_t module_id, unsigned int count,
                            const void *data)
{
    if (count == 0) {
        return FWK_SUCCESS;
    }

    module_ctx.dev_count = count;
    module_ctx.dev_ctx = fwk_mm_calloc(count, sizeof(*module_ctx.dev_ctx));

    return FWK_SUCCESS;
}

static int optee_clock_element_init(fwk_id_t element_id, unsigned int dev_count,
                                    const void *data)
{
    struct optee_clock_dev_ctx *ctx = elt_id_to_ctx(element_id);
    const struct mod_optee_clock_config *config = data;
    TEE_Result res;

    ctx->clk = config->clk;
    if (ctx->clk) {
        ctx->enabled = config->default_enabled;

        if (ctx->enabled) {
            res = clk_enable(ctx->clk);
            if (res) {
                return FWK_E_DEVICE;
            }
        }
    }

    return FWK_SUCCESS;
}

static int optee_clock_process_bind_request(fwk_id_t requester_id, fwk_id_t id,
                                            fwk_id_t api_type, const void **api)
{
    *api = &api_optee_clock;

    return FWK_SUCCESS;
}

const struct fwk_module module_optee_clock = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .event_count = 0,
    .init = optee_clock_init,
    .element_init = optee_clock_element_init,
    .process_bind_request = optee_clock_process_bind_request,
};

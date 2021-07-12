/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_clock.h>
#include <mod_mock_clock.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static struct mod_mock_clock_element_ctx {
    const struct mod_mock_clock_element_cfg *config;
    unsigned int current_rate_index;
    enum mod_clock_state state;

    /*
     * We don't rely on the bootloader leaving the clocks
     * in any specific state.
     */
    bool rate_initialized;
} * elements_ctx;

static struct mod_mock_clock_element_ctx *mod_mock_clock_get_ctx(
    fwk_id_t element_id)
{
    unsigned int element_idx = fwk_id_get_element_idx(element_id);

    return &elements_ctx[element_idx];
}

static int get_rate_entry(
    struct mod_mock_clock_element_ctx *ctx,
    uint64_t target_rate,
    const struct mod_mock_clock_rate **entry)
{
    unsigned int i;

    fwk_assert(ctx != NULL);
    fwk_assert(entry != NULL);

    /* Find the entry matching the requested rate */
    for (i = 0; i < ctx->config->rate_count; i++) {
        if (ctx->config->rate_table[i].rate == target_rate) {
            *entry = (struct mod_mock_clock_rate *)&ctx->config->rate_table[i];
            return FWK_SUCCESS;
        }
    }

    return FWK_E_PARAM;
}

/*
 * Mock clock driver functions
 */

static int mod_mock_clock_set_rate(
    fwk_id_t clock_id,
    uint64_t rate,
    enum mod_clock_round_mode round_mode)
{
    int status;
    struct mod_mock_clock_element_ctx *ctx;
    const struct mod_mock_clock_rate *rate_entry;

    ctx = mod_mock_clock_get_ctx(clock_id);

    if (ctx->state == MOD_CLOCK_STATE_STOPPED) {
        return FWK_E_STATE;
    }

    /*
     * Look up the divider and source settings. We do not perform any rounding
     * on the clock rate given as input which has to be precise in order not to
     * be refused with an FWK_E_PARAM error code.
     */
    status = get_rate_entry(ctx, rate, &rate_entry);
    if (status != FWK_SUCCESS) {
        return FWK_E_PARAM;
    }

    ctx->current_rate_index =
        (unsigned int)(rate_entry - ctx->config->rate_table);

    ctx->rate_initialized = true;

    return FWK_SUCCESS;
}

static int mod_mock_clock_get_rate(fwk_id_t clock_id, uint64_t *rate)
{
    struct mod_mock_clock_element_ctx *ctx;

    ctx = mod_mock_clock_get_ctx(clock_id);

    if (!ctx->rate_initialized) {
        return FWK_E_STATE;
    }

    *rate = ctx->config->rate_table[ctx->current_rate_index].rate;

    return FWK_SUCCESS;
}

static int mod_mock_clock_get_rate_from_index(
    fwk_id_t clock_id,
    unsigned int rate_index,
    uint64_t *rate)
{
    struct mod_mock_clock_element_ctx *ctx;

    ctx = mod_mock_clock_get_ctx(clock_id);

    if (rate_index >= ctx->config->rate_count) {
        return FWK_E_PARAM;
    }

    *rate = ctx->config->rate_table[rate_index].rate;

    return FWK_SUCCESS;
}

static int mod_mock_clock_set_state(
    fwk_id_t clock_id,
    enum mod_clock_state state)
{
    struct mod_mock_clock_element_ctx *ctx;

    ctx = mod_mock_clock_get_ctx(clock_id);

    ctx->state = state;

    return FWK_SUCCESS;
}

static int mod_mock_clock_get_state(
    fwk_id_t clock_id,
    enum mod_clock_state *state)
{
    struct mod_mock_clock_element_ctx *ctx;

    ctx = mod_mock_clock_get_ctx(clock_id);

    *state = ctx->state;

    return FWK_SUCCESS;
}

static int mod_mock_clock_get_range(
    fwk_id_t clock_id,
    struct mod_clock_range *range)
{
    struct mod_mock_clock_element_ctx *ctx;

    ctx = mod_mock_clock_get_ctx(clock_id);

    range->rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE;
    range->min = ctx->config->rate_table[0].rate;
    range->max = ctx->config->rate_table[ctx->config->rate_count - 1].rate;
    range->rate_count = ctx->config->rate_count;

    return FWK_SUCCESS;
}

/*
 * Notification handler invoked after the state of a clock's power domain
 * has changed.
 */
static int mod_mock_clock_process_power_transition(
    fwk_id_t clock_id,
    unsigned int state)
{
    /* Noop, the power state won't affect this module */
    return FWK_SUCCESS;
}

static int mod_mock_clock_update_input_rate(
    fwk_id_t clock_id,
    uint64_t input_rate,
    uint64_t *output_rate)
{
    struct mod_mock_clock_element_ctx *ctx;
    const struct mod_mock_clock_rate *rate_entry;
    int status;

    ctx = mod_mock_clock_get_ctx(clock_id);

    if (ctx->state == MOD_CLOCK_STATE_STOPPED || !ctx->rate_initialized) {
        return FWK_E_STATE;
    }
    /*
     * Performs the same operation as set_rate synchronously
     */
    status = get_rate_entry(ctx, input_rate, &rate_entry);
    if (status != FWK_SUCCESS) {
        return FWK_E_PARAM;
    }

    ctx->current_rate_index =
        (unsigned int)(rate_entry - ctx->config->rate_table);

    return mod_mock_clock_get_rate(clock_id, output_rate);
}

static const struct mod_clock_drv_api mod_mock_clock_driver_api = {
    .set_rate = mod_mock_clock_set_rate,
    .get_rate = mod_mock_clock_get_rate,
    .get_rate_from_index = mod_mock_clock_get_rate_from_index,
    .set_state = mod_mock_clock_set_state,
    .get_state = mod_mock_clock_get_state,
    .get_range = mod_mock_clock_get_range,
    .process_power_transition = mod_mock_clock_process_power_transition,
    .update_input_rate = mod_mock_clock_update_input_rate,
};

static const struct mod_clock_driver_response_api
    mod_mock_clock_response_driver_api = { 0 };

static int mod_mock_clock_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    fwk_check(data == NULL);

    elements_ctx = fwk_mm_calloc(element_count, sizeof(elements_ctx[0]));

    return FWK_SUCCESS;
}

static int mod_mock_clock_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    unsigned int rate_index = 0;
    uint64_t rate;
    uint64_t last_rate = 0;
    struct mod_mock_clock_element_ctx *ctx;
    const struct mod_mock_clock_element_cfg *cfg = data;

    fwk_check(sub_element_count == 0);

    ctx = mod_mock_clock_get_ctx(element_id);

    /* Verify that the rate entries in the lookup table are ordered */
    for (rate_index = 0; rate_index < cfg->rate_count; rate_index++) {
        rate = cfg->rate_table[rate_index].rate;

        /* The rate entries must be in ascending order */
        if (rate < last_rate) {
            return FWK_E_DATA;
        }

        last_rate = rate;

        /* If a default rate is set, let it be so */
        if (cfg->default_rate == rate) {
            ctx->current_rate_index = rate_index;
            ctx->rate_initialized = true;
            ctx->state = MOD_CLOCK_STATE_RUNNING;
        }
    }

    ctx->config = cfg;

    return FWK_SUCCESS;
}

static int mod_mock_clock_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    enum mod_mock_clock_api_type api_type = fwk_id_get_api_idx(api_id);

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_E_ACCESS;
    }

    switch (api_type) {
    case MOD_MOCK_CLOCK_API_TYPE_DRIVER:
        *api = &mod_mock_clock_driver_api;
        return FWK_SUCCESS;

    case MOD_MOCK_CLOCK_API_TYPE_RESPONSE_DRIVER:
        *api = &mod_mock_clock_response_driver_api;
        return FWK_SUCCESS;

    default:
        return FWK_E_ACCESS;
    }
}

const struct fwk_module module_mock_clock = {
    .type = FWK_MODULE_TYPE_DRIVER,

    .init = mod_mock_clock_init,
    .element_init = mod_mock_clock_element_init,

    .api_count = (unsigned int)MOD_MOCK_CLOCK_API_COUNT,
    .process_bind_request = mod_mock_clock_process_bind_request,
};

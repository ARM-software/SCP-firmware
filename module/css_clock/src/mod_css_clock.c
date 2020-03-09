/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_clock.h>
#include <mod_css_clock.h>
#include <mod_power_domain.h>

#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdint.h>
#include <stdlib.h>

/* Device context */
struct css_clock_dev_ctx {
    bool initialized;
    uint64_t current_rate;
    struct mod_clock_drv_api *pll_api;
    struct mod_css_clock_direct_api *clock_api;
    const struct mod_css_clock_dev_config *config;
};

/* Module context */
struct css_clock_ctx {
    struct css_clock_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
};

static struct css_clock_ctx module_ctx;

/*
 * Static helper functions
 */

static int compare_rate_entry(const void *a, const void *b)
{
    struct mod_css_clock_rate *key = (struct mod_css_clock_rate *)a;
    struct mod_css_clock_rate *element = (struct mod_css_clock_rate *)b;

    return (key->rate - element->rate);
}

static int get_rate_entry(struct css_clock_dev_ctx *ctx, uint64_t target_rate,
                          struct mod_css_clock_rate **entry)
{
    struct mod_css_clock_rate *current_rate_entry;

    if (ctx == NULL)
        return FWK_E_PARAM;
    if (entry == NULL)
        return FWK_E_PARAM;

    /* Perform a binary search to find the entry matching the requested rate */
    current_rate_entry = (struct mod_css_clock_rate *) bsearch(&target_rate,
        ctx->config->rate_table, ctx->config->rate_count,
        sizeof(struct mod_css_clock_rate), compare_rate_entry);

    if (current_rate_entry == NULL)
        return FWK_E_PARAM;

    *entry = current_rate_entry;
    return FWK_SUCCESS;
}

static int set_rate_indexed(struct css_clock_dev_ctx *ctx, uint64_t rate,
                            enum mod_clock_round_mode round_mode)
{
    int status;
    unsigned int i;
    struct mod_css_clock_rate *rate_entry;

    if (ctx == NULL)
        return FWK_E_PARAM;

    /* Look up the divider and source settings */
    status = get_rate_entry(ctx, rate, &rate_entry);
    if (status != FWK_SUCCESS)
        goto exit;

    /* Switch each member clock away from the PLL source */
    for (i = 0; i < ctx->config->member_count; i++) {
        status = ctx->clock_api->set_source(ctx->config->member_table[i],
            ctx->config->clock_switching_source);
        if (status != FWK_SUCCESS)
            goto exit;

        status = ctx->clock_api->set_div(ctx->config->member_table[i],
                                         rate_entry->clock_div_type,
                                         rate_entry->clock_div);
        if (status != FWK_SUCCESS)
            goto exit;

        if (ctx->config->modulation_supported) {
            status = ctx->clock_api->set_mod(ctx->config->member_table[i],
                                             rate_entry->clock_mod_numerator,
                                             rate_entry->clock_mod_denominator);
            if (status != FWK_SUCCESS)
                goto exit;
        }
    }

    /* Change the PLL to the desired rate */
    status = ctx->pll_api->set_rate(ctx->config->pll_id, rate_entry->pll_rate,
                                    MOD_CLOCK_ROUND_MODE_NONE);
    if (status != FWK_SUCCESS)
        goto exit;

    /* Return each member clock back to the PLL source */
    for (i = 0; i < ctx->config->member_count; i++) {
        status = ctx->clock_api->set_source(ctx->config->member_table[i],
                                            rate_entry->clock_source);
        if (status != FWK_SUCCESS)
            goto exit;
    }

exit:
    if (status == FWK_SUCCESS)
        ctx->current_rate = rate;
    return status;
}

static int set_rate_non_indexed(struct css_clock_dev_ctx *ctx, uint64_t rate,
                                enum mod_clock_round_mode round_mode)
{
    int status;
    unsigned int i;

    if (ctx == NULL)
        return FWK_E_PARAM;

    /* Switch each member clock away from the PLL source */
    for (i = 0; i < ctx->config->member_count; i++) {
        status = ctx->clock_api->set_source(ctx->config->member_table[i],
            ctx->config->clock_switching_source);
        if (status != FWK_SUCCESS)
            goto exit;
    }

    /* Change the PLL to the desired rate */
    status = ctx->pll_api->set_rate(ctx->config->pll_id, rate, round_mode);
    if (status != FWK_SUCCESS)
        goto exit;

    /* Return each member clock back to the PLL source */
    for (i = 0; i < ctx->config->member_count; i++) {
        status = ctx->clock_api->set_source(ctx->config->member_table[i],
                                            ctx->config->clock_default_source);
        if (status != FWK_SUCCESS)
            goto exit;
    }

exit:
    if (status == FWK_SUCCESS)
        ctx->current_rate = rate;
    return status;
}

/*
 * Module API functions
 */

static int css_clock_set_rate(fwk_id_t dev_id, uint64_t rate,
                              enum mod_clock_round_mode round_mode)
{
    struct css_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (ctx->config->clock_type == MOD_CSS_CLOCK_TYPE_INDEXED)
        return set_rate_indexed(ctx, rate, round_mode);
    else
        return set_rate_non_indexed(ctx, rate, round_mode);
}

static int css_clock_get_rate(fwk_id_t dev_id, uint64_t *rate)
{
    struct css_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
    *rate = ctx->current_rate;

    return FWK_SUCCESS;
}

static int css_clock_get_rate_from_index(fwk_id_t dev_id,
                                         unsigned int rate_index,
                                         uint64_t *rate)
{
    struct css_clock_dev_ctx *ctx;

    if (rate == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (rate_index >= ctx->config->rate_count)
        return FWK_E_PARAM;

    if (ctx->config->clock_type == MOD_CSS_CLOCK_TYPE_INDEXED) {
        *rate = ctx->config->rate_table[rate_index].rate;
        return FWK_SUCCESS;
    } else
        return FWK_E_SUPPORT;
}

static int css_clock_set_state(fwk_id_t dev_id, enum mod_clock_state state)
{
    if (state == MOD_CLOCK_STATE_RUNNING)
        return FWK_SUCCESS; /* CSS clocks are always running */

    /* CSS clocks cannot be turned off */
    return FWK_E_SUPPORT;
}

static int css_clock_get_state(fwk_id_t dev_id, enum mod_clock_state *state)
{
    *state = MOD_CLOCK_STATE_RUNNING;

    return FWK_SUCCESS;
}

static int css_clock_get_range(fwk_id_t dev_id, struct mod_clock_range *range)
{
    struct css_clock_dev_ctx *ctx;

    if (range == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (ctx->config->clock_type == MOD_CSS_CLOCK_TYPE_INDEXED) {
        range->rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE;
        range->min = ctx->config->rate_table[0].rate;
        range->max = ctx->config->rate_table[ctx->config->rate_count - 1].rate;
        range->rate_count = ctx->config->rate_count;
        return FWK_SUCCESS;
    } else
        return ctx->pll_api->get_range(ctx->config->pll_id, range);
}

static int css_clock_power_state_change(
    fwk_id_t dev_id,
    unsigned int next_state)
{
    int status;
    unsigned int clock_idx;
    struct css_clock_dev_ctx *ctx;
    const struct mod_css_clock_dev_config *dev_config;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
    dev_config = ctx->config;

    /* The group's clock driver is not required to handle this transition */
    if (ctx->clock_api->process_power_transition != NULL) {
        for (clock_idx = 0; clock_idx < dev_config->member_count; clock_idx++) {
            /* Allow the member clock's driver to perform any required
             * processing */
            status = ctx->clock_api->process_power_transition(
                dev_config->member_table[clock_idx], next_state);

            if (status != FWK_SUCCESS)
                return status;
        }
    }

    if (next_state == MOD_PD_STATE_ON) {
        if (ctx->initialized) {
            /* Restore all clocks in the group to the last frequency */
            return css_clock_set_rate(dev_id, ctx->current_rate,
                                      MOD_CLOCK_ROUND_MODE_NONE);
        } else {
            ctx->initialized = true;
            /* Set all clocks in the group to the initial frequency */
            return css_clock_set_rate(dev_id, dev_config->initial_rate,
                                      MOD_CLOCK_ROUND_MODE_NONE);
        }
    }

    return FWK_SUCCESS;
}

static int css_clock_pending_power_state_change(
    fwk_id_t dev_id,
    unsigned int current_state,
    unsigned int next_state)
{
    int status;
    unsigned int clock_idx;
    struct css_clock_dev_ctx *ctx;
    const struct mod_css_clock_dev_config *dev_config;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
    dev_config = ctx->config;

    /* The group's clock driver is not required to handle this transition */
    if (ctx->clock_api->process_pending_power_transition != NULL) {
        for (clock_idx = 0; clock_idx < dev_config->member_count; clock_idx++) {
            /* Allow the member clock's driver to perform any required
             * processing */
            status = ctx->clock_api->process_pending_power_transition(
                dev_config->member_table[clock_idx], current_state, next_state);

            if (status != FWK_SUCCESS)
                return status;
        }
    }

    /* Nothing specific to be done in this driver */
    return FWK_SUCCESS;
}

static const struct mod_clock_drv_api api_clock = {
    .set_rate = css_clock_set_rate,
    .get_rate = css_clock_get_rate,
    .get_rate_from_index = css_clock_get_rate_from_index,
    .set_state = css_clock_set_state,
    .get_state = css_clock_get_state,
    .get_range = css_clock_get_range,
    .process_power_transition = css_clock_power_state_change,
    .process_pending_power_transition = css_clock_pending_power_state_change,
};

/*
 * Framework handler functions
 */

static int css_clock_init(fwk_id_t module_id, unsigned int element_count,
                          const void *data)
{
    module_ctx.dev_count = element_count;

    if (element_count == 0)
        return FWK_SUCCESS;

    module_ctx.dev_ctx_table = fwk_mm_calloc(element_count,
                                             sizeof(struct css_clock_dev_ctx));
    return FWK_SUCCESS;
}

static int css_clock_element_init(fwk_id_t element_id,
                                  unsigned int sub_element_count,
                                  const void *data)
{
    unsigned int i = 0;
    uint64_t current_rate;
    uint64_t last_rate = 0;
    struct css_clock_dev_ctx *ctx;
    const struct mod_css_clock_dev_config *dev_config = data;

    if (!fwk_module_is_valid_element_id(element_id))
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(element_id);

    if (dev_config->clock_type == MOD_CSS_CLOCK_TYPE_INDEXED) {
        /* Verify that the rate entries in the lookup table are ordered */
        while (i < dev_config->rate_count) {
            current_rate = dev_config->rate_table[i].rate;

            /* The rate entries must be in ascending order */
            if (current_rate < last_rate)
                return FWK_E_DATA;

            last_rate = current_rate;
            i++;
        }
    }

    ctx->config = dev_config;
    ctx->current_rate = ctx->config->initial_rate;

    return FWK_SUCCESS;
}

static int css_clock_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct css_clock_dev_ctx *ctx;
    const struct mod_css_clock_dev_config *config;

    if (round == 1)
        return FWK_SUCCESS;

    if (fwk_module_is_valid_module_id(id))
        /* No module-level binding required */
        return FWK_SUCCESS;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(id);
    config = ctx->config;

    /* Ensure that the group has at least one member */
    if (config->member_count == 0)
        return FWK_E_DATA;

    /* Bind to the group's common PLL driver */
    status = fwk_module_bind(config->pll_id, config->pll_api_id,
                             &ctx->pll_api);
    if (status != FWK_SUCCESS)
        return status;

    /* Bind to the API used to control the clocks in the group */
    status = fwk_module_bind(config->member_table[0],
                             config->member_api_id, &ctx->clock_api);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

static int css_clock_process_bind_request(fwk_id_t source_id,
                                          fwk_id_t target_id, fwk_id_t api_id,
                                          const void **api)
{
    if (fwk_id_get_api_idx(api_id) != MOD_CSS_CLOCK_API_TYPE_CLOCK)
        /* The requested API is not supported. */
        return FWK_E_ACCESS;

    *api = &api_clock;
    return FWK_SUCCESS;
}

const struct fwk_module module_css_clock = {
    .name = "Subsystem Clock Driver",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_CSS_CLOCK_API_COUNT,
    .event_count = 0,
    .init = css_clock_init,
    .element_init = css_clock_element_init,
    .bind = css_clock_bind,
    .process_bind_request = css_clock_process_bind_request,
};

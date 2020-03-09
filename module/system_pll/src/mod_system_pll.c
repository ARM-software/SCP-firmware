/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_clock.h>
#include <mod_power_domain.h>
#include <mod_system_pll.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>

/* Device context */
struct system_pll_dev_ctx {
    bool initialized;
    uint64_t current_rate;
    enum mod_clock_state current_state;
    const struct mod_system_pll_dev_config *config;
};

/* Module context */
struct system_pll_ctx {
    struct system_pll_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
};

static struct system_pll_ctx module_ctx;

/*
 * Static helper functions
 */

/*
 * Given a frequency (Hz), return the period (picoseconds) of a half cycle.
 *
 * Note: For performance reasons, 32-bit math is used for the conversion. This
 *       may cause a loss of precision if the given frequency is not a multiple
 *       of 1 KHz.
 */
static unsigned int freq_to_half_cycle_ps(unsigned int freq_hz)
{
    unsigned int freq_khz;

    /* Check if the given frequency is a multiple of 1 KHz */
    if (freq_hz % MOD_SYSTEM_PLL_MIN_INTERVAL != 0)
        return FWK_E_PARAM;

    freq_khz = freq_hz / FWK_KHZ;
    if (freq_khz == 0)
        return 0;

    /*
     * Legend:
     *   s = seconds
     *   P = Half cycle period in picoseconds
     *   Fh = Frequency in hertz
     *   Fk = Frequency in kilohertz
     *
     * Starting from "Period = Time / Frequency"
     * General equation for half cycle in picoseconds:
     *     P = ((1s / Fh ) / 2) * 10^12
     * To avoid decimal calculations, re-arrange and simplify equation:
     *     P = 10^12 / 2 * Fh
     * To avoid dividend overflowing a 32-bit storage, dividend and divisor can
     * be divided by 10^3:
     *     P = (10^12 / 10^3) / (2 * Fh / 10^3)
     * Given Fk = Fh / 10^3, the equation can be further simplified as:
     *     P = 10^9 / 2 * Fk
     *     P = 5*10^8 / Fk
     */
    return 500000000UL / freq_khz;
}

/*
 * Clock driver API functions
 */

static int system_pll_set_rate(fwk_id_t dev_id, uint64_t rate,
                               enum mod_clock_round_mode round_mode)
{
    uint64_t rounded_rate;
    uint64_t rounded_rate_alt;
    unsigned int picoseconds;
    struct system_pll_dev_ctx *ctx;

    if (!fwk_module_is_valid_element_id(dev_id))
    return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (ctx->current_state == MOD_CLOCK_STATE_STOPPED)
        return FWK_E_PWRSTATE;

    /* If the given rate is not attainable as-is then round as requested */
    if ((rate % ctx->config->min_step) > 0) {
        switch (round_mode) {
        case MOD_CLOCK_ROUND_MODE_NONE:
            return FWK_E_RANGE;
        case MOD_CLOCK_ROUND_MODE_NEAREST:
            rounded_rate = FWK_ALIGN_PREVIOUS(rate, ctx->config->min_step);
            rounded_rate_alt = FWK_ALIGN_NEXT(rate, ctx->config->min_step);

            /* Select the rounded rate that is closest to the given rate */
            if ((rate - rounded_rate) > (rounded_rate_alt - rate))
                rounded_rate = rounded_rate_alt;
            break;
        case MOD_CLOCK_ROUND_MODE_DOWN:
            rounded_rate = FWK_ALIGN_PREVIOUS(rate, ctx->config->min_step);
            break;
        case MOD_CLOCK_ROUND_MODE_UP:
            rounded_rate = FWK_ALIGN_NEXT(rate, ctx->config->min_step);
            break;
        default:
            return FWK_E_SUPPORT;
        }
    } else
        rounded_rate = rate;

    if (rounded_rate < ctx->config->min_rate)
        return FWK_E_RANGE;
    if (rounded_rate > ctx->config->max_rate)
        return FWK_E_RANGE;

    picoseconds = freq_to_half_cycle_ps(rounded_rate);

    if (picoseconds == 0)
        return FWK_E_RANGE;

    *ctx->config->control_reg = picoseconds;

    if (ctx->config->status_reg != NULL) {
        /* Wait until the PLL has locked */
        while ((*ctx->config->status_reg & ctx->config->lock_flag_mask) == 0)
            continue;
    }

    ctx->current_rate = rounded_rate;

    return FWK_SUCCESS;
}

static int system_pll_get_rate(fwk_id_t dev_id, uint64_t *rate)
{
    struct system_pll_dev_ctx *ctx;

    if (!fwk_module_is_valid_element_id(dev_id))
        return FWK_E_PARAM;
    if (rate == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
    *rate = ctx->current_rate;

    return FWK_SUCCESS;
}

static int system_pll_get_rate_from_index(fwk_id_t dev_id,
                                          unsigned int rate_index,
                                          uint64_t *rate)
{
    /* PLLs have a continuous range of rates and are not indexed */
    return FWK_E_SUPPORT;
}

static int system_pll_set_state(fwk_id_t dev_id, enum mod_clock_state state)
{
    if (state == MOD_CLOCK_STATE_RUNNING)
        return FWK_SUCCESS;

    /* PLLs can only be stopped by a parent power domain state change. */
    return FWK_E_SUPPORT;
}

static int system_pll_get_state(fwk_id_t dev_id, enum mod_clock_state *state)
{
    struct system_pll_dev_ctx *ctx;

    if (!fwk_module_is_valid_element_id(dev_id))
        return FWK_E_PARAM;
    if (state == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
    *state = ctx->current_state;

    return FWK_SUCCESS;
}

static int system_pll_get_range(fwk_id_t dev_id, struct mod_clock_range *range)
{
    struct system_pll_dev_ctx *ctx;

    if (!fwk_module_is_valid_element_id(dev_id))
        return FWK_E_PARAM;
    if (range == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    range->rate_type = MOD_CLOCK_RATE_TYPE_CONTINUOUS;
    range->min = ctx->config->min_rate;
    range->max = ctx->config->max_rate;
    range->step = ctx->config->min_step;

    return FWK_SUCCESS;
}

static int system_pll_power_state_change(
    fwk_id_t dev_id,
    unsigned int state)
{
    uint64_t rate;
    struct system_pll_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (state != MOD_PD_STATE_ON)
        return FWK_SUCCESS;

    ctx->current_state = MOD_CLOCK_STATE_RUNNING;

    if (ctx->initialized) {
        /* Restore the previous rate */
        rate = ctx->current_rate;
    } else {
        /* Initialize the PLL to its default rate */
        ctx->initialized = true;
        rate = ctx->config->initial_rate;
    }

    return system_pll_set_rate(dev_id, rate, MOD_CLOCK_ROUND_MODE_NONE);
}

static int system_pll_power_state_pending_change(
    fwk_id_t dev_id,
    unsigned int current_state,
    unsigned int next_state)
{
    struct system_pll_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (next_state == MOD_PD_STATE_OFF) {
        /* Just mark the PLL as stopped */
        ctx->current_state = MOD_CLOCK_STATE_STOPPED;
    }

    return FWK_SUCCESS;
}

static const struct mod_clock_drv_api api_system_pll = {
    .set_rate = system_pll_set_rate,
    .get_rate = system_pll_get_rate,
    .get_rate_from_index = system_pll_get_rate_from_index,
    .set_state = system_pll_set_state,
    .get_state = system_pll_get_state,
    .get_range = system_pll_get_range,
    .process_power_transition = system_pll_power_state_change,
    .process_pending_power_transition = system_pll_power_state_pending_change,
};

/*
 * Framework handler functions
 */

static int system_pll_init(fwk_id_t module_id, unsigned int element_count,
                             const void *data)
{
    module_ctx.dev_count = element_count;

    if (element_count == 0)
        return FWK_SUCCESS;

    module_ctx.dev_ctx_table = fwk_mm_calloc(element_count,
                                             sizeof(struct system_pll_dev_ctx));
    return FWK_SUCCESS;
}

static int system_pll_element_init(fwk_id_t element_id, unsigned int unused,
                                  const void *data)
{
    struct system_pll_dev_ctx *ctx;
    const struct mod_system_pll_dev_config *dev_config = data;

    if (!fwk_module_is_valid_element_id(element_id))
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(element_id);

    ctx->config = dev_config;

    if (ctx->config->defer_initialization)
        return FWK_SUCCESS;

    ctx->initialized = true;
    ctx->current_state = MOD_CLOCK_STATE_RUNNING;
    return system_pll_set_rate(element_id, ctx->config->initial_rate,
                                MOD_CLOCK_ROUND_MODE_NONE);
}

static int system_pll_process_bind_request(fwk_id_t requester_id, fwk_id_t id,
                                        fwk_id_t api_type, const void **api)
{
    *api = &api_system_pll;
    return FWK_SUCCESS;
}

const struct fwk_module module_system_pll = {
    .name = "SYSTEM PLL Driver",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_SYSTEM_PLL_API_COUNT,
    .event_count = 0,
    .init = system_pll_init,
    .element_init = system_pll_element_init,
    .process_bind_request = system_pll_process_bind_request,
};

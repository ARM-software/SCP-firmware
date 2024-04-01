/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Silicon Creations PLL driver
 */

#include <internal/sc_pll.h>

#include <mod_clock.h>
#include <mod_power_domain.h>
#include <mod_sc_pll.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>

#define FREQ_TOLERANCE_HZ 10000

/* Device context */
struct sc_pll_dev_ctx {
    /* Configuration data of the PLL instance */
    const struct mod_sc_pll_dev_config *config;

    /* Current state of the PLL */
    enum mod_clock_state current_state;

    /* Current frequency output of the PLL */
    uint64_t current_rate;

    /* Initialization state of the PLL */
    bool initialized;
};

/* Module context */
struct sc_pll_ctx {
    /* Pointer to module configuration data */
    const struct sc_pll_module_config *mod_config;

    /* List of device contexts of all PLLs */
    struct sc_pll_dev_ctx *dev_ctx_table;

    /* Number of PLL instances */
    unsigned int dev_count;
};

static struct sc_pll_ctx module_ctx;

static inline struct sc_pll_dev_ctx *get_ctx(fwk_id_t dev_id)
{
    return module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
}

static inline bool check_fbdiv_within_range(uint16_t fbdiv_d)
{
    return (
        (fbdiv_d >= MOD_SC_PLL_FBDIV_MIN) && (fbdiv_d <= MOD_SC_PLL_FBDIV_MAX));
}
/*
 * Write the calculated PLL parameters to control register
 */
static int pll_write(
    struct sc_pll_dev_ctx *ctx,
    uint8_t postdiv1,
    uint8_t postdiv2,
    uint8_t refdiv,
    uint16_t fbdiv_d,
    uint64_t rate)
{
    const struct mod_sc_pll_dev_config *config;
    uint32_t wait_cycles;

    config = ctx->config;

    /* Configure PLL settings */
    *config->control_reg0 =
        (fbdiv_d << PLL_FBDIV_BIT_POS) | (refdiv << PLL_REFDIV_POS);
    *config->control_reg1 = (*config->control_reg1) |
        (postdiv1 << PLL_POSTDIV1_POS) | (postdiv2 << PLL_POSTDIV2_POS);

    /* Enable PLL settings */
    *config->control_reg0 |= (UINT32_C(1) << PLL_PLLEN_POS);

    wait_cycles = MOD_SC_PLL_LOCK_TIMEOUT;
    /* Wait until the PLL has locked */
    while (
        ((*config->control_reg1 & (UINT32_C(1) << PLL_LOCK_STATUS_POS)) == 0) &&
        (wait_cycles > 0)) {
        wait_cycles--;
    }

    if (wait_cycles == 0) {
        return FWK_E_TIMEOUT;
    }
    /* Store the current configured PLL rate */
    ctx->current_rate = rate;

    return FWK_SUCCESS;
}

/*
 * Pinpoint postdiv1 & 2, then find the refdiv and fbdiv within the valid
 * threshold
 */
static int pll_calc_fbdiv(
    struct sc_pll_dev_ctx *ctx,
    uint64_t rate,
    uint8_t postdiv1,
    uint8_t postdiv2)
{
    float fbdiv_f, fvco, fout;
    uint16_t fbdiv_d;
    uint8_t refdiv;
    int64_t diff;
    const struct mod_sc_pll_dev_config *config = NULL;

    config = ctx->config;

    /* Check if REF input is valid range (2MHz - 1200MHz) */
    if (config->ref_rate < MOD_SC_PLL_REF_MIN ||
        config->ref_rate > MOD_SC_PLL_REF_MAX) {
        return FWK_E_SUPPORT;
    }

    for (refdiv = MOD_SC_PLL_REFDIV_MIN; refdiv <= MOD_SC_PLL_REFDIV_MAX;
         refdiv++) {
        fbdiv_f = (float)(rate * postdiv1 * postdiv2 * refdiv) /
            (float)config->ref_rate;
        fbdiv_d = (uint16_t)fbdiv_f;

        /* Round fbdiv_d to nearest integer */
        fbdiv_d = ((fbdiv_f - (float)fbdiv_d) < 0.5) ? fbdiv_d : fbdiv_d + 1;

        /* Check if fbdiv_d is in valid range */
        if ((fbdiv_d < MOD_SC_PLL_FBDIV_MIN) ||
            (fbdiv_d > MOD_SC_PLL_FBDIV_MAX)) {
            continue;
        }

        fvco = (((float)config->ref_rate / (float)refdiv) * (float)fbdiv_d);
        /* Check if VCO output is in valid range */
        if ((fvco < (float)MOD_SC_PLL_FVCO_MIN) ||
            (fvco > (float)MOD_SC_PLL_FVCO_MAX)) {
            continue;
        }

        fout = (fvco / (float)postdiv1) / (float)postdiv2;
        diff = (uint64_t)fout - rate;
        if (diff < 0) {
            diff = diff * -1;
        }

        if (diff <= FREQ_TOLERANCE_HZ) {
            return pll_write(ctx, postdiv1, postdiv2, refdiv, fbdiv_d, rate);
        }
    }
    return FWK_E_SUPPORT;
}

/*
 * Calculate the Post Divider and Feedback
 */
static int pll_calc_post_divider_and_feedback_params(
    struct sc_pll_dev_ctx *ctx,
    uint64_t rate,
    uint8_t postdiv2)
{
    uint8_t postdiv1;
    int status;
    const struct mod_sc_pll_dev_config *config = NULL;

    config = ctx->config;

    for (postdiv1 = config->dev_param->postdiv1_min;
         postdiv1 <= config->dev_param->postdiv1_max;
         postdiv1++) {
        status = pll_calc_fbdiv(ctx, rate, postdiv1, postdiv2);
        if ((status == FWK_SUCCESS) || (status == FWK_E_TIMEOUT)) {
            return status;
        }
    }
    return FWK_E_SUPPORT;
}

/*
 * Pinpoint the first postdiv and iterate
 */
static int pll_fractional_calc(struct sc_pll_dev_ctx *ctx, uint64_t rate)
{
    uint8_t postdiv2;
    int status;
    const struct mod_sc_pll_dev_config *config = NULL;

    config = ctx->config;

    for (postdiv2 = config->dev_param->postdiv2_min;
         postdiv2 <= config->dev_param->postdiv2_max;
         postdiv2++) {
        status = pll_calc_post_divider_and_feedback_params(ctx, rate, postdiv2);
        if ((status == FWK_SUCCESS) || (status == FWK_E_TIMEOUT)) {
            return status;
        }
    }

    return FWK_E_SUPPORT;
}

/*
 * PLL rate configuration function
 */
static int pll_set_rate(struct sc_pll_dev_ctx *ctx, uint64_t rate)
{
    uint16_t fbdiv_d;
    uint8_t postdiv1, postdiv2, refdiv;
    const struct mod_sc_pll_dev_config *config = NULL;

    if (ctx == NULL) {
        return FWK_E_PARAM;
    }

    if (rate >= ((uint64_t)UINT16_MAX * FWK_MHZ)) {
        return FWK_E_PARAM;
    }

    if (ctx->current_state == MOD_CLOCK_STATE_STOPPED) {
        return FWK_E_PWRSTATE;
    }

    config = ctx->config;

    if ((rate < config->dev_param->pll_rate_min) ||
        (rate > config->dev_param->pll_rate_max)) {
        return FWK_E_RANGE;
    }

    /*
     * If requested frequency is integer multiple of reference frequency
     * then the fbdiv_d calculation is a simple division provided it is
     * within the valid range.
     */
    if ((rate % config->ref_rate) == 0) {
        fbdiv_d = rate / config->ref_rate;
        if (check_fbdiv_within_range(fbdiv_d)) {
            refdiv = 1;
            postdiv1 = 1;
            postdiv2 = 1;
            return pll_write(ctx, postdiv1, postdiv2, refdiv, fbdiv_d, rate);
        }
    }

    /*
     * If requested frequency is not an integer multiple of reference frequency
     * then fine tune the parameters ( fbdiv, postdiv1, postdiv2, refdiv )
     * This values will then written to PLL control registers.
     */
    return pll_fractional_calc(ctx, rate);
}

/*
 * Clock driver API functions
 */

static int sc_pll_set_rate(
    fwk_id_t dev_id,
    uint64_t rate,
    enum mod_clock_round_mode unused)
{
    struct sc_pll_dev_ctx *ctx = NULL;

    if (!fwk_module_is_valid_element_id(dev_id)) {
        return FWK_E_PARAM;
    }

    ctx = get_ctx(dev_id);

    return pll_set_rate(ctx, rate);
}

static int sc_pll_get_rate(fwk_id_t dev_id, uint64_t *rate)
{
    struct sc_pll_dev_ctx *ctx = NULL;

    if ((!fwk_module_is_valid_element_id(dev_id)) || (rate == NULL)) {
        return FWK_E_PARAM;
    }

    ctx = get_ctx(dev_id);
    *rate = ctx->current_rate;

    return FWK_SUCCESS;
}

static int sc_pll_get_rate_from_index(
    fwk_id_t dev_id,
    unsigned int rate_index,
    uint64_t *rate)
{
    /* PLLs have a continuous range of rates and are not indexed */
    return FWK_E_SUPPORT;
}

static int sc_pll_set_state(fwk_id_t dev_id, enum mod_clock_state state)
{
    if (state == MOD_CLOCK_STATE_RUNNING) {
        return FWK_SUCCESS;
    }

    /* PLLs can only be stopped by a parent power domain state change. */
    return FWK_E_SUPPORT;
}

static int sc_pll_get_state(fwk_id_t dev_id, enum mod_clock_state *state)
{
    struct sc_pll_dev_ctx *ctx = NULL;

    if ((!fwk_module_is_valid_element_id(dev_id)) || (state == NULL)) {
        return FWK_E_PARAM;
    }

    ctx = get_ctx(dev_id);
    *state = ctx->current_state;

    return FWK_SUCCESS;
}

static int sc_pll_get_range(fwk_id_t dev_id, struct mod_clock_range *range)
{
    struct sc_pll_dev_ctx *ctx = NULL;
    const struct mod_sc_pll_dev_config *config = NULL;

    if ((!fwk_module_is_valid_element_id(dev_id)) || (range == NULL)) {
        return FWK_E_PARAM;
    }

    ctx = get_ctx(dev_id);
    config = ctx->config;

    range->rate_type = MOD_CLOCK_RATE_TYPE_CONTINUOUS;
    range->min = config->dev_param->pll_rate_min;
    range->max = config->dev_param->pll_rate_max;
    range->step = MOD_SC_PLL_STEP_SIZE;

    return FWK_SUCCESS;
}

static int sc_pll_power_state_change(fwk_id_t dev_id, unsigned int state)
{
    uint64_t rate;
    struct sc_pll_dev_ctx *ctx = NULL;

    if (!fwk_module_is_valid_element_id(dev_id)) {
        return FWK_E_PARAM;
    }

    ctx = get_ctx(dev_id);

    if (state != MOD_PD_STATE_ON) {
        return FWK_SUCCESS;
    }

    ctx->current_state = MOD_CLOCK_STATE_RUNNING;

    if (ctx->initialized) {
        /* Restore the previous rate */
        rate = ctx->current_rate;
    } else {
        /* Initialize the PLL to its default rate */
        ctx->initialized = true;
        rate = ctx->config->initial_rate;
    }

    return sc_pll_set_rate(dev_id, rate, MOD_CLOCK_ROUND_MODE_NONE);
}

static int sc_pll_power_state_pending_change(
    fwk_id_t dev_id,
    unsigned int current_state,
    unsigned int next_state)
{
    struct sc_pll_dev_ctx *ctx = NULL;

    if (!fwk_module_is_valid_element_id(dev_id)) {
        return FWK_E_PARAM;
    }

    ctx = get_ctx(dev_id);

    if (next_state == MOD_PD_STATE_OFF) {
        /* Just mark the PLL as stopped */
        ctx->current_state = MOD_CLOCK_STATE_STOPPED;
    }

    return FWK_SUCCESS;
}

static const struct mod_clock_drv_api sc_pll_api = {
    .set_rate = sc_pll_set_rate,
    .get_rate = sc_pll_get_rate,
    .get_rate_from_index = sc_pll_get_rate_from_index,
    .set_state = sc_pll_set_state,
    .get_state = sc_pll_get_state,
    .get_range = sc_pll_get_range,
    .process_power_transition = sc_pll_power_state_change,
    .process_pending_power_transition = sc_pll_power_state_pending_change,
};

/*
 * Framework handler functions
 */

static int sc_pll_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (element_count == 0) {
        return FWK_E_PARAM;
    }

    FWK_LOG_DEBUG("[SC PLL] Initializing ");

    module_ctx.dev_count = element_count;

    module_ctx.dev_ctx_table =
        fwk_mm_calloc(element_count, sizeof(struct sc_pll_dev_ctx));

    return FWK_SUCCESS;
}

static int sc_pll_element_init(
    fwk_id_t dev_id,
    unsigned int unused,
    const void *data)
{
    struct sc_pll_dev_ctx *ctx;

    if (data == NULL) {
        return FWK_E_DATA;
    }

    ctx = get_ctx(dev_id);

    ctx->config = (struct mod_sc_pll_dev_config *)data;

    /* Check for valid element configuration data */
    if ((ctx->config->control_reg0 == NULL) ||
        (ctx->config->control_reg1 == NULL) || (ctx->config->ref_rate == 0)) {
        return FWK_E_PARAM;
    }

    ctx->initialized = true;
    ctx->current_state = MOD_CLOCK_STATE_RUNNING;
    return pll_set_rate(ctx, ctx->config->initial_rate);
}

static int sc_pll_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_E_ACCESS;
    }

    if (fwk_id_is_equal(api_id, mod_sc_pll_api_id_pll)) {
        *api = &sc_pll_api;
    } else {
        return FWK_E_PARAM;
    }
    return FWK_SUCCESS;
}

const struct fwk_module module_sc_pll = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_SC_PLL_API_COUNT,
    .event_count = 0,
    .init = sc_pll_init,
    .element_init = sc_pll_element_init,
    .process_bind_request = sc_pll_process_bind_request,
};

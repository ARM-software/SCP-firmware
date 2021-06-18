/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/morello_pll.h>

#include <mod_clock.h>
#include <mod_morello_pll.h>
#include <mod_power_domain.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>

#define FREQ_TOLERANCE_HZ 10000

/* Device context */
struct morello_pll_dev_ctx {
    /* Configuration data of the PLL instance */
    const struct mod_morello_pll_dev_config *config;

    /* Current state of the PLL */
    enum mod_clock_state current_state;

    /* Current frequency output of the PLL */
    uint64_t current_rate;

    /* Initialization state of the PLL */
    bool initialized;
};

/* Module context */
struct morello_pll_ctx {
    /* Pointer to module configuration data */
    const struct morello_pll_module_config *mod_config;

    /* List of device contexts of all PLLs */
    struct morello_pll_dev_ctx *dev_ctx_table;

    /* Number of PLL instances */
    unsigned int dev_count;
};

static struct morello_pll_ctx module_ctx;

/*
 * PLL rate configuration function
 */

static int pll_set_rate(
    struct morello_pll_dev_ctx *ctx,
    uint64_t rate,
    enum mod_clock_round_mode unused)
{
    float fbdiv_f, fvco, fout;
    uint16_t fbdiv_d;
    uint8_t postdiv1, postdiv2, refdiv;
    int64_t diff;
    uint32_t wait_cycles;
    const struct mod_morello_pll_dev_config *config = NULL;

    fwk_assert(ctx != NULL);
    fwk_assert(rate <= ((uint64_t)UINT16_MAX * FWK_MHZ));

    config = ctx->config;

    if (ctx->current_state == MOD_CLOCK_STATE_STOPPED) {
        return FWK_E_PWRSTATE;
    }

    if ((rate < MOD_MORELLO_PLL_RATE_MIN) ||
        (rate > MOD_MORELLO_PLL_RATE_MAX)) {
        return FWK_E_RANGE;
    }

    /*
     * If requested frequency is integer multiple of reference frequency
     * then the fbdiv_d calculation is a simple division provided it is
     * within the valid range.
     */
    if ((rate % config->ref_rate) == 0) {
        fbdiv_d = rate / config->ref_rate;
        if ((fbdiv_d >= MOD_MORELLO_PLL_FBDIV_MIN) &&
            (fbdiv_d <= MOD_MORELLO_PLL_FBDIV_MAX)) {
            refdiv = 1;
            postdiv1 = 1;
            postdiv2 = 1;
            goto write_pll_params;
        }
    }

    for (postdiv2 = MOD_MORELLO_PLL_POSTDIV_MIN;
         postdiv2 <= MOD_MORELLO_PLL_POSTDIV_MAX;
         postdiv2++) {
        for (postdiv1 = MOD_MORELLO_PLL_POSTDIV_MIN;
             postdiv1 <= MOD_MORELLO_PLL_POSTDIV_MAX;
             postdiv1++) {
            for (refdiv = MOD_MORELLO_PLL_REFDIV_MIN;
                 refdiv <= MOD_MORELLO_PLL_REFDIV_MAX;
                 refdiv++) {
                /* Check if REF input is valid */
                if (((float)config->ref_rate / (float)refdiv) <
                    (float)MOD_MORELLO_PLL_REF_MIN) {
                    continue;
                }

                fbdiv_f = (float)(rate * postdiv1 * postdiv2 * refdiv) /
                    (float)config->ref_rate;
                fbdiv_d = (uint16_t)fbdiv_f;

                /* Round fbdiv_d to nearest integer */
                fbdiv_d =
                    ((fbdiv_f - (float)fbdiv_d) < 0.5) ? fbdiv_d : fbdiv_d + 1;

                /* Check if fbdiv_d is in valid range */
                if ((fbdiv_d < MOD_MORELLO_PLL_FBDIV_MIN) ||
                    (fbdiv_d > MOD_MORELLO_PLL_FBDIV_MAX)) {
                    continue;
                }

                fvco =
                    (((float)config->ref_rate / (float)refdiv) *
                     (float)fbdiv_d);
                /* Check if VCO output is in valid range */
                if ((fvco < (float)MOD_MORELLO_PLL_FVCO_MIN) ||
                    (fvco > (float)MOD_MORELLO_PLL_FVCO_MAX)) {
                    continue;
                }

                fout = fvco / (float)postdiv1 / (float)postdiv2;
                diff = (uint64_t)fout - rate;
                if (diff < 0) {
                    diff = diff * -1;
                }

                if (diff <= FREQ_TOLERANCE_HZ) {
                    goto write_pll_params;
                }
            }
        }
    }
    /* We have reached end of loop without valid parameters */
    return FWK_E_SUPPORT;

write_pll_params:
    /* Configure PLL settings */
    *config->control_reg0 =
        (fbdiv_d << PLL_FBDIV_BIT_POS) | (refdiv << PLL_REFDIV_POS);
    *config->control_reg1 =
        (postdiv1 << PLL_POSTDIV1_POS) | (postdiv2 << PLL_POSTDIV2_POS);

    /* Enable PLL settings */
    *config->control_reg0 |= (UINT32_C(1) << PLL_PLLEN_POS);

    wait_cycles = MOD_MORELLO_PLL_LOCK_TIMEOUT;
    /* Wait until the PLL has locked */
    while ((*config->control_reg1 & (UINT32_C(1) << PLL_LOCK_STATUS_POS)) ==
           0) {
        wait_cycles--;
        if (wait_cycles == 0) {
            return FWK_E_TIMEOUT;
        }
    }

    /* Store the current configured PLL rate */
    ctx->current_rate = rate;

    return FWK_SUCCESS;
}

/*
 * Clock driver API functions
 */

static int morello_pll_set_rate(
    fwk_id_t dev_id,
    uint64_t rate,
    enum mod_clock_round_mode unused)
{
    struct morello_pll_dev_ctx *ctx = NULL;

    if (!fwk_module_is_valid_element_id(dev_id)) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    return pll_set_rate(ctx, rate, unused);
}

static int morello_pll_get_rate(fwk_id_t dev_id, uint64_t *rate)
{
    struct morello_pll_dev_ctx *ctx = NULL;

    if ((!fwk_module_is_valid_element_id(dev_id)) || (rate == NULL)) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
    *rate = ctx->current_rate;

    return FWK_SUCCESS;
}

static int morello_pll_get_rate_from_index(
    fwk_id_t dev_id,
    unsigned int rate_index,
    uint64_t *rate)
{
    /* PLLs have a continuous range of rates and are not indexed */
    return FWK_E_SUPPORT;
}

static int morello_pll_set_state(fwk_id_t dev_id, enum mod_clock_state state)
{
    if (state == MOD_CLOCK_STATE_RUNNING) {
        return FWK_SUCCESS;
    }

    /* PLLs can only be stopped by a parent power domain state change. */
    return FWK_E_SUPPORT;
}

static int morello_pll_get_state(fwk_id_t dev_id, enum mod_clock_state *state)
{
    struct morello_pll_dev_ctx *ctx = NULL;

    if ((!fwk_module_is_valid_element_id(dev_id)) || (state == NULL)) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
    *state = ctx->current_state;

    return FWK_SUCCESS;
}

static int morello_pll_get_range(fwk_id_t dev_id, struct mod_clock_range *range)
{
    if ((!fwk_module_is_valid_element_id(dev_id)) || (range == NULL)) {
        return FWK_E_PARAM;
    }

    range->rate_type = MOD_CLOCK_RATE_TYPE_CONTINUOUS;
    range->min = MOD_MORELLO_PLL_RATE_MIN;
    range->max = MOD_MORELLO_PLL_RATE_MAX;
    range->step = MOD_MORELLO_PLL_STEP_SIZE;

    return FWK_SUCCESS;
}

static int morello_pll_power_state_change(fwk_id_t dev_id, unsigned int state)
{
    uint64_t rate;
    struct morello_pll_dev_ctx *ctx = NULL;

    if (!fwk_module_is_valid_element_id(dev_id)) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

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

    return morello_pll_set_rate(dev_id, rate, MOD_CLOCK_ROUND_MODE_NONE);
}

static int morello_pll_power_state_pending_change(
    fwk_id_t dev_id,
    unsigned int current_state,
    unsigned int next_state)
{
    struct morello_pll_dev_ctx *ctx = NULL;

    if (!fwk_module_is_valid_element_id(dev_id)) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (next_state == MOD_PD_STATE_OFF) {
        /* Just mark the PLL as stopped */
        ctx->current_state = MOD_CLOCK_STATE_STOPPED;
    }

    return FWK_SUCCESS;
}

static const struct mod_clock_drv_api morello_pll_api = {
    .set_rate = morello_pll_set_rate,
    .get_rate = morello_pll_get_rate,
    .get_rate_from_index = morello_pll_get_rate_from_index,
    .set_state = morello_pll_set_state,
    .get_state = morello_pll_get_state,
    .get_range = morello_pll_get_range,
    .process_power_transition = morello_pll_power_state_change,
    .process_pending_power_transition = morello_pll_power_state_pending_change,
};

/*
 * Framework handler functions
 */

static int morello_pll_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (element_count == 0) {
        return FWK_E_PARAM;
    }

    module_ctx.dev_count = element_count;

    module_ctx.dev_ctx_table =
        fwk_mm_calloc(element_count, sizeof(struct morello_pll_dev_ctx));
    if (module_ctx.dev_ctx_table == NULL) {
        return FWK_E_NOMEM;
    }

    return FWK_SUCCESS;
}

static int morello_pll_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    struct morello_pll_dev_ctx *ctx = NULL;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(element_id);

    ctx->config = (struct mod_morello_pll_dev_config *)data;

    /* Check for valid element configuration data */
    if ((ctx->config->control_reg0 == NULL) ||
        (ctx->config->control_reg1 == NULL) || (ctx->config->ref_rate == 0)) {
        return FWK_E_PARAM;
    }

    if (ctx->config->defer_initialization) {
        return FWK_SUCCESS;
    }

    ctx->initialized = true;
    ctx->current_state = MOD_CLOCK_STATE_RUNNING;
    return pll_set_rate(
        ctx, ctx->config->initial_rate, MOD_CLOCK_ROUND_MODE_NONE);
}

static int morello_pll_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t id,
    fwk_id_t api_type,
    const void **api)
{
    *api = &morello_pll_api;
    return FWK_SUCCESS;
}

const struct fwk_module module_morello_pll = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_MORELLO_PLL_API_COUNT,
    .event_count = 0,
    .init = morello_pll_init,
    .element_init = morello_pll_element_init,
    .process_bind_request = morello_pll_process_bind_request,
};

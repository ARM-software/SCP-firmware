/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_clock.h>
#include <mod_power_domain.h>
#include <mod_synquacer_pik_clock.h>

#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdint.h>
#include <stdlib.h>

/*
 * Masks for single-source clock divider control.
 */
#define SSCLK_CONTROL_CLKDIV UINT32_C(0x000000F0)
#define SSCLK_CONTROL_CRNTCLKDIV UINT32_C(0x0000F000)
#define SSCLK_CONTROL_ENTRY_DLY UINT32_C(0xFF000000)

/*
 * Offsets for single-source clock divider control.
 */
#define SSCLK_CONTROL_CLKDIV_POS 4
#define SSCLK_CONTROL_CRNTCLKDIV_POS 12
#define SSCLK_CONTROL_ENTRY_DLY_POS 24

/*
 * Masks for multi-source clock divider control (both DIV1 and DIV2).
 */
#define MSCLK_DIV_CLKDIV UINT32_C(0x0000001F)
#define MSCLK_DIV_CRNTCLKDIV UINT32_C(0x001F0000)
#define MSCLK_DIV_ENTRY_DLY UINT32_C(0xFF000000)

/*
 * Offsets for multi-source clock divider control (both DIV1 and DIV2).
 */
#define MSCLK_DIV_CLKDIV_POS 0
#define MSCLK_DIV_CRNTCLKDIV_POS 16
#define MSCLK_DIV_ENTRY_DLY_POS 24

/*
 * Masks for multi-source clock source selection.
 */
#define MSCLK_CONTROL_CLKSEL UINT32_C(0x000000FF)
#define MSCLK_CONTROL_CLKSEL_GATED UINT32_C(0x00000000)
#define MSCLK_CONTROL_CLKSEL_SYSREFCLK UINT32_C(0x00000001)
#define MSCLK_CONTROL_CLKSEL_SYSPLLCLK UINT32_C(0x00000002)
#define MSCLK_CONTROL_CLKSEL_PRIVPLLCLK UINT32_C(0x00000004)
#define MSCLK_CONTROL_CRNTCLK UINT32_C(0x0000FF00)

/*
 * Offsets for multi-source clock source selection.
 */
#define MSCLK_CONTROL_CLKSEL_POS 0
#define MSCLK_CONTROL_CRNTCLK_POS 8

/*
 * Masks for cluster clock source selection.
 *
 * Note: The CLKSEL field mask is shared with the multi-source clock subtype, as
 * are the masks for the GATED and REFCLK sources.
 */
#define CLUSCLK_CONTROL_CLKSEL_PLL0 UINT32_C(0x00000002)
#define CLUSCLK_CONTROL_CLKSEL_PLL1 UINT32_C(0x00000003)
#define CLUSCLK_CONTROL_CLKSEL_PLL2 UINT32_C(0x00000004)
#define CLUSCLK_CONTROL_CLKSEL_PLL3 UINT32_C(0x00000005)
#define CLUSCLK_CONTROL_CLKSEL_PLL4 UINT32_C(0x00000006)
#define CLUSCLK_CONTROL_CLKSEL_PLL5 UINT32_C(0x00000007)
#define CLUSCLK_CONTROL_CLKSEL_PLL6 UINT32_C(0x00000008)
#define CLUSCLK_CONTROL_CLKSEL_PLL7 UINT32_C(0x00000009)
#define CLUSCLK_CONTROL_CLKSEL_SYSPLLCLK UINT32_C(0x0000000A)

/*
 * Masks for cluster clock modulator control.
 */
#define CLUSCLK_MOD_DENOMINATOR UINT32_C(0x000000FF)
#define CLUSCLK_MOD_NUMERATOR UINT32_C(0x0000FF00)
#define CLUSCLK_MOD_CRNTDENOMINATOR UINT32_C(0x00FF0000)
#define CLUSCLK_MOD_CRNTNUMERATOR UINT32_C(0xFF000000)

/*
 * Offsets for cluster clock modulator control.
 */
#define CLUSCLK_MOD_DENOMINATOR_POS 0
#define CLUSCLK_MOD_NUMERATOR_POS 8
#define CLUSCLK_MOD_CRNTDENOMINATOR_POS 16
#define CLUSCLK_MOD_CRNTNUMERATOR_POS 24

/* Device context */
struct pik_clock_dev_ctx {
    bool initialized;
    uint64_t current_rate;
    uint8_t current_source;
    enum mod_clock_state current_state;
    const struct mod_pik_clock_dev_config *config;
};

/* Module context */
struct pik_clock_ctx {
    struct pik_clock_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
    unsigned int divider_max;
};

static struct pik_clock_ctx module_ctx;

/*
 * Static helper functions
 */

static int compare_rate_entry(const void *a, const void *b)
{
    struct mod_pik_clock_rate *key = (struct mod_pik_clock_rate *)a;
    struct mod_pik_clock_rate *element = (struct mod_pik_clock_rate *)b;

    return (key->rate - element->rate);
}

static int get_rate_entry(
    struct pik_clock_dev_ctx *ctx,
    uint64_t target_rate,
    struct mod_pik_clock_rate **entry)
{
    struct mod_pik_clock_rate *current_rate_entry;

    if (ctx == NULL)
        return FWK_E_PARAM;
    if (entry == NULL)
        return FWK_E_PARAM;

    /* Perform a binary search to find the entry matching the requested rate */
    current_rate_entry = (struct mod_pik_clock_rate *)bsearch(
        &target_rate,
        ctx->config->rate_table,
        ctx->config->rate_count,
        sizeof(struct mod_pik_clock_rate),
        compare_rate_entry);

    if (current_rate_entry == NULL)
        return FWK_E_PARAM;

    *entry = current_rate_entry;
    return FWK_SUCCESS;
}

static int ssclock_set_div(
    struct pik_clock_dev_ctx *ctx,
    uint32_t divider,
    bool wait_after_set)
{
    uint32_t clkdiv;

    if (divider == 0)
        return FWK_E_PARAM;
    if (divider > module_ctx.divider_max)
        return FWK_E_PARAM;
    if (ctx == NULL)
        return FWK_E_PARAM;
    if (ctx->config->type != MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE)
        return FWK_E_PARAM;

    /* The resulting divider is the programmed value plus one */
    clkdiv = divider - 1;

    /* Set */
    *ctx->config->control_reg =
        (*ctx->config->control_reg & ~SSCLK_CONTROL_CLKDIV) |
        (clkdiv << SSCLK_CONTROL_CLKDIV_POS);

    if (wait_after_set) {
        while ((*ctx->config->control_reg & SSCLK_CONTROL_CRNTCLKDIV) !=
               (clkdiv << SSCLK_CONTROL_CRNTCLKDIV_POS))
            continue;
    }

    return FWK_SUCCESS;
}

static int msclock_set_div(
    struct pik_clock_dev_ctx *ctx,
    enum mod_pik_clock_msclock_divider divider_type,
    uint32_t divider,
    bool wait_after_set)
{
    volatile uint32_t *divider_reg;
    uint32_t clkdiv;

    if (ctx == NULL)
        return FWK_E_PARAM;
    if (divider == 0)
        return FWK_E_PARAM;
    if (divider > module_ctx.divider_max)
        return FWK_E_PARAM;
    if (ctx->config->type == MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE)
        return FWK_E_PARAM;

    /* The resulting divider is the programmed value plus one */
    clkdiv = divider - 1;

    if (divider_type == MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_SYS)
        divider_reg = ctx->config->divsys_reg;
    else if (divider_type == MOD_PIK_CLOCK_MSCLOCK_DIVIDER_DIV_EXT)
        divider_reg = ctx->config->divext_reg;
    else
        return FWK_E_PARAM;

    /* Set */
    *divider_reg =
        (*divider_reg & ~MSCLK_DIV_CLKDIV) | (clkdiv << MSCLK_DIV_CLKDIV_POS);

    if (wait_after_set) {
        while ((*divider_reg & MSCLK_DIV_CRNTCLKDIV) !=
               (clkdiv << MSCLK_DIV_CRNTCLKDIV_POS))
            continue;
    }

    return FWK_SUCCESS;
}

static int msclock_set_source(
    struct pik_clock_dev_ctx *ctx,
    uint32_t source,
    bool wait_after_set)
{
    if (ctx == NULL)
        return FWK_E_PARAM;
    if (ctx->config->type == MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE)
        return FWK_E_PARAM;

    /* Set */
    *ctx->config->control_reg =
        (*ctx->config->control_reg & ~MSCLK_CONTROL_CLKSEL) |
        (source << MSCLK_CONTROL_CLKSEL_POS);

    if (wait_after_set) {
        while ((*ctx->config->control_reg & MSCLK_CONTROL_CRNTCLK) !=
               ((uint32_t)(source << MSCLK_CONTROL_CRNTCLK_POS)))
            continue;
    }

    return FWK_SUCCESS;
}

static int cluster_set_modulator(
    struct pik_clock_dev_ctx *ctx,
    uint32_t numerator,
    uint32_t denominator,
    bool wait_after_set)
{
    uint32_t modulator_setting;

    if (ctx == NULL)
        return FWK_E_PARAM;
    if (ctx->config->type != MOD_PIK_CLOCK_TYPE_CLUSTER)
        return FWK_E_PARAM;
    if (ctx->config->modulator_reg == NULL)
        return FWK_E_PARAM;
    if (denominator > 255)
        return FWK_E_PARAM;
    if (denominator == 0)
        return FWK_E_PARAM;
    if (numerator > 255)
        return FWK_E_PARAM;

    modulator_setting = (denominator << CLUSCLK_MOD_DENOMINATOR_POS) |
        (numerator << CLUSCLK_MOD_NUMERATOR_POS);

    *ctx->config->modulator_reg =
        ((*ctx->config->modulator_reg &
          ~(CLUSCLK_MOD_DENOMINATOR | CLUSCLK_MOD_NUMERATOR)) |
         modulator_setting);

    if (wait_after_set) {
        while ((*ctx->config->modulator_reg & CLUSCLK_MOD_CRNTNUMERATOR) !=
               (numerator << CLUSCLK_MOD_CRNTNUMERATOR_POS))
            continue;

        while ((*ctx->config->modulator_reg & CLUSCLK_MOD_CRNTDENOMINATOR) !=
               (denominator << CLUSCLK_MOD_CRNTDENOMINATOR_POS))
            continue;
    }

    return FWK_SUCCESS;
}

static int do_pik_clock_set_rate(
    fwk_id_t dev_id,
    uint64_t rate,
    enum mod_clock_round_mode round_mode)
{
    int status;
    struct pik_clock_dev_ctx *ctx;
    struct mod_pik_clock_rate *rate_entry;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    /* Look up the divider and source settings */
    status = get_rate_entry(ctx, rate, &rate_entry);
    if (status != FWK_SUCCESS)
        return status;

    switch (ctx->config->type) {
    case MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE:
        status = ssclock_set_div(ctx, rate_entry->divider, false);
        if (status != FWK_SUCCESS)
            goto exit;
        break;
    case MOD_PIK_CLOCK_TYPE_CLUSTER:
        /* Modulator feature not currently used */
        cluster_set_modulator(ctx, 1, 1, false);
        /* Intentional fall-through */
    case MOD_PIK_CLOCK_TYPE_MULTI_SOURCE:
        if (ctx->current_source == MOD_PIK_CLOCK_MSCLOCK_SOURCE_GATED)
            /* Leave the new rate to be applied when the clock is (re)started */
            goto exit;

        status = msclock_set_div(
            ctx, rate_entry->divider_reg, rate_entry->divider, false);
        if (status != FWK_SUCCESS)
            goto exit;

        status = msclock_set_source(ctx, rate_entry->source, false);
        if (status != FWK_SUCCESS)
            goto exit;

        ctx->current_source = rate_entry->source;
        break;
    default:
        return FWK_E_SUPPORT;
    }

exit:
    if (status == FWK_SUCCESS)
        ctx->current_rate = rate;
    return status;
}

/*
 * Clock driver API functions
 */

static int pik_clock_set_rate(
    fwk_id_t dev_id,
    uint64_t rate,
    enum mod_clock_round_mode round_mode)
{
    struct pik_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (!ctx->initialized)
        return FWK_E_INIT;

    if (ctx->current_state == MOD_CLOCK_STATE_STOPPED)
        return FWK_E_PWRSTATE;

    return do_pik_clock_set_rate(dev_id, rate, round_mode);
}

static int pik_clock_get_rate(fwk_id_t dev_id, uint64_t *rate)
{
    struct pik_clock_dev_ctx *ctx;

    if (rate == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (ctx->config->type == MOD_PIK_CLOCK_TYPE_MULTI_SOURCE &&
        ctx->current_source == MOD_PIK_CLOCK_MSCLOCK_SOURCE_GATED)
        /* Indicate that the clock is not running */
        *rate = 0;
    else
        *rate = ctx->current_rate;

    return FWK_SUCCESS;
}

static int pik_clock_get_rate_from_index(
    fwk_id_t dev_id,
    unsigned int rate_index,
    uint64_t *rate)
{
    struct pik_clock_dev_ctx *ctx;

    if (rate == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (rate_index >= ctx->config->rate_count)
        return FWK_E_PARAM;

    *rate = ctx->config->rate_table[rate_index].rate;
    return FWK_SUCCESS;
}

static int pik_clock_set_state(
    fwk_id_t dev_id,
    enum mod_clock_state target_state)
{
    int status;
    struct pik_clock_dev_ctx *ctx;
    struct mod_pik_clock_rate *rate_entry;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (ctx->config->type == MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE)
        /* Cannot gate single-source clocks */
        return FWK_E_SUPPORT;

    if (!ctx->initialized)
        return FWK_E_INIT;

    if (ctx->current_state == MOD_CLOCK_STATE_STOPPED)
        /*
         * This state from the device context relates only to the clock state
         * that is derived from its parent power domain.
         */
        return FWK_E_PWRSTATE;

    if (target_state == MOD_CLOCK_STATE_STOPPED) {
        /* The clock is powered and will be gated. */
        status =
            msclock_set_source(ctx, MOD_PIK_CLOCK_MSCLOCK_SOURCE_GATED, false);
        if (status == FWK_SUCCESS)
            ctx->current_source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_GATED;

        return status;
    } else {
        /* Look up the divider and source settings */
        status = get_rate_entry(ctx, ctx->current_rate, &rate_entry);
        if (status != FWK_SUCCESS)
            return status;

        status = msclock_set_source(ctx, rate_entry->source, false);
        if (status == FWK_SUCCESS)
            ctx->current_source = rate_entry->source;

        return status;
    }
}

static int pik_clock_get_state(fwk_id_t dev_id, enum mod_clock_state *state)
{
    struct pik_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (ctx->config->type == MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE) {
        /*
         * Single-source clocks cannot be gated so their running state will be
         * derived purely from the state of their parent power domain, if any.
         */
        *state = ctx->current_state;
        return FWK_SUCCESS;
    }

    /*
     * Multi-source clocks may be stopped due to gating as well as the state of
     * their parent power domain.
     */
    if (ctx->current_source == MOD_PIK_CLOCK_MSCLOCK_SOURCE_GATED)
        *state = MOD_CLOCK_STATE_STOPPED;
    else
        *state = ctx->current_state;

    return FWK_SUCCESS;
}

static int pik_clock_get_range(fwk_id_t dev_id, struct mod_clock_range *range)
{
    struct pik_clock_dev_ctx *ctx;

    if (range == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    range->rate_type = MOD_CLOCK_RATE_TYPE_DISCRETE;
    range->min = ctx->config->rate_table[0].rate;
    range->max = ctx->config->rate_table[ctx->config->rate_count - 1].rate;
    range->rate_count = ctx->config->rate_count;

    return FWK_SUCCESS;
}

static int pik_clock_power_state_change(fwk_id_t dev_id, unsigned int state)
{
    struct pik_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (ctx->config->is_group_member)
        return FWK_E_ACCESS;

    if (state == MOD_PD_STATE_ON) {
        if (ctx->initialized)
            /* Restore the previous rate */
            return do_pik_clock_set_rate(
                dev_id, ctx->current_rate, MOD_CLOCK_ROUND_MODE_NONE);
        else {
            /* Perform deferred initialization and set the initial rate */
            ctx->current_state = MOD_CLOCK_STATE_RUNNING;
            ctx->initialized = true;
            return do_pik_clock_set_rate(
                dev_id, ctx->config->initial_rate, MOD_CLOCK_ROUND_MODE_NONE);
        }
    } else
        ctx->current_state = MOD_CLOCK_STATE_STOPPED;

    return FWK_SUCCESS;
}

static const struct mod_clock_drv_api api_clock = {
    .set_rate = pik_clock_set_rate,
    .get_rate = pik_clock_get_rate,
    .get_rate_from_index = pik_clock_get_rate_from_index,
    .set_state = pik_clock_set_state,
    .get_state = pik_clock_get_state,
    .get_range = pik_clock_get_range,
    .process_power_transition = pik_clock_power_state_change,
};

/*
 * Direct driver API functions
 */

#if BUILD_HAS_MOD_CSS_CLOCK
static int pik_clock_direct_set_div(
    fwk_id_t clock_id,
    uint32_t divider_type,
    uint32_t divider)
{
    int status;
    struct pik_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(clock_id);
    assert(ctx->config->is_group_member);

    if (ctx->current_state == MOD_CLOCK_STATE_STOPPED)
        return FWK_E_PWRSTATE;

    switch (ctx->config->type) {
    case MOD_PIK_CLOCK_TYPE_SINGLE_SOURCE:
        status = ssclock_set_div(ctx, divider, false);
        break;
    case MOD_PIK_CLOCK_TYPE_CLUSTER:
    case MOD_PIK_CLOCK_TYPE_MULTI_SOURCE:
        status = msclock_set_div(
            ctx,
            (enum mod_pik_clock_msclock_divider)divider_type,
            divider,
            false);
        break;
    default:
        return FWK_E_SUPPORT;
    }

    return status;
}

static int pik_clock_direct_set_source(fwk_id_t clock_id, uint8_t source)
{
    struct pik_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(clock_id);
    assert(ctx->config->is_group_member);

    if (ctx->current_state == MOD_CLOCK_STATE_STOPPED)
        return FWK_E_PWRSTATE;

    return msclock_set_source(ctx, source, false);
}

static int pik_clock_direct_set_mod(
    fwk_id_t clock_id,
    uint32_t numerator,
    uint32_t denominator)
{
    struct pik_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(clock_id);
    assert(ctx->config->is_group_member);

    if (ctx->current_state == MOD_CLOCK_STATE_STOPPED)
        return FWK_E_PWRSTATE;

    return cluster_set_modulator(ctx, numerator, denominator, false);
}

static int pik_clock_direct_power_state_change(
    fwk_id_t dev_id,
    unsigned int state)
{
    struct pik_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (!ctx->config->is_group_member)
        return FWK_E_ACCESS;

    if (state == MOD_PD_STATE_ON) {
        if (!ctx->initialized)
            /* Perform delayed initialization */
            ctx->initialized = true;
        ctx->current_state = MOD_CLOCK_STATE_RUNNING;
    } else
        ctx->current_state = MOD_CLOCK_STATE_STOPPED;

    return FWK_SUCCESS;
}

static const struct mod_css_clock_direct_api api_direct = {
    .set_div = pik_clock_direct_set_div,
    .set_source = pik_clock_direct_set_source,
    .set_mod = pik_clock_direct_set_mod,
    .process_power_transition = pik_clock_direct_power_state_change,
};
#endif

/*
 * Framework handler functions
 */

static int pik_clock_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    struct mod_pik_clock_module_config *config =
        (struct mod_pik_clock_module_config *)data;
    module_ctx.dev_count = element_count;

    if (element_count == 0)
        return FWK_SUCCESS;

    module_ctx.divider_max =
        ((config == NULL) ? (1 << MOD_PIK_CLOCK_DIVIDER_BITFIELD_WIDTH_5BITS) :
                            config->divider_max);

    module_ctx.dev_ctx_table =
        fwk_mm_calloc(element_count, sizeof(struct pik_clock_dev_ctx));

    return FWK_SUCCESS;
}

static int pik_clock_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    unsigned int i = 0;
    uint64_t current_rate;
    uint64_t last_rate = 0;
    struct pik_clock_dev_ctx *ctx;
    const struct mod_pik_clock_dev_config *dev_config = data;

    if (!fwk_module_is_valid_element_id(element_id))
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(element_id);

    /* Verify that the rate entries in the device's lookup table are ordered */
    while (i < dev_config->rate_count) {
        current_rate = dev_config->rate_table[i].rate;

        /* The rate entries must be in ascending order */
        if (current_rate < last_rate)
            return FWK_E_DATA;

        last_rate = current_rate;
        i++;
    }

    ctx->config = dev_config;

    /* Begin with an invalid source */
    ctx->current_source = MOD_PIK_CLOCK_MSCLOCK_SOURCE_MAX;

    if (ctx->config->defer_initialization)
        return FWK_SUCCESS;

    ctx->current_state = MOD_CLOCK_STATE_RUNNING;
    ctx->initialized = true;

    /*
     * Clock devices that are members of a clock group must skip initialization
     * at this time since they will be set to a specific rate by the CSS Clock
     * driver during the start stage or in response to a notification.
     */
    if (ctx->config->is_group_member)
        return FWK_SUCCESS;

    return do_pik_clock_set_rate(
        element_id, dev_config->initial_rate, MOD_CLOCK_ROUND_MODE_NONE);
}

static int pik_clock_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    struct pik_clock_dev_ctx *ctx;

    /* Only elements can be bound to as the API depends on the element type */
    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_ACCESS;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(target_id);

    if (ctx->config->is_group_member) {
#if BUILD_HAS_MOD_CSS_CLOCK
        /* Only the CSS Clock module can bind to group members. */
        if (fwk_id_get_module_idx(source_id) == FWK_MODULE_IDX_CSS_CLOCK) {
            *api = &api_direct;
            return FWK_SUCCESS;
        } else
            return FWK_E_ACCESS;
#else
        /* The CSS Clock module is required to support group members. */
        return FWK_E_SUPPORT;
#endif
    } else
        *api = &api_clock;

    return FWK_SUCCESS;
}

const struct fwk_module module_synquacer_pik_clock = {
    .name = "SynQuacer PIK Clock Driver",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_PIK_CLOCK_API_COUNT,
    .event_count = 0,
    .init = pik_clock_init,
    .element_init = pik_clock_element_init,
    .process_bind_request = pik_clock_process_bind_request,
};

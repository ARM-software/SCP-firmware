/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_clock.h"
#include "juno_id.h"
#include "juno_scc.h"
#include "system_clock.h"

#include <mod_clock.h>
#include <mod_juno_hdlcd.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

/*
 * This driver supports the following policy with regards to how both HDLCDs
 * share the PLL:
 *
 * Shared:
 * When both HDLCDs require different frequencies that can only be generated
 * using the PLL, then the last HDLCD requesting the new frequency will succeed.
 * This implies that both HDLCDs will have the same frequency when using the
 * PLL. Due to this constraint, the module assumes that the HDLCD clocks have
 * the same rate limits.
 */

struct juno_hdlcd_dev_ctx {
    const struct mod_juno_hdlcd_dev_config *config;
    const struct mod_clock_driver_response_api *driver_response_api;
    const struct mod_juno_hdlcd_drv_api *driver_api;
    int index;
};

struct juno_hdlcd_ctx {
    const struct mod_juno_hdlcd_config *config;

    /* Shared PLL rate value */
    uint32_t current_pll_rate;

    /*
     * Identifier of the clock for which the async request is on-going.
     */
    fwk_id_t request_clock_id;
};

static struct juno_hdlcd_dev_ctx *ctx_table;
static struct juno_hdlcd_ctx module_ctx;

static enum juno_idx_platform platform;

/*
 * Helper functions
 */

static int round_rate(struct juno_hdlcd_dev_ctx *ctx,
                     enum mod_clock_round_mode round_mode,
                     uint32_t rate,
                     uint32_t *rounded_rate)
{
    uint32_t round_up = FWK_ALIGN_NEXT(rate, ctx->config->min_step);
    uint32_t round_down = FWK_ALIGN_PREVIOUS(rate, ctx->config->min_step);

    switch (round_mode) {
    case MOD_CLOCK_ROUND_MODE_NONE:
        *rounded_rate = rate;
        break;

    case MOD_CLOCK_ROUND_MODE_NEAREST:
        /* Select the rounded rate that is closest to the given rate */
        if ((rate - round_down) > (round_up - rate))
            *rounded_rate = round_up;
        else
            *rounded_rate = round_down;
        break;

    case MOD_CLOCK_ROUND_MODE_DOWN:
        *rounded_rate = round_down;
        break;

    case MOD_CLOCK_ROUND_MODE_UP:
        *rounded_rate = round_up;
        break;

    default:
        return FWK_E_SUPPORT;

    }

    if ((*rounded_rate % ctx->config->min_step) != 0)
        return FWK_E_RANGE;

    return FWK_SUCCESS;
}

static void enable_pll(fwk_id_t clock_id, struct juno_hdlcd_dev_ctx *ctx)
{
    FWK_LOG_INFO(
        "[HDLCD%u] Setting PLL R0:0x%" PRIX32 " R1:0x%" PRIX32,
        fwk_id_get_element_idx(clock_id),
        ctx->config->lookup_table[ctx->index].pll.REG0,
        ctx->config->lookup_table[ctx->index].pll.REG1);
    /*
     * The pre-calculated PLL_R0 contains the reset bit cleared which means
     * it will re-enable the PLL.
     */
    SCC->PLL[PLL_IDX_HDLCD].REG1 =
        ctx->config->lookup_table[ctx->index].pll.REG1;
    SCC->PLL[PLL_IDX_HDLCD].REG0 =
        ctx->config->lookup_table[ctx->index].pll.REG0;

    /* Switch HDLCD controller to use PLL clock source */
    *ctx->config->scc_control &= ~SCC_HDLCD_CONTROL_PXLCLK_SEL;
}

/*
 * HDLCD Driver Response API
 */

void juno_hdlcd_request_complete(fwk_id_t dev_id,
    struct mod_clock_driver_resp_params *response_param)
{
    struct juno_hdlcd_dev_ctx *ctx;

    fwk_assert(response_param != NULL);

    ctx = ctx_table + fwk_id_get_element_idx(module_ctx.request_clock_id);

    if (response_param->status == FWK_SUCCESS)
        enable_pll(module_ctx.request_clock_id, ctx);

    ctx->driver_response_api->request_complete(ctx->config->clock_hal_id,
        response_param);

    module_ctx.request_clock_id = FWK_ID_NONE;
}

static const struct mod_clock_driver_response_api hdlcd_driver_response_api = {
    .request_complete = juno_hdlcd_request_complete,
};

/*
 * Clock driver API
 */
static int juno_hdlcd_set_rate(fwk_id_t clock_id, uint64_t rate,
    enum mod_clock_round_mode round_mode)
{
    int status;
    struct juno_hdlcd_dev_ctx *ctx;
    uint32_t clock_rate;
    uint32_t rounded_rate;

    ctx = ctx_table + fwk_id_get_element_idx(clock_id);

    status = round_rate(ctx, round_mode, rate, &rounded_rate);
    if (status != FWK_SUCCESS)
        return status;

    if ((rounded_rate < ctx->config->min_rate) ||
        (rounded_rate > ctx->config->max_rate))
        return FWK_E_RANGE;

    if (platform == JUNO_IDX_PLATFORM_RTL) {
        if (!fwk_id_is_equal(module_ctx.request_clock_id, FWK_ID_NONE))
            return FWK_E_BUSY;
    }

    /*
     * Clock rate is always twice the pixel clock rate. This is because Juno has
     * an implicit "divide by 2" stage.
     */
    clock_rate = rounded_rate * 2;

    /*
     * If the target rate matches the external OSC, then use it.
     */
    if (clock_rate == PXL_CLK_IN_RATE) {
        /* Switch HDLCD controller to use external OSC clock source */
        *ctx->config->scc_control &= ~SCC_HDLCD_CONTROL_PXLCLK_SEL;
        *ctx->config->scc_control |= SCC_HDLCD_CONTROL_PXLCLK_SEL_CLKIN;

        FWK_LOG_INFO(
            "[HDLCD%u]: Request:%" PRIu32 "Hz",
            fwk_id_get_element_idx(clock_id),
            rounded_rate);

        return FWK_SUCCESS;
    }

    /*
     * Check if we can re-use the current PLL frequency.
     */
    if (clock_rate == module_ctx.current_pll_rate) {
        /* Switch HDLCD controller to use PLL clock source */
        *ctx->config->scc_control &= ~SCC_HDLCD_CONTROL_PXLCLK_SEL;
        *ctx->config->scc_control |= SCC_HDLCD_CONTROL_PXLCLK_SEL_PLL;

        FWK_LOG_INFO(
            "[HDLCD%u]: Request:%" PRIu32 "Hz",
            fwk_id_get_element_idx(clock_id),
            rounded_rate);

        return FWK_SUCCESS;
    }

    /*
     * Due to the sharing policy: go ahead and change the PLL even if the other
     * PLL is already using it
     */
    /* Find entry on the look-up table */
    ctx->index = (clock_rate - PXL_CLK_IN_RATE) / (500 * FWK_KHZ);
    if ((ctx->index < 0) ||
        ((unsigned int)ctx->index >= ctx->config->lookup_table_count))
        return FWK_E_RANGE;

    FWK_LOG_INFO(
        "[HDLCD%u] Entry index:%d",
        fwk_id_get_element_idx(clock_id),
        ctx->index);

    /* Hold PLL in reset during the configuration process */
    SCC->PLL[PLL_IDX_HDLCD].REG0 = (PLL_REG0_PLL_RESET | PLL_REG0_HARD_BYPASS);

    module_ctx.current_pll_rate = clock_rate;
    if (platform == JUNO_IDX_PLATFORM_RTL) {
        /* CLK_HDLCD_REFCLK is an external I2C based oscillator. */
        status = ctx->driver_api->set_rate_from_index(ctx->config->driver_id,
            ctx->index);
        if ((status != FWK_PENDING) && (status != FWK_SUCCESS)) {
            FWK_LOG_ERR("[HDLCD] Failed to set board clock");
            return FWK_E_DEVICE;
        }
        if (status == FWK_PENDING)
            module_ctx.request_clock_id = clock_id;
        return status;
    }

    enable_pll(clock_id, ctx);

    return FWK_SUCCESS;
}

static int juno_hdlcd_get_rate(fwk_id_t clock_id, uint64_t *rate)
{
    struct juno_hdlcd_dev_ctx *ctx;

    ctx = ctx_table + fwk_id_get_element_idx(clock_id);

    /* Find out the clock source in use */
    if ((*ctx->config->scc_control & SCC_HDLCD_CONTROL_PXLCLK_SEL) ==
        SCC_HDLCD_CONTROL_PXLCLK_SEL_CLKIN)
        *rate = PXL_CLK_IN_RATE;
    else
        *rate = module_ctx.current_pll_rate;

    /* All frequencies are double the target pixel clock frequency */
    *rate /= 2;

    return FWK_SUCCESS;
}

static int juno_hdlcd_get_rate_from_index(fwk_id_t clock_id,
                                          unsigned int rate_index,
                                          uint64_t *rate)
{
    return FWK_E_SUPPORT;
}

static int juno_hdlcd_set_state(fwk_id_t clock_id,
                                enum mod_clock_state state)
{
    if (state != MOD_CLOCK_STATE_RUNNING)
        return FWK_E_SUPPORT;

    return FWK_SUCCESS;
}

static int juno_hdlcd_get_state(fwk_id_t clock_id,
                                enum mod_clock_state *state)
{
    *state = MOD_CLOCK_STATE_RUNNING;

    return FWK_SUCCESS;
}

static int juno_hdlcd_get_range(fwk_id_t clock_id,
                                struct mod_clock_range *range)
{
    struct juno_hdlcd_dev_ctx *ctx;

    ctx = ctx_table + fwk_id_get_element_idx(clock_id);

    range->min = ctx->config->min_rate;
    range->max = ctx->config->max_rate;
    range->step = ctx->config->min_step;
    range->rate_type = ctx->config->rate_type;

    return FWK_SUCCESS;
}

static const struct mod_clock_drv_api clock_driver_api = {
    .set_rate = juno_hdlcd_set_rate,
    .get_rate = juno_hdlcd_get_rate,
    .get_rate_from_index = juno_hdlcd_get_rate_from_index,
    .set_state = juno_hdlcd_set_state,
    .get_state = juno_hdlcd_get_state,
    .get_range = juno_hdlcd_get_range,
};

/*
 * Framework handlers
 */
static int juno_hdlcd_init(fwk_id_t module_id,
                           unsigned int element_count,
                           const void *data)
{
    ctx_table = fwk_mm_calloc(element_count, sizeof(*ctx_table));

    return FWK_SUCCESS;
}

static int juno_hdlcd_dev_init(fwk_id_t element_id,
                               unsigned int sub_element_count,
                               const void *data)
{
    static uint64_t min_rate;
    static uint64_t max_rate;
    const struct mod_juno_hdlcd_dev_config *config = data;

    fwk_assert(config->min_rate != 0);
    fwk_assert(config->max_rate != 0);

    if (min_rate == 0)
        min_rate = config->min_rate;
    if (max_rate == 0)
        max_rate = config->max_rate;

    /*
     * All the HDLCD clocks should have the same max_rate and the same min_rate
     * due to the PLL shared policy.
     */
    fwk_assert(min_rate == config->min_rate);
    fwk_assert(max_rate == config->max_rate);

    ctx_table[fwk_id_get_element_idx(element_id)].config = config;

    return FWK_SUCCESS;
}

static int juno_hdlcd_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct juno_hdlcd_dev_ctx *ctx;

    /* Only bind in first round of calls */
    if (round > 0)
        return FWK_SUCCESS;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    ctx = ctx_table + fwk_id_get_element_idx(id);

    /* Bind to clock HAL */
    status = fwk_module_bind(ctx->config->clock_hal_id,
        ctx->config->clock_api_id, &ctx->driver_response_api);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    /* Bind to the driver API */
    if (platform == JUNO_IDX_PLATFORM_RTL) {
        status = fwk_module_bind(ctx->config->driver_id,
            ctx->config->driver_api_id, &ctx->driver_api);
        if (status != FWK_SUCCESS)
            return FWK_E_HANDLER;
    }

    return FWK_SUCCESS;
}

static int juno_hdlcd_process_bind_request(fwk_id_t source_id,
                                           fwk_id_t target_id,
                                           fwk_id_t api_id,
                                           const void **api)
{
    if (fwk_id_is_equal(api_id, mod_juno_hdlcd_api_id_clock_driver))
        *api = &clock_driver_api;
    else if (fwk_id_is_equal(api_id,
        mod_juno_hdlcd_api_id_hdlcd_driver_response))
        *api = &hdlcd_driver_response_api;
    else
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int juno_hdlcd_start(fwk_id_t id)
{
    unsigned int nf;
    unsigned int nr;
    unsigned int od;
    int status;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    status = juno_id_get_platform(&platform);
    if (status != FWK_SUCCESS)
        return status;

    /* Read current PLL frequency */
    nf = ((SCC->PLL[PLL_IDX_HDLCD].REG0 & PLL_REG0_NF) >> PLL_REG0_NF_POS) + 1;
    nr = ((SCC->PLL[PLL_IDX_HDLCD].REG1 & PLL_REG1_NR) >> PLL_REG1_NR_POS) + 1;
    od = ((SCC->PLL[PLL_IDX_HDLCD].REG1 & PLL_REG1_OD) >> PLL_REG1_OD_POS) + 1;

    module_ctx.current_pll_rate = ((uint64_t)(PXL_REF_CLK_RATE) * nf) /
                                  (nr * od);
    module_ctx.request_clock_id = FWK_ID_NONE;

    return FWK_SUCCESS;
}

const struct fwk_module module_juno_hdlcd = {
    .name = "JUNO HDLCD",
    .api_count = MOD_JUNO_HDLCD_API_COUNT,
    .type = FWK_MODULE_TYPE_HAL,
    .init = juno_hdlcd_init,
    .element_init = juno_hdlcd_dev_init,
    .bind = juno_hdlcd_bind,
    .process_bind_request = juno_hdlcd_process_bind_request,
    .start = juno_hdlcd_start,
};

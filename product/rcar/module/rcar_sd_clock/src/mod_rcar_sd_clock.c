/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clock_sd_devices.h>
#include <mmio.h>

#include <mod_clock.h>
#include <mod_rcar_sd_clock.h>
#include <mod_rcar_system.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdint.h>
#include <stdlib.h>

static struct rcar_sd_clock_ctx module_ctx;

/*
 * Static helper functions
 */
static int get_rate_entry(
    struct rcar_sd_clock_dev_ctx *ctx,
    uint64_t target_rate,
    struct mod_rcar_sd_clock_rate **entry)
{
    struct mod_rcar_sd_clock_rate *rate_entry;
    uint32_t cnt;
    if (ctx == NULL)
        return FWK_E_PARAM;
    if (entry == NULL)
        return FWK_E_PARAM;

    /* Perform a binary search to find the entry matching the requested rate */
    if (ctx->config->rate_type == MOD_CLOCK_RATE_TYPE_CONTINUOUS) {
        if ((ctx->rate_table[0] > target_rate) &&
            (ctx->rate_table[1] < target_rate))
            return FWK_E_PARAM;
        rate_entry = (struct mod_rcar_sd_clock_rate *)ctx->config->rate_table;
        *entry = rate_entry;
    } else {
        for (cnt = 0; cnt < ctx->config->rate_count; cnt++) {
            if (ctx->rate_table[cnt] == target_rate)
                break;
        }

        if (cnt >= ctx->config->rate_count)
            return FWK_E_PARAM;
        *entry = (struct mod_rcar_sd_clock_rate *)&ctx->config->rate_table[cnt];
    }
    return FWK_SUCCESS;
}

static int do_sd_clock_set_rate(fwk_id_t dev_id, uint64_t rate)
{
    int status;
    struct rcar_sd_clock_dev_ctx *ctx;
    struct mod_rcar_sd_clock_rate *rate_entry;
    uint32_t value;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    status = get_rate_entry(ctx, rate, &rate_entry);
    if (status != FWK_SUCCESS)
        return status;

    switch (ctx->config->type) {
    case MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE:
        return FWK_E_SUPPORT;
    case MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE:
        if (ctx->config->rate_type == MOD_CLOCK_RATE_TYPE_DISCRETE) {
            value = mmio_read_32(ctx->config->control_reg);
            value &= (~rate_entry->divider_mask);
            value |= rate_entry->divider;
            mmio_write_32(ctx->config->control_reg, value);
        } else {
            value = mmio_read_32(ctx->config->control_reg);
            value &= (~CPG_CON_MASK);
            value |= (CPG_CON_MAX - (rate / ctx->rate_table[2])) & CPG_CON_MASK;
            mmio_write_32(ctx->config->control_reg, value);
        }
        break;
    default:
        return FWK_E_SUPPORT;
    }

    ctx->current_rate = rate;

    return FWK_SUCCESS;
}

/*
 * Clock driver API functions
 */
static int sd_clock_set_rate(
    fwk_id_t dev_id,
    uint64_t rate,
    enum mod_clock_round_mode round_mode)
{
    struct rcar_sd_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (!ctx->initialized)
        return FWK_E_INIT;

    if (ctx->current_state == MOD_CLOCK_STATE_STOPPED)
        return FWK_E_PWRSTATE;

    return do_sd_clock_set_rate(dev_id, rate);
}

static int sd_clock_get_rate(fwk_id_t dev_id, uint64_t *rate)
{
    struct rcar_sd_clock_dev_ctx *ctx;

    if (rate == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    *rate = ctx->current_rate;

    return FWK_SUCCESS;
}

static int sd_clock_get_rate_from_index(
    fwk_id_t dev_id,
    unsigned int rate_index,
    uint64_t *rate)
{
    struct rcar_sd_clock_dev_ctx *ctx;

    if (rate == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (rate_index >= ctx->config->rate_count)
        return FWK_E_PARAM;

    *rate = ctx->rate_table[rate_index];
    return FWK_SUCCESS;
}

static int sd_clock_set_state(
    fwk_id_t dev_id,
    enum mod_clock_state target_state)
{
    struct rcar_sd_clock_dev_ctx *ctx;
    uint32_t value;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    /* No operation if the HW doesn't allow clock state change */
    if (!ctx->config->stop_clk)
        return FWK_SUCCESS;

    value = mmio_read_32(ctx->config->control_reg);
    if (MOD_CLOCK_STATE_RUNNING == target_state)
        value &= ~(BIT(ctx->config->stop_clk_bit));
    else
        value |= BIT(ctx->config->stop_clk_bit);

    mmio_write_32(ctx->config->control_reg, value);

    ctx->current_state = target_state;

    return FWK_SUCCESS;
}

static int sd_clock_get_state(fwk_id_t dev_id, enum mod_clock_state *state)
{
    struct rcar_sd_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
    *state = ctx->current_state;
    return FWK_SUCCESS;
}
static int sd_clock_get_range(fwk_id_t dev_id, struct mod_clock_range *range)
{
    struct rcar_sd_clock_dev_ctx *ctx;

    if (range == NULL)
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    range->rate_type = ctx->config->rate_type;
    if (ctx->config->rate_count > 0) {
        if (MOD_CLOCK_RATE_TYPE_CONTINUOUS == range->rate_type) {
            /* SCMI_CLOCK_RATE_FORMAT_RANGE */
            range->min = ctx->rate_table[0];
            range->max = ctx->rate_table[1];
            range->step = ctx->rate_table[2];
        } else {
            /* SCMI_CLOCK_RATE_FORMAT_LIST */
            range->min = ctx->rate_table[0];
            range->max = ctx->rate_table[ctx->config->rate_count - 1];
        }
    }
    range->rate_count = ctx->config->rate_count;

    return FWK_SUCCESS;
}

static int sd_clock_hw_initial_set_state(
    fwk_id_t element_id,
    struct rcar_sd_clock_dev_ctx *ctx)
{
    uint32_t i = 0;
    uint32_t value = 0;
    uint32_t div_value = 0;
    uint64_t rate = 0;

    switch (ctx->config->type) {
    case MOD_RCAR_CLOCK_TYPE_SINGLE_SOURCE:
        ctx->current_state = MOD_CLOCK_STATE_RUNNING;
        div_value = ctx->config->div;
        break;
    case MOD_RCAR_CLOCK_TYPE_MULTI_SOURCE:
        /* Maintain clock supply at startup. */
        ctx->current_state = MOD_CLOCK_STATE_RUNNING;
        if (ctx->config->stop_clk) {
            if (mmio_read_32(ctx->config->control_reg) &
                BIT(ctx->config->stop_clk_bit))
                ctx->current_state = MOD_CLOCK_STATE_STOPPED;
        }
        /* Holds clock frequency at startup. */
        if (ctx->config->rate_type == MOD_CLOCK_RATE_TYPE_DISCRETE) {
            value = mmio_read_32(ctx->config->control_reg);
            value &= ctx->config->rate_table[0].divider_mask;
            for (i = 0; i < ctx->config->rate_count; i++) {
                if (value == ctx->config->rate_table[i].divider) {
                    div_value = ctx->config->rate_table[i].divider_num;
                    break;
                }
            }
        } else {
            value = mmio_read_32(ctx->config->control_reg);
            value &= (CPG_CON_MASK);
            div_value = (value + 1);
        }
        break;
    default:
        return FWK_E_SUPPORT;
    }
    ctx->current_rate = module_ctx.parent_clk[ctx->config->parent] / div_value;

    if (ctx->config->need_hardware_init) {
        rate = module_ctx.parent_clk[ctx->config->parent] / ctx->config->div;
        do_sd_clock_set_rate(element_id, rate);
    }

    return FWK_SUCCESS;
}

static int sd_clock_resume(void)
{
    fwk_id_t element_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLK_ID_SD_START);
    struct rcar_sd_clock_dev_ctx *ctx;
    uint32_t sd_id = 0;
    int ret = FWK_SUCCESS;

    for (sd_id = CLK_ID_SD_START; sd_id < CLK_ID_SD_END; sd_id++) {
        element_id.element.element_idx = sd_id;
        ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(element_id);

        ret = sd_clock_hw_initial_set_state(element_id, ctx);
        if (ret != FWK_SUCCESS)
            break;
    }
    return ret;
}

static const struct mod_rcar_clock_drv_api api_clock = {
    .set_rate = sd_clock_set_rate,
    .get_rate = sd_clock_get_rate,
    .get_rate_from_index = sd_clock_get_rate_from_index,
    .set_state = sd_clock_set_state,
    .get_state = sd_clock_get_state,
    .get_range = sd_clock_get_range,
};

static const struct mod_rcar_system_drv_api api_system = {
     .resume = sd_clock_resume,
 };

/*
 * Framework handler functions
 */

static int r8a7795_cpg_pll1_init(uint32_t ext_rate)
{
    const struct rcar_gen3_cpg_pll_config *cpg_pll_config;
    uint32_t saved_mode;
    saved_mode = mmio_read_32(RCAR_MODEMR);
    cpg_pll_config = &cpg_pll_configs[CPG_PLL_CONFIG_INDEX(saved_mode)];
    return (ext_rate * cpg_pll_config->pll1_mult / cpg_pll_config->pll1_div);
}

static int r8a7795_cpg_init(uint32_t ext_rate)
{
    const struct rcar_gen3_cpg_pll_config *cpg_pll_config;
    uint32_t saved_mode;
    saved_mode = mmio_read_32(RCAR_MODEMR);
    cpg_pll_config = &cpg_pll_configs[CPG_PLL_CONFIG_INDEX(saved_mode)];
    return (ext_rate / cpg_pll_config->osc_prediv);
}

static int sd_clock_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    struct mod_ext_clock_rate *ext;
    module_ctx.dev_count = element_count;

    if (element_count == 0)
        return FWK_SUCCESS;

    module_ctx.dev_ctx_table =
        fwk_mm_calloc(element_count, sizeof(struct rcar_sd_clock_dev_ctx));
    if (module_ctx.dev_ctx_table == NULL)
        return FWK_E_NOMEM;

    /* Parent clock calculation. */
    ext = (struct mod_ext_clock_rate *)data;
    module_ctx.parent_clk[CLK_EXTAL] = ext->ext_clk_rate;
    module_ctx.parent_clk[CLK_OSC_EXTAL] = r8a7795_cpg_init(ext->ext_clk_rate);
    module_ctx.parent_clk[CLK_PLL1] = r8a7795_cpg_pll1_init(ext->ext_clk_rate);
    module_ctx.parent_clk[CLK_PLL1_DIV2] = module_ctx.parent_clk[CLK_PLL1] / 2;
    module_ctx.parent_clk[CLK_PLL1_DIV4] = module_ctx.parent_clk[CLK_PLL1] / 4;
    ;
    module_ctx.parent_clk[CLK_S0] = module_ctx.parent_clk[CLK_PLL1_DIV2] / 2;
    module_ctx.parent_clk[CLK_S1] = module_ctx.parent_clk[CLK_PLL1_DIV2] / 3;
    module_ctx.parent_clk[CLK_S2] = module_ctx.parent_clk[CLK_PLL1_DIV2] / 4;
    module_ctx.parent_clk[CLK_S3] = module_ctx.parent_clk[CLK_PLL1_DIV2] / 6;
    module_ctx.parent_clk[CLK_SDSRC] = module_ctx.parent_clk[CLK_PLL1_DIV2] / 2;

    return FWK_SUCCESS;
}

static int sd_clock_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct rcar_sd_clock_dev_ctx *ctx;
    const struct mod_rcar_sd_clock_dev_config *dev_config = data;
    unsigned int i;
    uint64_t current_rate;
    uint64_t last_rate = 0;

    if (!fwk_module_is_valid_element_id(element_id))
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(element_id);
    ctx->config = dev_config;

    if (ctx->config->rate_count > 0) {
        ctx->rate_table =
            fwk_mm_calloc(ctx->config->rate_count, sizeof(uint64_t));
        if (ctx->rate_table == NULL)
            return FWK_E_NOMEM;

        for (i = 0; i < ctx->config->rate_count; i++)
            ctx->rate_table[i] = module_ctx.parent_clk[ctx->config->parent] /
                ctx->config->rate_table[i].divider_num;
    }

    /* Verify that the rate entries in the device's lookup table are ordered */
    if (dev_config->rate_type == MOD_CLOCK_RATE_TYPE_CONTINUOUS) {
        if (ctx->rate_table[0] > ctx->rate_table[1])
            return FWK_E_DATA;
    } else {
        i = 0;
        while (i < dev_config->rate_count) {
            current_rate = ctx->rate_table[i];

            /* The rate entries must be in ascending order */
            if (current_rate < last_rate)
                return FWK_E_DATA;

            last_rate = current_rate;
            i++;
        }
    }

    ctx->initialized = true;

    return FWK_SUCCESS;
}

static int sd_clock_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{

    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_RCAR_CLOCK_API_TYPE_SYSTEM:
        *api = &api_system;
        break;
    default:
        *api = &api_clock;
        break;
    }

    return FWK_SUCCESS;
}

static int sd_clock_start(fwk_id_t id)
{
    int ret = FWK_SUCCESS;

    /* for Not Module */
    if (!fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return ret;

    ret = sd_clock_resume();
    return ret;
}

const struct fwk_module module_rcar_sd_clock = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_RCAR_CLOCK_API_COUNT,
    .event_count = 0,
    .init = sd_clock_init,
    .element_init = sd_clock_element_init,
    .process_bind_request = sd_clock_process_bind_request,
    .start = sd_clock_start,
};

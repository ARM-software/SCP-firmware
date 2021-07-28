/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <clock_mstp_devices.h>
#include <mmio.h>
#include <utils_def.h>

#include <mod_clock.h>
#include <mod_rcar_clock.h>
#include <mod_rcar_mstp_clock.h>
#include <mod_rcar_system.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdint.h>

static struct rcar_mstp_clock_ctx module_ctx;

/*
 * Static helper functions
 */
static int mstp_clock_set_state(
    fwk_id_t dev_id,
    enum mod_clock_state target_state)
{
    struct rcar_mstp_clock_dev_ctx *ctx;
    uint32_t value;
    int i;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    value = mmio_read_32(CPG_BASE + smstpcr[ctx->config->control_reg]);
    if (MOD_CLOCK_STATE_RUNNING == target_state)
        value &= ~(BIT(ctx->config->bit));
    else
        value |= BIT(ctx->config->bit);

    mmio_write_32((CPG_BASE + smstpcr[ctx->config->control_reg]), value);

    if (MOD_CLOCK_STATE_RUNNING == target_state) {
        for (i = 1000; i > 0; --i) {
            if (!(mmio_read_32(CPG_BASE + mstpsr[ctx->config->control_reg]) &
                  BIT(ctx->config->bit)))
                break;
        }

        if (!i)
            return FWK_E_TIMEOUT;
    }

    ctx->current_state = target_state;
    return FWK_SUCCESS;
}

static int mstp_clock_get_state(fwk_id_t dev_id, enum mod_clock_state *state)
{
    struct rcar_mstp_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
    *state = ctx->current_state;
    return FWK_SUCCESS;
}

static void mstp_clock_hw_initial_set_state(
    fwk_id_t element_id,
    struct rcar_mstp_clock_dev_ctx *ctx)
{
    /* Maintain clock supply at startup. */
    if (module_ctx.mstp_init->smstpcr_init[ctx->config->control_reg] &
        BIT(ctx->config->bit))
        ctx->current_state = MOD_CLOCK_STATE_STOPPED;
    else
        ctx->current_state = MOD_CLOCK_STATE_RUNNING;

    /* If true, the driver will provide a default clock supply. */
    if (ctx->config->defer_initialization)
        mstp_clock_set_state(element_id, MOD_CLOCK_STATE_RUNNING);
}

static int mstp_clock_resume(void)
{
    fwk_id_t element_id =
        FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_CLOCK, CLK_ID_MSTP_START);
    uint32_t mstp_id;
    struct rcar_mstp_clock_dev_ctx *ctx;

    mmio_write_32(SMSTPCR0, SMSTPCR0_VALUE);
    mmio_write_32(SMSTPCR1, SMSTPCR1_VALUE);
    mmio_write_32(SMSTPCR2, SMSTPCR2_VALUE);
    mmio_write_32(SMSTPCR3, SMSTPCR3_VALUE);
    mmio_write_32(SMSTPCR4, SMSTPCR4_VALUE);
    mmio_write_32(SMSTPCR5, SMSTPCR5_VALUE);
    mmio_write_32(SMSTPCR6, SMSTPCR6_VALUE);
    mmio_write_32(SMSTPCR7, SMSTPCR7_VALUE);
    mmio_write_32(SMSTPCR8, SMSTPCR8_VALUE);
    mmio_write_32(SMSTPCR9, SMSTPCR9_VALUE);
    mmio_write_32(SMSTPCR10, SMSTPCR10_VALUE);
    mmio_write_32(SMSTPCR11, SMSTPCR11_VALUE);

    for (mstp_id = CLK_ID_MSTP_START; mstp_id < CLK_ID_MSTP_END; mstp_id++) {
        element_id.element.element_idx = mstp_id;
        ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(element_id);
        mstp_clock_hw_initial_set_state(element_id, ctx);
    }
    return FWK_SUCCESS;
}

static int mstp_clock_set_rate(
    fwk_id_t dev_id,
    uint64_t rate,
    enum mod_clock_round_mode round_mode)
{
    return FWK_SUCCESS;
}

static int mstp_clock_get_rate(fwk_id_t dev_id, uint64_t *rate)
{
    return FWK_SUCCESS;
}

static int mstp_clock_get_rate_from_index(
    fwk_id_t dev_id,
    unsigned int rate_index,
    uint64_t *rate)
{
    return FWK_SUCCESS;
}

static int mstp_clock_get_range(fwk_id_t dev_id, struct mod_clock_range *range)
{
    return FWK_SUCCESS;
}

static const struct mod_rcar_clock_drv_api api_clock = {
    .set_state = mstp_clock_set_state,
    .get_state = mstp_clock_get_state,
    .set_rate = mstp_clock_set_rate,
    .get_rate = mstp_clock_get_rate,
    .get_rate_from_index = mstp_clock_get_rate_from_index,
    .get_range = mstp_clock_get_range,
};

static const struct mod_rcar_system_drv_api api_system = {
    .resume = mstp_clock_resume,
};

/*
 * Framework handler functions
 */

static int mstp_clock_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    const struct mod_rcar_mstp_clock_init *mstp_init = data;

    module_ctx.dev_count = element_count;

    if (element_count == 0)
        return FWK_SUCCESS;

    if (mstp_init == NULL)
        return FWK_E_PARAM;

    module_ctx.mstp_init = mstp_init;
    module_ctx.dev_ctx_table =
        fwk_mm_calloc(element_count, sizeof(struct rcar_mstp_clock_dev_ctx));
    if (module_ctx.dev_ctx_table == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int mstp_clock_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct rcar_mstp_clock_dev_ctx *ctx;
    const struct mod_rcar_mstp_clock_dev_config *dev_config = data;

    if (!fwk_module_is_valid_element_id(element_id))
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(element_id);
    ctx->config = dev_config;
    ctx->initialized = true;

    return FWK_SUCCESS;
}

static int mstp_clock_process_bind_request(
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

static int mstp_clock_start(fwk_id_t id)
{
    int ret = FWK_SUCCESS;

    /* for Not Module */
    if (!fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return ret;

    ret = mstp_clock_resume();
    return ret;
}

const struct fwk_module module_rcar_mstp_clock = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_RCAR_CLOCK_API_COUNT,
    .event_count = 0,
    .init = mstp_clock_init,
    .element_init = mstp_clock_element_init,
    .process_bind_request = mstp_clock_process_bind_request,
    .start = mstp_clock_start,
};

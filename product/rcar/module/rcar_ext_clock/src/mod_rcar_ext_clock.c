/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mmio.h>
#include <clock_ext_devices.h>

#include <mod_clock.h>
#include <mod_rcar_clock.h>
#include <mod_rcar_ext_clock.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_status.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static struct rcar_ext_clock_ctx module_ctx;

/*
 * Static helper functions
 */
static int ext_clock_set_rate(
    fwk_id_t dev_id,
    uint64_t rate,
    enum mod_clock_round_mode round_mode)
{
    return FWK_SUCCESS;
}

static int ext_clock_get_rate(fwk_id_t dev_id, uint64_t *rate)
{
    struct rcar_ext_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
    *rate = ctx->config->clock_rate;
    return FWK_SUCCESS;
}

static int ext_clock_set_state(
    fwk_id_t dev_id,
    enum mod_clock_state target_state)
{
    return FWK_SUCCESS;
}

static int ext_clock_get_state(fwk_id_t dev_id, enum mod_clock_state *state)
{
    struct rcar_ext_clock_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
    *state = ctx->config->clock_state;
    return FWK_SUCCESS;
}

static int ext_clock_get_range(fwk_id_t dev_id, struct mod_clock_range *range)
{
    return FWK_SUCCESS;
}

static int ext_clock_get_rate_from_index(
    fwk_id_t dev_id,
    unsigned int rate_index,
    uint64_t *rate)
{
    return FWK_SUCCESS;
}

static const struct mod_rcar_clock_drv_api api_clock = {
    .set_rate = ext_clock_set_rate,
    .get_rate = ext_clock_get_rate,
    .set_state = ext_clock_set_state,
    .get_state = ext_clock_get_state,
    .get_rate_from_index = ext_clock_get_rate_from_index,
    .get_range = ext_clock_get_range,
};

/*
 * Framework handler functions
 */

static int ext_clock_init(fwk_id_t module_id, unsigned int element_count,
                          const void *data)
{
    module_ctx.dev_count = element_count;

    if (element_count == 0x0)
        return FWK_SUCCESS;

    module_ctx.dev_ctx_table = fwk_mm_calloc(element_count,
                                     sizeof(struct rcar_ext_clock_dev_ctx));
    if (module_ctx.dev_ctx_table == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int ext_clock_element_init(fwk_id_t element_id,
                                  unsigned int sub_element_count,
                                  const void *data)
{
    struct rcar_ext_clock_dev_ctx *ext_clock_ctx;
    const struct mod_rcar_ext_clock_dev_config *dev_config = data;

    if (!fwk_module_is_valid_element_id(element_id))
        return FWK_E_PARAM;

    ext_clock_ctx = module_ctx.dev_ctx_table +
        fwk_id_get_element_idx(element_id);
    ext_clock_ctx->config = dev_config;
    ext_clock_ctx->ext_clock_initialized = true;

    return FWK_SUCCESS;
}

static int ext_clock_process_bind_request(fwk_id_t source_id,
                                           fwk_id_t target_id, fwk_id_t api_id,
                                           const void **api)
{
    *api = &api_clock;
    return FWK_SUCCESS;
}

static int ext_clock_start(fwk_id_t id)
{
    return FWK_SUCCESS;
}

const struct fwk_module module_rcar_ext_clock = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_RCAR_CLOCK_API_COUNT,
    .event_count = 0,
    .init = ext_clock_init,
    .element_init = ext_clock_element_init,
    .process_bind_request = ext_clock_process_bind_request,
    .start = ext_clock_start,
};

/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_reset_domain.h>
#include <mod_rcar_reset.h>
#include <mmio.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_status.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>
#include <stdlib.h>
#include <lib/utils_def.h>

static struct rcar_reset_ctx module_ctx;

/*
 * Static helper functions
 */
static void udelay(uint32_t cycles)
{
  volatile uint32_t i;

    for (i = 0UL; i < cycles ; ++i)
        __asm__ volatile ("nop");
}

static int rcar_auto_domain(fwk_id_t dev_id, uint32_t state)
{
    struct rcar_reset_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    mmio_write_32((CPG_BASE + srcr[ctx->config->control_reg]),
                   BIT(ctx->config->bit));

    /* Wait for at least one cycle of the RCLK clock (@ ca. 32 kHz) */
    udelay(SCSR_DELAY_US);

    /* Release module from reset state */
    mmio_write_32((CPG_BASE + SRSTCLR(ctx->config->control_reg)),
                   BIT(ctx->config->bit));

    return FWK_SUCCESS;
}

static int rcar_assert_domain(fwk_id_t dev_id)
{
    struct rcar_reset_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    mmio_write_32((CPG_BASE + srcr[ctx->config->control_reg]),
                   BIT(ctx->config->bit));

    return FWK_SUCCESS;
}

static int rcar_deassert_domain(fwk_id_t dev_id)
{
    struct rcar_reset_dev_ctx *ctx;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    mmio_write_32((CPG_BASE + SRSTCLR(ctx->config->control_reg)),
                   BIT(ctx->config->bit));

    return FWK_SUCCESS;
}

static int rcar_set_reset_state(fwk_id_t dev_id,
                           enum mod_reset_domain_mode mode,
                           uint32_t reset_state,
                           uintptr_t cookie)
{

    switch (mode) {
    case MOD_RESET_DOMAIN_AUTO_RESET:
        rcar_auto_domain(dev_id, reset_state);
        break;

    case MOD_RESET_DOMAIN_MODE_EXPLICIT_ASSERT:
        rcar_assert_domain(dev_id);
        break;

    case MOD_RESET_DOMAIN_MODE_EXPLICIT_DEASSERT:
        rcar_deassert_domain(dev_id);
        break;

    default:
        return FWK_E_ACCESS;
    }

    return FWK_SUCCESS;
}

static const struct mod_reset_domain_drv_api api_reset = {
    .set_reset_state = rcar_set_reset_state,
};

/*
 * Framework handler functions
 */

static int reset_init(fwk_id_t module_id, unsigned int element_count,
                          const void *data)
{
    module_ctx.dev_count = element_count;

    if (element_count == 0)
        return FWK_SUCCESS;

    module_ctx.dev_ctx_table = fwk_mm_calloc(element_count,
                                         sizeof(struct rcar_reset_dev_ctx));
    if (module_ctx.dev_ctx_table == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int reset_element_init(fwk_id_t element_id,
                                  unsigned int sub_element_count,
                                  const void *data)
{
    struct rcar_reset_dev_ctx *ctx;
    const struct mod_rcar_reset_dev_config *dev_config = data;

    if (!fwk_module_is_valid_element_id(element_id))
        return FWK_E_PARAM;

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(element_id);
    ctx->config = dev_config;

    return FWK_SUCCESS;
}

static int reet_process_bind_request(fwk_id_t source_id,
                                          fwk_id_t target_id, fwk_id_t api_id,
                                          const void **api)
{
    *api = &api_reset;
    return FWK_SUCCESS;
}

const struct fwk_module module_rcar_reset = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_RCAR_RESET_API_COUNT,
    .event_count = 0,
    .init = reset_init,
    .element_init = reset_element_init,
    .process_bind_request = reet_process_bind_request,
};

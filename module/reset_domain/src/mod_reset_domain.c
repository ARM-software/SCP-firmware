/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Reset domain HAL
 */

#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_log.h>
#include <mod_reset_domain.h>

/*
 * Module and devices contexts for Reset Domain
 * TODO: notification
 */

/* Device context */
struct rd_dev_ctx {
    const struct mod_reset_domain_dev_config *config;
    struct mod_reset_domain_drv_api *driver_api;
};

/* Module context */
struct rd_mod_ctx {
    const struct mod_reset_domain_config *config;
    struct rd_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
    const struct mod_log_api *log_api;
};

/*
 * Internal variables
 */
static struct rd_mod_ctx module_ctx;

/*
 * API functions
 */
static int rd_autonomous(fwk_id_t reset_id, unsigned int state)
{
    int status = 0;
    struct rd_dev_ctx *reset_ctx = NULL;

    status = fwk_module_check_call(reset_id);
    if (status != FWK_SUCCESS)
        return status;

    reset_ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(reset_id)];

    return reset_ctx->driver_api->auto_domain(reset_ctx->config->driver_id,
                                              state);
}

static int rd_assert(fwk_id_t reset_id)
{
    int status = 0;
    struct rd_dev_ctx *reset_ctx = NULL;

    status = fwk_module_check_call(reset_id);
    if (status != FWK_SUCCESS)
        return status;

    reset_ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(reset_id)];

    return reset_ctx->driver_api->assert_domain(reset_ctx->config->driver_id);
}

static int rd_deassert(fwk_id_t reset_id)
{
    int status = 0;
    struct rd_dev_ctx *reset_ctx = NULL;

    status = fwk_module_check_call(reset_id);
    if (status != FWK_SUCCESS)
        return status;

    reset_ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(reset_id)];

    return reset_ctx->driver_api->deassert_domain(reset_ctx->config->driver_id);
}

static const struct mod_reset_domain_drv_api reset_api = {
    .auto_domain = rd_autonomous,
    .assert_domain = rd_assert,
    .deassert_domain = rd_deassert,
};

/*
 * Framework handlers
 */
static int rd_init(fwk_id_t module_id,
                   unsigned int dev_count, const void *data)
{
    module_ctx.dev_count = dev_count;

    module_ctx.config = (struct mod_reset_domain_config *)data;
    module_ctx.dev_ctx_table = fwk_mm_calloc(dev_count,
                                             sizeof(struct rd_dev_ctx));
    if (module_ctx.dev_ctx_table == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int rd_elt_init(fwk_id_t elt_id,
                       unsigned int dev_count, const void *data)
{
    struct rd_dev_ctx *reset_ctx = NULL;

    reset_ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(elt_id)];
    reset_ctx->config = (const struct mod_reset_domain_dev_config *)data;

    return FWK_SUCCESS;
}


static int rd_bind(fwk_id_t id, unsigned int round)
{
    struct rd_dev_ctx *reset_ctx = NULL;

    /* Nothing to do but during the first round of calls */
    if (round != 0) {
        return FWK_SUCCESS;
    }

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_LOG),
                               FWK_ID_API(FWK_MODULE_IDX_LOG, 0),
                               &module_ctx.log_api);
    }

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_SUCCESS;
    }

    reset_ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(id);

    return fwk_module_bind(reset_ctx->config->driver_id,
                           reset_ctx->config->api_id,
                           &reset_ctx->driver_api);
}

static int rd_process_bind_request(fwk_id_t source_id, fwk_id_t target_id,
                                      fwk_id_t api_id, const void **api)
{
    if (fwk_id_get_api_idx(api_id) != MOD_RESET_DOMAIN_API_TYPE_HAL) {
        return FWK_E_ACCESS;
    }

    *api = &reset_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_reset_domain = {
    .name = "Reset domain",
    .type = FWK_MODULE_TYPE_HAL,
    .api_count = MOD_RESET_DOMAIN_API_COUNT,
    .init = rd_init,
    .element_init = rd_elt_init,
    .bind = rd_bind,
    .process_bind_request = rd_process_bind_request,
};

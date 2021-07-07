/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_voltage_domain.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Device context */
struct voltd_dev_ctx {
    /* Pointer to the element configuration data */
    const struct mod_voltd_dev_config *config;

    /* Driver API */
    struct mod_voltd_drv_api *api;
};

/* Module context */
struct voltd_ctx {
    /* Table of elements context */
    struct voltd_dev_ctx *dev_ctx_table;
};

/* Expect a single module describing all agents */
static struct voltd_ctx module_ctx;

/*
 * Utility functions
 */

static void get_ctx(fwk_id_t voltd_id, struct voltd_dev_ctx **ctx)
{
    fwk_assert(fwk_module_is_valid_element_id(voltd_id));

    *ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(voltd_id)];
}

/*
 * Module API functions
 */

static int voltd_set_level(fwk_id_t voltd_id, int32_t level_uv)
{
    struct voltd_dev_ctx *ctx;

    get_ctx(voltd_id, &ctx);

    if (!ctx->api->set_level)
        return FWK_E_SUPPORT;

    return ctx->api->set_level(ctx->config->driver_id, level_uv);
}

static int voltd_get_level(fwk_id_t voltd_id, int32_t *level_uv)
{
    struct voltd_dev_ctx *ctx;

    get_ctx(voltd_id, &ctx);

    if (level_uv == NULL)
        return FWK_E_PARAM;

    if (!ctx->api->get_level)
        return FWK_E_SUPPORT;

    return ctx->api->get_level(ctx->config->driver_id, level_uv);
}

static int voltd_set_config(
    fwk_id_t voltd_id,
    uint8_t mode_type,
    uint8_t mode_id)
{
    struct voltd_dev_ctx *ctx;

    get_ctx(voltd_id, &ctx);

    if (!ctx->api->set_config)
        return FWK_E_SUPPORT;

    return ctx->api->set_config(ctx->config->driver_id, mode_type, mode_id);
}

static int voltd_get_config(
    fwk_id_t voltd_id,
    uint8_t *mode_type,
    uint8_t *mode_id)
{
    struct voltd_dev_ctx *ctx;

    get_ctx(voltd_id, &ctx);

    if (mode_id == NULL)
        return FWK_E_PARAM;

    if (!ctx->api->get_config)
        return FWK_E_SUPPORT;

    return ctx->api->get_config(ctx->config->driver_id, mode_type, mode_id);
}

static int voltd_get_info(fwk_id_t voltd_id, struct mod_voltd_info *info)
{
    int status;
    struct voltd_dev_ctx *ctx;

    get_ctx(voltd_id, &ctx);

    fwk_assert(info);

    status = ctx->api->get_info(ctx->config->driver_id, info);

    if (status == FWK_SUCCESS && info->name == NULL)
        info->name = fwk_module_get_element_name(voltd_id);

    return status;
}

static int voltd_get_level_from_index(
    fwk_id_t dev_id,
    unsigned int index,
    int32_t *level_uv)
{
    struct voltd_dev_ctx *ctx = NULL;

    get_ctx(dev_id, &ctx);

    if (level_uv == NULL)
        return FWK_E_PARAM;

    return ctx->api->get_level_from_index(ctx->config->driver_id, index,
                                          level_uv);
}

static const struct mod_voltd_api voltd_api = {
    .get_config = voltd_get_config,
    .set_config = voltd_set_config,
    .get_level = voltd_get_level,
    .set_level = voltd_set_level,
    .get_info = voltd_get_info,
    .get_level_from_index = voltd_get_level_from_index,
};

/*
 * Framework handler functions
 */

static int voltd_init(fwk_id_t module_id, unsigned int element_count,
                      const void *data)
{
    if (element_count == 0)
        return FWK_SUCCESS;

    module_ctx.dev_ctx_table = fwk_mm_calloc(element_count,
                                             sizeof(struct voltd_dev_ctx));
    return FWK_SUCCESS;
}

static int voltd_dev_init(fwk_id_t element_id, unsigned int sub_element_count,
                          const void *data)
{
    struct voltd_dev_ctx *ctx;
    const struct mod_voltd_dev_config *dev_config = data;

    ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(element_id)];
    ctx->config = dev_config;

    return FWK_SUCCESS;
}

static int voltd_bind(fwk_id_t id, unsigned int round)
{
    struct voltd_dev_ctx *ctx;

    if (round == 1)
        return FWK_SUCCESS;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        /* Only element binding is supported */
        return FWK_SUCCESS;
    }

    ctx = &module_ctx.dev_ctx_table[fwk_id_get_element_idx(id)];

    return fwk_module_bind(ctx->config->driver_id,
                           ctx->config->api_id,
                           &ctx->api);
}

static int voltd_process_bind_request(fwk_id_t source_id, fwk_id_t target_id,
                                      fwk_id_t api_id, const void **api)
{
    enum mod_voltd_api_type api_type = fwk_id_get_api_idx(api_id);

    switch (api_type) {
    case MOD_VOLTD_API_TYPE_HAL:
        *api = &voltd_api;
        break;
    default:
        return FWK_E_ACCESS;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_voltage_domain = {
    .type = FWK_MODULE_TYPE_HAL,
    .api_count = MOD_VOLTD_API_COUNT,
    .init = voltd_init,
    .element_init = voltd_dev_init,
    .bind = voltd_bind,
    .process_bind_request = voltd_process_bind_request,
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_psu.h>
#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>

static struct mod_psu_ctx {
    struct mod_psu_element_ctx {
        const struct mod_psu_driver_api *driver;
    } *elements;
} mod_psu_ctx;

static struct mod_psu_element_ctx *mod_psu_get_element_ctx(fwk_id_t element_id)
{
    unsigned int element_idx = fwk_id_get_element_idx(element_id);

    return &mod_psu_ctx.elements[element_idx];
}

static int mod_psu_check_call(fwk_id_t element_id)
{
    int status = FWK_E_PARAM;

    if (fwk_id_get_module_idx(element_id) != FWK_MODULE_IDX_PSU)
        goto exit;

    status = fwk_module_check_call(element_id);
    if (status != FWK_SUCCESS)
        status = FWK_E_STATE;

exit:
    return status;
}

static int mod_psu_get_cfg_ctx(
    fwk_id_t element_id,
    const struct mod_psu_element_cfg **cfg,
    struct mod_psu_element_ctx **ctx)
{
    int status;

    status = mod_psu_check_call(element_id);
    if (status != FWK_SUCCESS)
        goto exit;

    if (ctx != NULL)
        *ctx = mod_psu_get_element_ctx(element_id);

    if (cfg != NULL)
        *cfg = fwk_module_get_data(element_id);

exit:
    return status;
}

static int mod_psu_get_enabled(fwk_id_t element_id, bool *enabled)
{
    int status = FWK_E_STATE;

    const struct mod_psu_element_cfg *cfg;
    struct mod_psu_element_ctx *ctx;

    status = mod_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status != FWK_SUCCESS)
        goto exit;

    status = ctx->driver->get_enabled(cfg->driver_id, enabled);

exit:
    return status;
}

static int mod_psu_set_enabled(fwk_id_t element_id, bool enabled)
{
    int status = FWK_E_STATE;

    const struct mod_psu_element_cfg *cfg;
    struct mod_psu_element_ctx *ctx;

    status = mod_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status != FWK_SUCCESS)
        goto exit;

    status = ctx->driver->set_enabled(cfg->driver_id, enabled);

exit:
    return status;
}

static int mod_psu_get_voltage(fwk_id_t element_id, uint64_t *voltage)
{
    int status = FWK_E_STATE;

    const struct mod_psu_element_cfg *cfg;
    struct mod_psu_element_ctx *ctx;

    status = mod_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status != FWK_SUCCESS)
        goto exit;

    status = ctx->driver->get_voltage(cfg->driver_id, voltage);

exit:
    return status;
}

static int mod_psu_set_voltage(fwk_id_t element_id, uint64_t voltage)
{
    int status = FWK_E_STATE;

    const struct mod_psu_element_cfg *cfg;
    struct mod_psu_element_ctx *ctx;

    status = mod_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status != FWK_SUCCESS)
        goto exit;

    status = ctx->driver->set_voltage(cfg->driver_id, voltage);

exit:
    return status;
}

static const struct mod_psu_device_api mod_psu_device_api = {
    .get_enabled = mod_psu_get_enabled,
    .set_enabled = mod_psu_set_enabled,
    .get_voltage = mod_psu_get_voltage,
    .set_voltage = mod_psu_set_voltage,
};

static int mod_psu_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    fwk_expect(data == NULL);

    mod_psu_ctx.elements =
        fwk_mm_calloc(element_count, sizeof(mod_psu_ctx.elements[0]));
    if (mod_psu_ctx.elements == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int mod_psu_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    fwk_expect(sub_element_count == 0);

    return FWK_SUCCESS;
}

static int mod_psu_bind_element(fwk_id_t element_id, unsigned int round)
{
    int status = FWK_SUCCESS;

    const struct mod_psu_element_ctx *ctx;
    const struct mod_psu_element_cfg *cfg;

    if (round > 0)
        goto exit;

    ctx = mod_psu_get_element_ctx(element_id);
    cfg = fwk_module_get_data(element_id);

    fwk_assert(ctx != NULL);
    fwk_assert(cfg != NULL);

    status = fwk_module_bind(cfg->driver_id, cfg->driver_api_id, &ctx->driver);
    if (status != FWK_SUCCESS)
        goto exit;

    if ((ctx->driver->set_enabled == NULL) ||
        (ctx->driver->get_enabled == NULL) ||
        (ctx->driver->set_voltage == NULL) ||
        (ctx->driver->get_voltage == NULL)) {
        status = FWK_E_PANIC;
    }

exit:
    return status;
}

static int mod_psu_bind(fwk_id_t id, unsigned int round)
{
    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return mod_psu_bind_element(id, round);

    return FWK_SUCCESS;
}

static int mod_psu_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_PARAM;

    if (!fwk_id_is_equal(api_id, mod_psu_api_id_device))
        return FWK_E_PARAM;

    *api = &mod_psu_device_api;

    return FWK_SUCCESS;
}

/* Module description */
const struct fwk_module module_psu = {
    .name = "psu",
    .type = FWK_MODULE_TYPE_HAL,

    .init = mod_psu_init,
    .element_init = mod_psu_element_init,
    .bind = mod_psu_bind,

    .api_count = MOD_PSU_API_IDX_COUNT,
    .process_bind_request = mod_psu_process_bind_request,
};

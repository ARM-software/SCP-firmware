/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_mock_psu.h>
#include <mod_psu.h>
#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static struct mod_mock_psu_ctx {
    struct mod_mock_psu_element_ctx {
        bool enabled;
        uint64_t voltage;
    } *elements;
} mod_mock_psu_ctx;

static struct mod_mock_psu_element_ctx *mod_mock_psu_get_ctx_unchecked(
    fwk_id_t element_id)
{
    unsigned int element_idx = fwk_id_get_element_idx(element_id);

    return &mod_mock_psu_ctx.elements[element_idx];
}

static struct mod_mock_psu_element_ctx *mod_mock_psu_get_element_ctx(
    fwk_id_t element_id)
{
    if (fwk_id_get_module_idx(element_id) != FWK_MODULE_IDX_MOCK_PSU)
        return NULL;
    else if (!fwk_module_is_valid_element_id(element_id))
        return NULL;
    else
        return mod_mock_psu_get_ctx_unchecked(element_id);
}

static int mod_mock_psu_get_cfg_ctx(
    fwk_id_t element_id,
    const struct mod_mock_psu_element_cfg **cfg,
    struct mod_mock_psu_element_ctx **ctx)
{
    int status;

    status = fwk_module_check_call(element_id);
    if (status != FWK_SUCCESS) {
        status = FWK_E_STATE;

        goto exit;
    }

    if (ctx != NULL) {
        *ctx = mod_mock_psu_get_element_ctx(element_id);
        if (ctx == NULL) {
            status = FWK_E_PARAM;

            goto exit;
        }
    }

    if (cfg != NULL) {
        *cfg = fwk_module_get_data(element_id);
        fwk_assert(cfg != NULL);
    }

exit:
    return status;
}

static int mod_mock_psu_get_enabled(fwk_id_t device_id, bool *enabled)
{
    int status;

    struct mod_mock_psu_element_ctx *ctx;

    status = mod_mock_psu_get_cfg_ctx(device_id, NULL, &ctx);
    if (status == FWK_SUCCESS)
        *enabled = ctx->enabled;

    return status;
}

static int mod_mock_psu_set_enabled(fwk_id_t device_id, bool enabled)
{
    int status;

    struct mod_mock_psu_element_ctx *ctx;

    status = mod_mock_psu_get_cfg_ctx(device_id, NULL, &ctx);
    if (status == FWK_SUCCESS)
        ctx->enabled = enabled;

    return status;
}

static int mod_mock_psu_get_voltage(fwk_id_t device_id, uint64_t *voltage)
{
    int status;

    struct mod_mock_psu_element_ctx *ctx;

    status = mod_mock_psu_get_cfg_ctx(device_id, NULL, &ctx);
    if (status == FWK_SUCCESS)
        *voltage = ctx->voltage;

    return status;
}

static int mod_mock_psu_set_voltage(fwk_id_t device_id, uint64_t voltage)
{
    int status;

    struct mod_mock_psu_element_ctx *ctx;

    status = mod_mock_psu_get_cfg_ctx(device_id, NULL, &ctx);
    if (status == FWK_SUCCESS)
        ctx->voltage = voltage;

    return status;
}

static const struct mod_psu_driver_api mod_mock_psu_driver_api = {
    .get_enabled = mod_mock_psu_get_enabled,
    .set_enabled = mod_mock_psu_set_enabled,
    .get_voltage = mod_mock_psu_get_voltage,
    .set_voltage = mod_mock_psu_set_voltage,
};

static int mod_mock_psu_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    fwk_expect(data == NULL);

    mod_mock_psu_ctx.elements =
        fwk_mm_calloc(element_count, sizeof(mod_mock_psu_ctx.elements[0]));
    if (mod_mock_psu_ctx.elements == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int mod_mock_psu_element_init(
    fwk_id_t device_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_mock_psu_element_ctx *ctx;
    const struct mod_mock_psu_element_cfg *cfg = data;

    fwk_expect(sub_element_count == 0);

    ctx = mod_mock_psu_get_ctx_unchecked(device_id);

    ctx->enabled = cfg->default_enabled;
    ctx->voltage = cfg->default_voltage;

    return FWK_SUCCESS;
}

static int mod_mock_psu_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    /* Only accept binds to the elements */
    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_PARAM;

    *api = &mod_mock_psu_driver_api;

    return FWK_SUCCESS;
}

/* Module description */
const struct fwk_module module_mock_psu = {
    .name = "mock_psu",
    .type = FWK_MODULE_TYPE_DRIVER,

    .init = mod_mock_psu_init,
    .element_init = mod_mock_psu_element_init,

    .api_count = MOD_MOCK_PSU_API_IDX_COUNT,
    .process_bind_request = mod_mock_psu_process_bind_request,
};

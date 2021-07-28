/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_rcar_pmic_private.h>

#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/* Device context table */
static struct mod_rcar_pmic_device_ctx (*device_ctx)[];

static int rcar_pmic_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    device_ctx = fwk_mm_calloc(element_count, sizeof((*device_ctx)[0]));
    if (device_ctx == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int rcar_pmic_element_init(
    fwk_id_t device_id,
    unsigned int sub_element_count,
    const void *data)
{
    assert(sub_element_count == 0);

    __mod_rcar_pmic_get_device_ctx(device_id)->config = data;

    return FWK_SUCCESS;
}

static int rcar_pmic_bind_element(fwk_id_t device_id, unsigned int round)
{
    int status;
    const struct mod_rcar_pmic_device_ctx *ctx;

    /* Only handle the first round */
    if (round > 0)
        return FWK_SUCCESS;

    ctx = __mod_rcar_pmic_get_device_ctx(device_id);

    /* Bind to the driver */
    status = fwk_module_bind(
        ctx->config->driver_id, ctx->config->driver_api_id, &ctx->apis.driver);
    if (status != FWK_SUCCESS) {
        assert(false);

        return FWK_E_PANIC;
    }

    assert(ctx->apis.driver->set_enabled != NULL);
    assert(ctx->apis.driver->get_enabled != NULL);
    assert(ctx->apis.driver->set_voltage != NULL);
    assert(ctx->apis.driver->get_voltage != NULL);

    return FWK_SUCCESS;
}

static int rcar_pmic_bind(fwk_id_t id, unsigned int round)
{
    /* We only need to handle element binding */
    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return rcar_pmic_bind_element(id, round);

    return FWK_SUCCESS;
}

static int rcar_pmic_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    /* Only accept binds to the elements */
    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_PARAM;

    /* Only expose the device API */
    if (!fwk_id_is_equal(api_id, mod_rcar_pmic_api_id_device))
        return FWK_E_PARAM;

    *api = &__mod_rcar_pmic_device_api;

    return FWK_SUCCESS;
}

struct mod_rcar_pmic_device_ctx *__mod_rcar_pmic_get_device_ctx(
    fwk_id_t device_id)
{
    unsigned int element_idx = fwk_id_get_element_idx(device_id);

    return &(*device_ctx)[element_idx];
}

struct mod_rcar_pmic_device_ctx *__mod_rcar_pmic_get_valid_device_ctx(
    fwk_id_t device_id)
{
    return __mod_rcar_pmic_get_device_ctx(device_id);
}

/* Module description */
const struct fwk_module module_rcar_pmic = {
    .type = FWK_MODULE_TYPE_HAL,
    .init = rcar_pmic_init,
    .element_init = rcar_pmic_element_init,
    .bind = rcar_pmic_bind,
    .process_bind_request = rcar_pmic_process_bind_request,
    .process_event = __mod_rcar_pmic_process_event,
    .api_count = MOD_RCAR_PMIC_API_IDX_COUNT,
    .event_count = MOD_RCAR_PMIC_EVENT_IDX_COUNT,
};

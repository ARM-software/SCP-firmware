/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_mm.h>
#include <mod_psu.h>

/* Device context */
struct mod_psu_device_ctx {
    /* Device configuration */
    const struct mod_psu_device_config *config;

    struct {
        /* Driver API */
        const struct mod_psu_driver_api *driver;
    } apis;
};

/* Device context table */
static struct mod_psu_device_ctx (*device_ctx)[];

static struct mod_psu_device_ctx *mod_psu_get_device_ctx(fwk_id_t device_id)
{
    unsigned int element_idx = fwk_id_get_element_idx(device_id);

    return &(*device_ctx)[element_idx];
}

static struct mod_psu_device_ctx *mod_psu_get_valid_device_ctx(
    fwk_id_t device_id)
{
    if (fwk_module_check_call(device_id) != FWK_SUCCESS)
        return NULL;

    return mod_psu_get_device_ctx(device_id);
}

static int api_get_enabled(fwk_id_t device_id, bool *enabled)
{
    int status;
    const struct mod_psu_device_ctx *ctx;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_PSU)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to a valid element */
    if (!fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    /* Validate the API call */
    status = fwk_module_check_call(device_id);
    if (status != FWK_SUCCESS)
        return FWK_E_STATE;

    ctx = mod_psu_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    /* Get the enabled state from the driver */
    status = ctx->apis.driver->get_enabled(ctx->config->driver_id, enabled);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int api_set_enabled(fwk_id_t device_id, bool enable)
{
    int status;
    const struct mod_psu_device_ctx *ctx;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_PSU)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to a valid element */
    if (!fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    /* Validate the API call */
    status = fwk_module_check_call(device_id);
    if (status != FWK_SUCCESS)
        return FWK_E_STATE;

    ctx = mod_psu_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    /* Set the enabled state through the driver */
    status = ctx->apis.driver->set_enabled(ctx->config->driver_id, enable);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int api_get_voltage(fwk_id_t device_id, uintmax_t *voltage)
{
    int status;
    const struct mod_psu_device_ctx *ctx;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_PSU)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to a valid element */
    if (!fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    /* Validate the API call */
    status = fwk_module_check_call(device_id);
    if (status != FWK_SUCCESS)
        return FWK_E_STATE;

    ctx = mod_psu_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    /* Get the voltage from the driver */
    status = ctx->apis.driver->get_voltage(ctx->config->driver_id, voltage);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int api_set_voltage(fwk_id_t device_id, uintmax_t voltage)
{
    int status;
    const struct mod_psu_device_ctx *ctx;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_PSU)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to a valid element */
    if (!fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    /* Validate the API call */
    status = fwk_module_check_call(device_id);
    if (status != FWK_SUCCESS)
        return FWK_E_STATE;

    ctx = mod_psu_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    /* Set the voltage state through the driver */
    status = ctx->apis.driver->set_voltage(ctx->config->driver_id, voltage);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

/* Module API implementation */
static const struct mod_psu_device_api mod_psu_device_api = {
    .get_enabled = api_get_enabled,
    .set_enabled = api_set_enabled,
    .get_voltage = api_get_voltage,
    .set_voltage = api_set_voltage,
};

static int psu_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    device_ctx = fwk_mm_calloc(
        element_count,
        sizeof((*device_ctx)[0]));
    if (device_ctx == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int psu_element_init(
    fwk_id_t device_id,
    unsigned int sub_element_count,
    const void *data)
{
    assert(sub_element_count == 0);

    mod_psu_get_device_ctx(device_id)->config = data;

    return FWK_SUCCESS;
}

static int psu_bind_element(fwk_id_t device_id, unsigned int round)
{
    int status;
    const struct mod_psu_device_ctx *ctx;

    /* Only handle the first round */
    if (round > 0)
        return FWK_SUCCESS;

    ctx = mod_psu_get_device_ctx(device_id);

    /* Bind to the driver */
    status = fwk_module_bind(
        ctx->config->driver_id,
        ctx->config->driver_api_id,
        &ctx->apis.driver);
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

static int psu_bind(fwk_id_t id, unsigned int round)
{
    /* We only need to handle element binding */
    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return psu_bind_element(id, round);

    return FWK_SUCCESS;
}

static int psu_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    /* Only accept binds to the elements */
    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_PARAM;

    /* Only expose the device API */
    if (!fwk_id_is_equal(api_id, mod_psu_api_id_psu_device))
        return FWK_E_PARAM;

    *api = &mod_psu_device_api;

    return FWK_SUCCESS;
}

/* Module description */
const struct fwk_module module_psu = {
    .name = "PSU",
    .type = FWK_MODULE_TYPE_HAL,
    .init = psu_init,
    .element_init = psu_element_init,
    .bind = psu_bind,
    .process_bind_request = psu_process_bind_request,
    .api_count = MOD_PSU_API_IDX_COUNT,
};

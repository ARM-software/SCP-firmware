/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2022, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_rcar_pmic_private.h>

#include <mod_psu.h>

#include <fwk_core.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static int api_get_enabled(fwk_id_t device_id, bool *enabled)
{
    int status;
    const struct mod_rcar_pmic_device_ctx *ctx;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_RCAR_PMIC)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to a valid element */
    if (!fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    ctx = __mod_rcar_pmic_get_valid_device_ctx(device_id);
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
    const struct mod_rcar_pmic_device_ctx *ctx;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_RCAR_PMIC)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to a valid element */
    if (!fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    ctx = __mod_rcar_pmic_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    /* Set the enabled state through the driver */
    status = ctx->apis.driver->set_enabled(ctx->config->driver_id, enable);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int api_get_voltage(fwk_id_t device_id, uint32_t *voltage)
{
    int status;
    const struct mod_rcar_pmic_device_ctx *ctx;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_RCAR_PMIC)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to a valid element */
    if (!fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    ctx = __mod_rcar_pmic_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    /* Get the voltage from the driver */
    status = ctx->apis.driver->get_voltage(ctx->config->driver_id, voltage);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int api_set_voltage(fwk_id_t device_id, uint32_t voltage)
{
    int status;
    const struct mod_rcar_pmic_device_ctx *ctx;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_RCAR_PMIC)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to a valid element */
    if (!fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    ctx = __mod_rcar_pmic_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    /* Set the voltage state through the driver */
    status = ctx->apis.driver->set_voltage(ctx->config->driver_id, voltage);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int api_set_pmic(fwk_id_t device_id, unsigned int state)
{
    int status;
    const struct mod_rcar_pmic_device_ctx *ctx;

    /* This API call cannot target another module */
    if (fwk_id_get_module_idx(device_id) != FWK_MODULE_IDX_RCAR_PMIC)
        return FWK_E_PARAM;

    /* Ensure the identifier refers to a valid element */
    if (!fwk_module_is_valid_element_id(device_id))
        return FWK_E_PARAM;

    ctx = __mod_rcar_pmic_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    status = ctx->apis.driver->set_pmic(ctx->config->driver_id, state);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

/* Module API implementation */
const struct mod_rcar_pmic_device_api __mod_rcar_pmic_device_api = {
    .get_enabled = api_get_enabled,
    .set_enabled = api_set_enabled,
    .get_voltage = api_get_voltage,
    .set_voltage = api_set_voltage,
    .set_pmic = api_set_pmic,
};

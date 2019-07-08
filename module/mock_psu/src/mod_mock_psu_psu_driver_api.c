/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <fwk_id.h>
#include <mod_mock_psu_private.h>

static int api_set_enabled(fwk_id_t device_id, bool enable)
{
    struct mod_mock_psu_device_ctx *ctx;

    ctx = __mod_mock_psu_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    ctx->enabled = enable;

    return FWK_SUCCESS;
}

static int api_get_enabled(fwk_id_t device_id, bool *enabled)
{
    struct mod_mock_psu_device_ctx *ctx;

    ctx = __mod_mock_psu_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *enabled = ctx->enabled;

    return FWK_SUCCESS;
}

static int api_set_voltage(fwk_id_t device_id, uint64_t voltage)
{
    struct mod_mock_psu_device_ctx *ctx;

    ctx = __mod_mock_psu_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    ctx->voltage = voltage;

    return FWK_SUCCESS;
}

static int api_get_voltage(fwk_id_t device_id, uint64_t *voltage)
{
    struct mod_mock_psu_device_ctx *ctx;

    ctx = __mod_mock_psu_get_valid_device_ctx(device_id);
    if (ctx == NULL)
        return FWK_E_PARAM;

    *voltage = ctx->voltage;

    return FWK_SUCCESS;
}

const struct mod_psu_driver_api __mod_mock_psu_psu_driver_api = {
    .set_enabled = api_set_enabled,
    .get_enabled = api_get_enabled,
    .set_voltage = api_set_voltage,
    .get_voltage = api_get_voltage,
};

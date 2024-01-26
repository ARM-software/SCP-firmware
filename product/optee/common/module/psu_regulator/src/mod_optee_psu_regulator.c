/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, STMicroelectronics and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <drivers/regulator.h>

#include <mod_optee_psu_regulator.h>
#include <mod_psu.h>

#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>

#include <stddef.h>

#define MOD_PREFIX "[optee-psu-regulator] "

/* Device context */
struct optee_psu_regulator_dev_ctx {
    struct regulator *regulator;
};

/* Module context */
struct optee_psu_regulator_ctx {
    struct optee_psu_regulator_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
};

/* A single instance handles all voltage regulators abstracted by regulator.h */
static struct optee_psu_regulator_ctx module_ctx;

static struct optee_psu_regulator_dev_ctx *get_ctx(fwk_id_t dev_id)
{
    if (fwk_module_is_valid_element_id(dev_id)) {
        return module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
    }

    return NULL;
}

static int optee_psu_regulator_set_enabled(fwk_id_t dev_id, bool enabled)
{
    struct optee_psu_regulator_dev_ctx *ctx;

    ctx = get_ctx(dev_id);
    if (ctx == NULL) {
        return FWK_E_PARAM;
    }
    if (ctx->regulator == NULL) {
        return FWK_E_ACCESS;
    }

    FWK_LOG_DEBUG(
        MOD_PREFIX "Set %s %s",
        regulator_name(ctx->regulator),
        .enabled ? "ON" : "OFF");

    if (enabled) {
        if (regulator_enable(ctx->regulator) != TEE_SUCCESS) {
            return FWK_E_DEVICE;
        }
    } else {
        if (regulator_disable(ctx->regulator) != TEE_SUCCESS) {
            return FWK_E_DEVICE;
        }
    }

    return FWK_SUCCESS;
}

static int optee_psu_regulator_get_enabled(fwk_id_t dev_id, bool *enabled)
{
    struct optee_psu_regulator_dev_ctx *ctx;

    ctx = get_ctx(dev_id);
    if ((ctx == NULL) || (enabled == NULL)) {
        return FWK_E_PARAM;
    }
    if (ctx->regulator == NULL) {
        return FWK_E_ACCESS;
    }

    *enabled = regulator_is_enabled(ctx->regulator);

    FWK_LOG_DEBUG(
        MOD_PREFIX "Get %s state: %s",
        regulator_name(ctx->regulator),
        enabled ? "ON" : "OFF");

    return FWK_SUCCESS;
}

static int optee_psu_regulator_set_voltage(fwk_id_t dev_id, uint32_t voltage_mv)
{
    struct optee_psu_regulator_dev_ctx *ctx;
    int voltage_uv;

    ctx = get_ctx(dev_id);
    if ((ctx == NULL) || (voltage_mv > INT_MAX)) {
        return FWK_E_PARAM;
    }
    if (ctx->regulator == NULL) {
        return FWK_E_ACCESS;
    }

    FWK_LOG_DEBUG(
        MOD_PREFIX "Set regulator %s level: %" PRIu32 " mV",
        regulator_name(ctx->regulator),
        voltage_mv);

    voltage_uv = (int)voltage_mv * 1000;

    if (regulator_set_voltage(ctx->regulator, voltage_uv) != TEE_SUCCESS) {
        return FWK_E_DEVICE;
    } else {
        return FWK_SUCCESS;
    }
}

static int optee_psu_regulator_get_voltage(fwk_id_t dev_id, uint32_t *voltage)
{
    struct optee_psu_regulator_dev_ctx *ctx;
    int voltage_uv;

    ctx = get_ctx(dev_id);
    if ((ctx == NULL) || (voltage == NULL)) {
        return FWK_E_PARAM;
    }
    if (ctx->regulator == NULL) {
        return FWK_E_ACCESS;
    }

    voltage_uv = regulator_get_voltage(ctx->regulator);
    if ((voltage_uv < 0) || (voltage_uv > INT32_MAX)) {
        return FWK_E_DEVICE;
    }

    *voltage = (uint32_t)voltage_uv / 1000;

    return FWK_SUCCESS;
}

static struct mod_psu_driver_api psu_driver_api = {
    .set_enabled = optee_psu_regulator_set_enabled,
    .get_enabled = optee_psu_regulator_get_enabled,
    .set_voltage = optee_psu_regulator_set_voltage,
    .get_voltage = optee_psu_regulator_get_voltage,
};

static int optee_psu_regulator_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (element_count == 0) {
        return FWK_E_PARAM;
    }

    module_ctx.dev_count = element_count;
    module_ctx.dev_ctx_table =
        fwk_mm_calloc(element_count, sizeof(*module_ctx.dev_ctx_table));

    return FWK_SUCCESS;
}

static int optee_psu_regulator_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    const struct mod_optee_psu_regulator_dev_config *dev_config;
    struct optee_psu_regulator_dev_ctx *ctx;

    ctx = get_ctx(element_id);
    if (ctx == NULL) {
        return FWK_E_PANIC;
    }

    dev_config = data;
    ctx->regulator = (struct regulator *)dev_config->regulator;

    return FWK_SUCCESS;
}

static int optee_psu_regulator_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &psu_driver_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_optee_psu_regulator = {
    .api_count = 1,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = optee_psu_regulator_init,
    .element_init = optee_psu_regulator_element_init,
    .process_bind_request = optee_psu_regulator_process_bind_request,
};

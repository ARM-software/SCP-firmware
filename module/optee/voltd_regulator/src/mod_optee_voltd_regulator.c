/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>

#include <mod_scmi_std.h>
#include <mod_voltage_domain.h>
#include <mod_optee_voltd_regulator.h>

#include <stddef.h>

#include <drivers/regulator.h>

#define MOD_PREFIX "[optee-voltd-regulator] "

/* Device context */
struct optee_voltd_regulator_dev_ctx {
    struct regulator *regulator;
    bool enabled;
};

/* Module context */
struct optee_voltd_regulator_ctx {
    struct optee_voltd_regulator_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
};

/* A single instance handles all voltage regulators abstracted by regulator.h */
static struct optee_voltd_regulator_ctx module_ctx;

static struct optee_voltd_regulator_dev_ctx *get_ctx(fwk_id_t dev_id)
{
    if (fwk_module_is_valid_element_id(dev_id)) {
        return module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);
    }

    return NULL;
}

static int optee_voltd_regulator_get_config(fwk_id_t dev_id,
                                            uint8_t *mode_type,
                                            uint8_t *mode_id)
{
    struct optee_voltd_regulator_dev_ctx *ctx;

    ctx = get_ctx(dev_id);
    if (ctx == NULL) {
        return FWK_E_PARAM;
    }

    if (ctx->regulator == NULL) {
        /* Treat unexposed voltage domain a stubbed 0V fixed level regulator */
        *mode_type = MOD_VOLTD_MODE_TYPE_ARCH;
        *mode_id = MOD_VOLTD_MODE_ID_OFF;

        return FWK_SUCCESS;
    }

    *mode_type = MOD_VOLTD_MODE_TYPE_ARCH;

    if (ctx->enabled) {
        *mode_id = MOD_VOLTD_MODE_ID_ON;
    } else {
        *mode_id = MOD_VOLTD_MODE_ID_OFF;
    }

    FWK_LOG_DEBUG(
        MOD_PREFIX "Get config for %u/%s: %#"PRIx8", %#"PRIx8,
        fwk_id_get_element_idx(dev_id), regulator_name(ctx->regulator),
        *mode_type, *mode_id);

    return FWK_SUCCESS;
}

static int optee_voltd_regulator_set_config(fwk_id_t dev_id,
                                            uint8_t mode_type,
                                            uint8_t mode_id)
{
    struct optee_voltd_regulator_dev_ctx *ctx;

    ctx = get_ctx(dev_id);
    if (ctx == NULL) {
        return FWK_E_PARAM;
    }

    if (mode_type != MOD_VOLTD_MODE_TYPE_ARCH) {
        return FWK_E_PARAM;
    }

    if (ctx->regulator == NULL) {
        /* Treat unexposed voltage domain a stubbed 0V fixed level regulator */
        if (mode_id == MOD_VOLTD_MODE_ID_OFF) {
            return FWK_SUCCESS;
        } else {
            return FWK_E_ACCESS;
        }
    }

    switch (mode_id) {
    case MOD_VOLTD_MODE_ID_ON:
        if (!ctx->enabled) {
            if (regulator_enable(ctx->regulator) != TEE_SUCCESS) {
                return FWK_E_DEVICE;
            }

            ctx->enabled = true;
        }
        break;

    case MOD_VOLTD_MODE_ID_OFF:
        if (ctx->enabled) {
            if (regulator_disable(ctx->regulator) != TEE_SUCCESS) {
                return FWK_E_DEVICE;
            }

            ctx->enabled = false;
        }
        break;

    default:
        return FWK_E_PARAM;
    }

    FWK_LOG_DEBUG(
        MOD_PREFIX "Set config for %u/%s: type %#"PRIx8" / mode %#"PRIx8,
        fwk_id_get_element_idx(dev_id), regulator_name(ctx->regulator),
        mode_type, mode_id);

    return FWK_SUCCESS;
}

static int optee_voltd_regulator_get_level(fwk_id_t dev_id,
                                           int *level_uv)
{
    struct optee_voltd_regulator_dev_ctx *ctx;

    ctx = get_ctx(dev_id);
    if (ctx == NULL) {
        return FWK_E_PARAM;
    }

    if (ctx->regulator == NULL) {
        /* Treat unexposed voltage domain a stubbed 0V fixed level regulator */
        *level_uv = 0;

        return FWK_SUCCESS;
    }

    *level_uv = regulator_get_voltage(ctx->regulator);

    FWK_LOG_DEBUG(
        MOD_PREFIX "Get level for %u/%s: %duV",
        fwk_id_get_element_idx(dev_id), regulator_name(ctx->regulator),
        *level_uv);

    return FWK_SUCCESS;
}

static int optee_voltd_regulator_set_level(fwk_id_t dev_id,
                                           int level_uv)
{
    struct optee_voltd_regulator_dev_ctx *ctx;

    ctx = get_ctx(dev_id);
    if (ctx == NULL) {
        return FWK_E_PARAM;
    }

    if (ctx->regulator == NULL) {
        return FWK_E_ACCESS;
    }

    if (regulator_set_voltage(ctx->regulator, level_uv) != TEE_SUCCESS) {
        return FWK_E_DEVICE;
    }

    FWK_LOG_DEBUG(
        MOD_PREFIX "Set level for %u/%s: %duV",
        fwk_id_get_element_idx(dev_id), regulator_name(ctx->regulator),
        level_uv);

    return FWK_SUCCESS;
}

static int optee_voltd_regulator_get_info(fwk_id_t dev_id,
                                        struct mod_voltd_info *info)
{
    struct optee_voltd_regulator_dev_ctx *ctx;

    ctx = get_ctx(dev_id);
    if (ctx == NULL) {
        return FWK_E_PARAM;
    }

    memset(info, 0, sizeof(*info));

    if (!ctx->regulator) {
        /* Treat unexposed voltage domain a stubbed 0V fixed level regulator */
        static const char reserved[] = "reserved";

        info->name = reserved;
        info->level_range.level_type = MOD_VOLTD_VOLTAGE_LEVEL_DISCRETE;
        info->level_range.level_count = 1;
        info->level_range.min_uv = 0;
        info->level_range.max_uv = 0;
    } else {
        struct regulator_voltages *desc;

        if (regulator_supported_voltages(ctx->regulator, &desc) != TEE_SUCCESS) {
            return FWK_E_SUPPORT;
        }

        if (desc->type == VOLTAGE_TYPE_FULL_LIST) {
            info->name = regulator_name(ctx->regulator);
            info->level_range.level_type = MOD_VOLTD_VOLTAGE_LEVEL_DISCRETE;
            info->level_range.level_count = desc->num_levels;
            info->level_range.min_uv = desc->entries[0];
            info->level_range.max_uv = desc->entries[desc->num_levels - 1];
        } else if (desc->type == VOLTAGE_TYPE_INCREMENT) {
            info->name = regulator_name(ctx->regulator);
            info->level_range.level_type = MOD_VOLTD_VOLTAGE_LEVEL_DISCRETE;
            info->level_range.min_uv = desc->entries[0];
            info->level_range.max_uv = desc->entries[1];
            info->level_range.step_uv = desc->entries[2];
        } else {
            fwk_unreachable();
            return FWK_E_PANIC;
        }
    }

    FWK_LOG_DEBUG(
        MOD_PREFIX "Get info for %u/%s: range [%d %d]",
        fwk_id_get_element_idx(dev_id), info->name,
        info->level_range.min_uv,
        info->level_range.max_uv);

    return FWK_SUCCESS;
}

static int optee_voltd_regulator_level_from_index(fwk_id_t dev_id,
                                                  unsigned int index,
                                                  int32_t *level_uv)
{
    struct optee_voltd_regulator_dev_ctx *ctx;
    struct regulator_voltages *desc;

    ctx = get_ctx(dev_id);
    if (ctx == NULL) {
        return FWK_E_PARAM;
    }

    if (ctx->regulator == NULL) {
        /* Treat unexposed voltage domain a stubbed 0V fixed level regulator */
        if (index > 0) {
            return FWK_E_RANGE;
        }

        *level_uv = 0;

        return FWK_SUCCESS;
    }

    if (regulator_supported_voltages(ctx->regulator, &desc) != TEE_SUCCESS) {
        return FWK_E_SUPPORT;
    }

    if (desc->type != VOLTAGE_TYPE_FULL_LIST) {
        return FWK_E_DATA;
    }
    if (index >= desc->num_levels) {
        return FWK_E_RANGE;
    }

    *level_uv = (int32_t)desc->entries[index];

    FWK_LOG_DEBUG(
        MOD_PREFIX "Get level from index for %u/%s: index %u, level %"PRId32"uV",
        fwk_id_get_element_idx(dev_id), regulator_name(ctx->regulator),
        index, *level_uv);

    return FWK_SUCCESS;
}

static const struct mod_voltd_drv_api api_optee_regu = {
    .get_level = optee_voltd_regulator_get_level,
    .set_level = optee_voltd_regulator_set_level,
    .set_config = optee_voltd_regulator_set_config,
    .get_config = optee_voltd_regulator_get_config,
    .get_info = optee_voltd_regulator_get_info,
    .get_level_from_index = optee_voltd_regulator_level_from_index,
};

/*
 * Framework handler functions
 */

static int optee_voltd_regulator_init(fwk_id_t module_id,
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

static int optee_voltd_regulator_element_init(fwk_id_t element_id,
                                              unsigned int unused,
                                              const void *data)
{
    const struct mod_optee_voltd_regulator_dev_config *dev_config;
    struct optee_voltd_regulator_dev_ctx *ctx;

    ctx = get_ctx(element_id);
    if (ctx == NULL) {
        return FWK_E_PANIC;
    }

    dev_config = data;
    ctx->regulator = (struct regulator *)dev_config->regulator;
    ctx->enabled = dev_config->default_enabled;

    if (ctx->enabled) {
        if (ctx->regulator == NULL) {
            return FWK_E_PANIC;
        }
        if (regulator_enable(ctx->regulator) != TEE_SUCCESS) {
            return FWK_E_DEVICE;
        }
    }

    return FWK_SUCCESS;
}

static int optee_voltd_regulator_process_bind_request(fwk_id_t requester_id,
                                                      fwk_id_t target_id,
                                                      fwk_id_t api_type,
                                                      const void **api)
{
    *api = &api_optee_regu;

    return FWK_SUCCESS;
}

const struct fwk_module module_optee_voltd_regulator = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = optee_voltd_regulator_init,
    .element_init = optee_voltd_regulator_element_init,
    .process_bind_request = optee_voltd_regulator_process_bind_request,
};

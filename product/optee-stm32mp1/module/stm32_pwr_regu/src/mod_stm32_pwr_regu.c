/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Interface SCP-firmare VOLTD instances with OP-TEE stm32mp1
 *     PWR regulator driver for SoC voltage regulator controller by
 *     OP-TEE for this platform.
 *
 *     The OP-TEE drvier is accessed by API function named
 *     stm32mp1_regultor_*(). They execute in a threaded interruptible,
 *     and rescheduable context.
 */

#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_log.h>

#include <mod_voltage_domain.h>
#include <mod_stm32_pwr_regu.h>

#include <drivers/stm32mp1_pwr.h>
#include <stm32_util.h>

#define MOD_NAME "[STM32 PWR] "

/* Device context */
struct stm32_pwr_regu_dev_ctx {
    enum pwr_regulator pwr_id;
    const char *name;
};

/* Module context */
struct stm32_pwr_regu_ctx {
    struct stm32_pwr_regu_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
};

static struct stm32_pwr_regu_ctx module_ctx;

static bool nsec_can_access_pwr_regu(enum pwr_regulator pwr_id)
{
    /* Currently allow non-secure world to access all PWR regulators */
    return true;
}

static int32_t pwr_regu_level(enum pwr_regulator pwr_id)
{
    return (int32_t)stm32mp1_pwr_regulator_mv(pwr_id) * 1000;
}

/*
 * Voltage domain driver API functions
 */
static int pwr_regu_get_config(fwk_id_t dev_id, uint8_t *mode_type,
                               uint8_t *mode_id)
{
    struct stm32_pwr_regu_dev_ctx *ctx;

    if (!fwk_module_is_valid_element_id(dev_id) ||
        mode_type == NULL || mode_id == NULL) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (!nsec_can_access_pwr_regu(ctx->pwr_id)) {
        return FWK_E_ACCESS;
    }

    *mode_type = MOD_VOLTD_MODE_TYPE_ARCH;

    if (stm32mp1_pwr_regulator_is_enabled(ctx->pwr_id)) {
        *mode_id = MOD_VOLTD_MODE_ID_ON;
    } else {
        *mode_id = MOD_VOLTD_MODE_ID_OFF;
    }

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI voltd %u: get_config PWR#%u = %s",
        fwk_id_get_element_idx(dev_id),
        ctx->pwr_id,
        *mode_id == MOD_VOLTD_MODE_ID_ON ? "on" : "off");

    return FWK_SUCCESS;
}

static int pwr_regu_set_config(fwk_id_t dev_id, uint8_t mode_type,
                               uint8_t mode_id)
{
    struct stm32_pwr_regu_dev_ctx *ctx = NULL;

    if (!fwk_module_is_valid_element_id(dev_id) ||
        mode_type != MOD_VOLTD_MODE_TYPE_ARCH ||
        (mode_id == MOD_VOLTD_MODE_ID_ON && mode_id == MOD_VOLTD_MODE_ID_OFF)) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (!nsec_can_access_pwr_regu(ctx->pwr_id)) {
        return FWK_E_ACCESS;
    }

    stm32mp1_pwr_regulator_set_state(ctx->pwr_id,
                                     mode_id == MOD_VOLTD_MODE_ID_ON);

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI voltd %u: set_config PWR#%u %s",
        fwk_id_get_element_idx(dev_id),
        ctx->pwr_id,
        mode_id == MOD_VOLTD_MODE_ID_ON ? "on" : "off");

    return FWK_SUCCESS;
}

static int pwr_regu_get_level(fwk_id_t dev_id, int *level_uv)
{
    struct stm32_pwr_regu_dev_ctx *ctx = NULL;

    if (!fwk_module_is_valid_element_id(dev_id) || level_uv == NULL) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (!nsec_can_access_pwr_regu(ctx->pwr_id)) {
        return FWK_E_ACCESS;
    }

    *level_uv = pwr_regu_level(ctx->pwr_id);

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI voltd %u: get_level PWR#%u = %d",
        fwk_id_get_element_idx(dev_id),
        ctx->pwr_id,
        *level_uv);

    return FWK_SUCCESS;
}

static int pwr_regu_set_level(fwk_id_t dev_id, int level_uv)
{
    struct stm32_pwr_regu_dev_ctx *ctx = NULL;

    if (!fwk_module_is_valid_element_id(dev_id)) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (!nsec_can_access_pwr_regu(ctx->pwr_id)) {
        return FWK_E_ACCESS;
    }

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI voltd %u: set_level PWR#%u to %d",
        fwk_id_get_element_idx(dev_id),
        ctx->pwr_id,
        level_uv);

    if (level_uv != pwr_regu_level(ctx->pwr_id)) {
        return FWK_E_RANGE;
    }

    return FWK_SUCCESS;
}

static int pwr_regu_get_info(fwk_id_t dev_id, struct mod_voltd_info *info)
{
    struct stm32_pwr_regu_dev_ctx *ctx = NULL;

    if (!fwk_module_is_valid_element_id(dev_id) || info == NULL) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (!nsec_can_access_pwr_regu(ctx->pwr_id)) {
        return FWK_E_ACCESS;
    }

    memset(info, 0, sizeof(*info));
    info->level_range.level_type = MOD_VOLTD_VOLTAGE_LEVEL_DISCRETE;
    info->level_range.min_uv = pwr_regu_level(ctx->pwr_id);
    info->level_range.max_uv = info->level_range.min_uv;
    info->level_range.level_count = 1;
    info->name = ctx->name;

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI voltd %u: get_info PWR#%u",
        fwk_id_get_element_idx(dev_id),
        ctx->pwr_id);

    return FWK_SUCCESS;
}

static int pwr_regu_level_from_index(fwk_id_t dev_id, unsigned int index,
                                     int32_t *level_uv)
{
    struct stm32_pwr_regu_dev_ctx *ctx = NULL;

    if (!fwk_module_is_valid_element_id(dev_id) || level_uv == NULL) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (!nsec_can_access_pwr_regu(ctx->pwr_id)) {
        return FWK_E_ACCESS;
    }

    if (index > 0) {
        return FWK_E_RANGE;
    }

    *level_uv = pwr_regu_level(ctx->pwr_id);

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI voltd %u: get_level_from_index PWR#%u = %" PRId32,
        fwk_id_get_element_idx(dev_id),
        ctx->pwr_id,
        *level_uv);

    return FWK_SUCCESS;
}

static const struct mod_voltd_drv_api api_stm32_pwr_regu = {
    .get_config = pwr_regu_get_config,
    .set_config = pwr_regu_set_config,
    .get_level = pwr_regu_get_level,
    .set_level = pwr_regu_set_level,
    .get_info = pwr_regu_get_info,
    .get_level_from_index = pwr_regu_level_from_index,
};

/*
 * Framework handler functions
 */

static int stm32_pwr_regu_init(fwk_id_t module_id, unsigned int element_count,
                               const void *data)
{
    module_ctx.dev_count = element_count;

    if (element_count) {
        module_ctx.dev_ctx_table =
            fwk_mm_calloc(element_count, sizeof(*module_ctx.dev_ctx_table));
    }

    return FWK_SUCCESS;
}

static int stm32_pwr_regu_element_init(fwk_id_t element_id,
                                       unsigned int unused,
                                       const void *data)
{
    struct stm32_pwr_regu_dev_ctx *ctx = NULL;
    const struct mod_stm32_pwr_regu_dev_config *dev_config = data;

    if (!fwk_module_is_valid_element_id(element_id)) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(element_id);

    ctx->pwr_id = dev_config->pwr_id;
    ctx->name = dev_config->regu_name;

    return FWK_SUCCESS;
}

static int stm32_pwr_regu_process_bind_request(fwk_id_t requester_id,
                                               fwk_id_t target_id,
                                               fwk_id_t api_type,
                                               const void **api)
{
    *api = &api_stm32_pwr_regu;

    return FWK_SUCCESS;
}

const struct fwk_module module_stm32_pwr_regu = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = stm32_pwr_regu_init,
    .element_init = stm32_pwr_regu_element_init,
    .process_bind_request = stm32_pwr_regu_process_bind_request,
};

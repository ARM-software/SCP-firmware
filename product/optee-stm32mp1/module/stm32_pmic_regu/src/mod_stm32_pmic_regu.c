/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Interface SCP-firmare VOLTD instances with OP-TEE stm32mp1
 *     PMIC regulator driver for voltage regulator controller from
 *     an external PMIC access with a I2C bus using OP-TEE resources.
 *
 *     The OP-TEE drvier is accessed by API function named like
 *     stpmic1_regulator_*(). They execute in a threaded interruptible,
 *     and rescheduable context.
 */
#include <drivers/stm32mp1_pmic.h>
#include <drivers/stpmic1_regulator.h>
#include <stm32_util.h>

#include <mod_scmi_std.h>
#include <mod_stm32_pmic_regu.h>
#include <mod_voltage_domain.h>

#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>

#include <stddef.h>

#define MOD_NAME "[STM32 PMIC] "

/* Device context */
struct stm32_pmic_regu_dev_ctx {
    const char *regu_id; /* Both name and backend regu ID */
    bool read_only;
};

/* Module context */
struct stm32_pmic_regu_ctx {
    struct stm32_pmic_regu_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
};

static struct stm32_pmic_regu_ctx module_ctx;

static int32_t get_regu_voltage(const char *regu_id)
{
    unsigned long level_uv = 0;

    stm32mp_get_pmic();
    level_uv = stpmic1_regulator_voltage_get(regu_id) * 1000;
    stm32mp_put_pmic();

    return (int32_t)level_uv;
}

static int32_t set_regu_voltage(const char *regu_id, int32_t level_uv)
{
    int rc = 0;
    unsigned int level_mv = level_uv / 1000;

    FWK_LOG_DEBUG(
        MOD_NAME "Set STPMIC1 regulator %s level to %dmV",
        regu_id,
        level_uv / 1000);

    fwk_assert(level_mv < UINT16_MAX);

    stm32mp_get_pmic();
    rc = stpmic1_regulator_voltage_set(regu_id, level_mv);
    stm32mp_put_pmic();

    return rc ? SCMI_GENERIC_ERROR : SCMI_SUCCESS;
}

static bool regu_is_enable(const char *regu_id)
{
    bool rc = false;

    stm32mp_get_pmic();
    rc = stpmic1_is_regulator_enabled(regu_id);
    stm32mp_put_pmic();

    return rc;
}

static int32_t set_regu_state(const char *regu_id, bool enable)
{
    int rc = 0;

    stm32mp_get_pmic();

    FWK_LOG_DEBUG(
        MOD_NAME "%sable STPMIC1 %s (was %s)",
        enable ? "En" : "Dis",
        regu_id,
        stpmic1_is_regulator_enabled(regu_id) ? "on" : "off");

    if (enable) {
        rc = stpmic1_regulator_enable(regu_id);
    } else {
        rc = stpmic1_regulator_disable(regu_id);
    }

    stm32mp_put_pmic();

    return rc ? SCMI_GENERIC_ERROR : SCMI_SUCCESS;
}

/*
 * Voltage domain driver API functions
 */
static int pmic_regu_get_config(
    fwk_id_t dev_id,
    uint8_t *mode_type,
    uint8_t *mode_id)
{
    struct stm32_pmic_regu_dev_ctx *ctx;

    if (!fwk_module_is_valid_element_id(dev_id) || mode_id == NULL ||
        mode_type == NULL) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    *mode_type = MOD_VOLTD_MODE_TYPE_ARCH;

    if (regu_is_enable(ctx->regu_id)) {
        *mode_id = MOD_VOLTD_MODE_ID_ON;
    } else {
        *mode_id = MOD_VOLTD_MODE_ID_OFF;
    }

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI voltd %u: get config PMIC %s: %s",
        fwk_id_get_element_idx(dev_id),
        ctx->regu_id,
        *mode_id == MOD_VOLTD_MODE_ID_ON ? "on" : "off");

    return FWK_SUCCESS;
}

static int pmic_regu_set_config(
    fwk_id_t dev_id,
    uint8_t mode_type,
    uint8_t mode_id)
{
    struct stm32_pmic_regu_dev_ctx *ctx = NULL;

    if (!fwk_module_is_valid_element_id(dev_id) ||
        mode_type != MOD_VOLTD_MODE_TYPE_ARCH ||
        (mode_id == MOD_VOLTD_MODE_ID_ON && mode_id == MOD_VOLTD_MODE_ID_OFF)) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (ctx->read_only) {
        return FWK_E_ACCESS;
    }

    if (set_regu_state(ctx->regu_id, mode_id == MOD_VOLTD_MODE_ID_ON)) {
        return FWK_E_DEVICE;
    }

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI voltd %u: set config PMIC %s to %s",
        fwk_id_get_element_idx(dev_id),
        ctx->regu_id,
        mode_id == MOD_VOLTD_MODE_ID_ON ? "on" : "off");

    return FWK_SUCCESS;
}

static int pmic_regu_get_level(fwk_id_t dev_id, int *level_uv)
{
    struct stm32_pmic_regu_dev_ctx *ctx = NULL;

    if (!fwk_module_is_valid_element_id(dev_id) || level_uv == NULL) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    *level_uv = get_regu_voltage(ctx->regu_id);

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI voltd %u: get level PMIC %s = %d",
        fwk_id_get_element_idx(dev_id),
        ctx->regu_id,
        *level_uv);

    return FWK_SUCCESS;
}

static int pmic_regu_set_level(fwk_id_t dev_id, int level_uv)
{
    struct stm32_pmic_regu_dev_ctx *ctx = NULL;

    if (!fwk_module_is_valid_element_id(dev_id)) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    if (ctx->read_only) {
        return FWK_E_ACCESS;
    }

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI voltd %u: set level PMIC %s to %d",
        fwk_id_get_element_idx(dev_id),
        ctx->regu_id,
        level_uv);

    if (set_regu_voltage(ctx->regu_id, level_uv)) {
        return FWK_E_DEVICE;
    }

    return FWK_SUCCESS;
}

static void find_bound_uv(
    const uint16_t *levels,
    size_t count,
    int32_t *min,
    int32_t *max)
{
    size_t n = 0;

    fwk_assert((count == 0 || levels != NULL) && min != NULL && max != NULL);

    *min = INT32_MAX;
    *max = INT32_MIN;

    for (n = 0; n < count; n++) {
        if (*min > levels[n]) {
            *min = levels[n];
        }
        if (*max < levels[n]) {
            *max = levels[n];
        }
    }

    /* Convert from mV to uV */
    *min *= 1000;
    *max *= 1000;
}

static int pmic_regu_get_info(fwk_id_t dev_id, struct mod_voltd_info *info)
{
    struct stm32_pmic_regu_dev_ctx *ctx = NULL;
    const uint16_t *levels = NULL;
    size_t full_count = 0;

    if (!fwk_module_is_valid_element_id(dev_id) || info == NULL) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    stpmic1_regulator_levels_mv(ctx->regu_id, &levels, &full_count);

    memset(info, 0, sizeof(*info));
    info->name = ctx->regu_id;
    info->level_range.level_type = MOD_VOLTD_VOLTAGE_LEVEL_DISCRETE;
    info->level_range.level_count = full_count;
    find_bound_uv(
        levels,
        full_count,
        &info->level_range.min_uv,
        &info->level_range.max_uv);

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI voltd %u: get_info PMIC %s, range [%d %d]",
        fwk_id_get_element_idx(dev_id),
        ctx->regu_id,
        info->level_range.min_uv,
        info->level_range.max_uv);

    return FWK_SUCCESS;
}

static int pmic_regu_level_from_index(
    fwk_id_t dev_id,
    unsigned int index,
    int32_t *level_uv)
{
    struct stm32_pmic_regu_dev_ctx *ctx = NULL;
    const uint16_t *levels = NULL;
    size_t full_count = 0;

    if (!fwk_module_is_valid_element_id(dev_id) || level_uv == NULL) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(dev_id);

    stpmic1_regulator_levels_mv(ctx->regu_id, &levels, &full_count);
    if (index >= full_count) {
        return FWK_E_RANGE;
    }

    *level_uv = (int32_t)levels[index] * 1000;

    FWK_LOG_DEBUG(
        MOD_NAME "SCMI voltd %u: get level PMIC %s = %d",
        fwk_id_get_element_idx(dev_id),
        ctx->regu_id,
        *level_uv);

    return FWK_SUCCESS;
}

static const struct mod_voltd_drv_api api_stm32_pmic_regu = {
    .get_config = pmic_regu_get_config,
    .set_config = pmic_regu_set_config,
    .get_level = pmic_regu_get_level,
    .set_level = pmic_regu_set_level,
    .get_info = pmic_regu_get_info,
    .get_level_from_index = pmic_regu_level_from_index,
};

/*
 * Framework handler functions
 */

static int stm32_pmic_regu_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    module_ctx.dev_count = element_count;

    if (element_count) {
        module_ctx.dev_ctx_table =
            fwk_mm_calloc(element_count, sizeof(*module_ctx.dev_ctx_table));
    }

    return FWK_SUCCESS;
}

static int stm32_pmic_regu_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    struct stm32_pmic_regu_dev_ctx *ctx = NULL;
    const struct mod_stm32_pmic_regu_dev_config *dev_config = data;

    if (!fwk_module_is_valid_element_id(element_id)) {
        return FWK_E_PARAM;
    }

    ctx = module_ctx.dev_ctx_table + fwk_id_get_element_idx(element_id);

    ctx->regu_id = dev_config->regu_name;
    ctx->read_only = dev_config->read_only;

    return FWK_SUCCESS;
}

static int stm32_pmic_regu_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t target_id,
    fwk_id_t api_type,
    const void **api)
{
    *api = &api_stm32_pmic_regu;

    return FWK_SUCCESS;
}

const struct fwk_module module_stm32_pmic_regu = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = stm32_pmic_regu_init,
    .element_init = stm32_pmic_regu_element_init,
    .process_bind_request = stm32_pmic_regu_process_bind_request,
};

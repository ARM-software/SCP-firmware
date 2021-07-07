/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_mock_voltage_domain.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>

enum mod_mock_voltage_domain_request {
    MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_NONE,

    MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_GET_MODE_ID,
    MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_SET_MODE_ID,
    MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_GET_VOLTAGE,
    MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_SET_VOLTAGE,
};

struct mod_mock_voltage_domain_operation {
    enum mod_mock_voltage_domain_request request;

    union {
        enum mod_voltd_mode_id mode_id;
        int32_t voltage;
    };
};

struct mod_mock_voltage_domain_ctx {
    struct mod_mock_voltage_domain_element_ctx {
        enum mod_voltd_mode_id mode_id;
        int32_t voltage;

        struct mod_mock_voltage_domain_operation op;

        struct {
            const struct mod_timer_alarm_api *alarm;
            const struct mod_mock_voltage_domain_driver_response_api *hal;
        } apis;
    } * elements;
};

static struct mod_mock_voltage_domain_ctx mock_voltage_domain_ctx;

static struct mod_mock_voltage_domain_element_ctx *
    mod_mock_voltage_domain_get_ctx(fwk_id_t element_id)
{
    unsigned int element_idx = fwk_id_get_element_idx(element_id);

    return &mock_voltage_domain_ctx.elements[element_idx];
}

static int mod_mock_voltage_domain_get_cfg_ctx(
    fwk_id_t element_id,
    const struct mod_mock_voltage_domain_element_cfg **cfg,
    struct mod_mock_voltage_domain_element_ctx **ctx)
{
    int status = FWK_SUCCESS;
    enum fwk_module_idx module_idx =
        (enum fwk_module_idx)fwk_id_get_module_idx(element_id);

    if (module_idx == FWK_MODULE_IDX_MOCK_VOLTAGE_DOMAIN) {
        if (ctx != NULL) {
            *ctx = mod_mock_voltage_domain_get_ctx(element_id);
        }

        if (cfg != NULL) {
            *cfg = fwk_module_get_data(element_id);
        }
    } else {
        status = FWK_E_PARAM;
    }

    return status;
}

static void mod_mock_voltage_domain_alarm_callback(uintptr_t element_idx)
{
    int status;

    fwk_id_t element_id =
        fwk_id_build_element_id(fwk_module_id_mock_voltage_domain, element_idx);

    const struct mod_mock_voltage_domain_element_cfg *cfg;
    struct mod_mock_voltage_domain_element_ctx *ctx;
    struct mod_mock_voltage_domain_driver_response response;

    status = mod_mock_voltage_domain_get_cfg_ctx(element_id, &cfg, &ctx);
    if (fwk_expect(status == FWK_SUCCESS)) {
        response = (struct mod_mock_voltage_domain_driver_response){
            .status = FWK_SUCCESS,
        };

        switch (ctx->op.request) {
        case MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_GET_MODE_ID:
            response.mode_id = ctx->mode_id;
            break;
        case MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_SET_MODE_ID:
            ctx->mode_id = ctx->op.mode_id;
            break;
        case MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_GET_VOLTAGE:
            response.voltage = ctx->voltage;
            break;
        case MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_SET_VOLTAGE:
            ctx->voltage = ctx->op.voltage;
            break;
        default:
            fwk_unreachable();
            break;
        }

        ctx->op.request = MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_NONE;

        ctx->apis.hal->respond(cfg->async_response_id, response);
    }
}

static int mod_mock_voltage_domain_trigger(
    fwk_id_t element_id,
    struct mod_mock_voltage_domain_operation op)
{
    static const unsigned int ASYNC_DELAY = 1U /* ms */;

    int status;

    const struct mod_mock_voltage_domain_element_cfg *cfg;
    struct mod_mock_voltage_domain_element_ctx *ctx;

    status = mod_mock_voltage_domain_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status == FWK_SUCCESS) {
        status = ctx->apis.alarm->start(
            cfg->async_alarm_id,
            ASYNC_DELAY,
            MOD_TIMER_ALARM_TYPE_ONCE,
            mod_mock_voltage_domain_alarm_callback,
            fwk_id_get_element_idx(element_id));

        if (status == FWK_SUCCESS) {
            status = FWK_PENDING;
            ctx->op = op;
        } else {
            status = FWK_E_HANDLER;
        }
    }

    return status;
}

static int mod_mock_voltage_domain_get_level(
    fwk_id_t element_id,
    int32_t *voltage)
{
    int status;

    const struct mod_mock_voltage_domain_element_cfg *cfg;
    struct mod_mock_voltage_domain_element_ctx *ctx;

    status = mod_mock_voltage_domain_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status == FWK_SUCCESS) {
        if (ctx->apis.alarm == NULL) {
            *voltage = ctx->voltage;
        } else if (ctx->op.request == MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_NONE) {
            struct mod_mock_voltage_domain_operation op = {
                .request = MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_GET_VOLTAGE,
            };
            status = mod_mock_voltage_domain_trigger(element_id, op);
        } else {
            status = FWK_E_BUSY;
        }
    }

    return status;
}

static int mod_mock_voltage_domain_set_level(
    fwk_id_t element_id,
    int32_t voltage)
{
    int status;
    size_t loop_index;

    const struct mod_mock_voltage_domain_element_cfg *cfg;
    struct mod_mock_voltage_domain_element_ctx *ctx;

    status = mod_mock_voltage_domain_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status == FWK_SUCCESS) {
        switch (cfg->level_type) {
        case MOD_VOLTD_VOLTAGE_LEVEL_DISCRETE: {
            for (loop_index = (size_t)0; loop_index < cfg->level_count;
                 loop_index++) {
                if (voltage == cfg->voltage_levels[loop_index]) {
                    break;
                }
            }
            if (loop_index == cfg->level_count) {
                status = FWK_E_RANGE;
            }
            break;
        }

        case MOD_VOLTD_VOLTAGE_LEVEL_CONTINUOUS: {
            if ((voltage >= cfg->voltage_levels[0]) &&
                (voltage <= cfg->voltage_levels[1])) {
                if (((voltage - cfg->voltage_levels[0]) %
                     cfg->voltage_levels[2]) != 0) {
                    status = FWK_E_RANGE;
                }
            } else {
                status = FWK_E_RANGE;
            }
            break;
        }

        default:
            status = FWK_E_STATE;
            break;
        }

        if (status == FWK_SUCCESS) {
            if (ctx->apis.alarm == NULL) {
                ctx->voltage = voltage;
            } else if (
                ctx->op.request == MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_NONE) {
                struct mod_mock_voltage_domain_operation op = {
                    .request = MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_SET_VOLTAGE,
                    .voltage = voltage,
                };
                status = mod_mock_voltage_domain_trigger(element_id, op);
            } else {
                status = FWK_E_BUSY;
            }
        }
    }

    return status;
}

static int mod_mock_voltage_domain_get_config(
    fwk_id_t element_id,
    uint8_t *mode_type,
    uint8_t *mode_id)
{
    int status;

    const struct mod_mock_voltage_domain_element_cfg *cfg;
    struct mod_mock_voltage_domain_element_ctx *ctx;

    status = mod_mock_voltage_domain_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status == FWK_SUCCESS) {
        *mode_type = (uint8_t)MOD_VOLTD_MODE_TYPE_ARCH;
        if (ctx->apis.alarm == NULL) {
            *mode_id = (uint8_t)ctx->mode_id;
        } else if (ctx->op.request == MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_NONE) {
            struct mod_mock_voltage_domain_operation op = {
                .request = MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_GET_MODE_ID,
            };
            status = mod_mock_voltage_domain_trigger(element_id, op);
        } else {
            status = FWK_E_BUSY;
        }
    }

    return status;
}

static int mod_mock_voltage_domain_set_config(
    fwk_id_t element_id,
    uint8_t mode_type,
    uint8_t mode_id)
{
    int status = FWK_SUCCESS;

    const struct mod_mock_voltage_domain_element_cfg *cfg;
    struct mod_mock_voltage_domain_element_ctx *ctx;

    status = mod_mock_voltage_domain_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status == FWK_SUCCESS) {
        if (mode_type != (uint8_t)MOD_VOLTD_MODE_TYPE_ARCH) {
            status = FWK_E_SUPPORT;
        } else if (
            (mode_id != (uint8_t)MOD_VOLTD_MODE_ID_OFF) &&
            (mode_id != (uint8_t)MOD_VOLTD_MODE_ID_ON)) {
            status = FWK_E_PARAM;
        } else if (ctx->apis.alarm == NULL) {
            ctx->mode_id = (enum mod_voltd_mode_id)mode_id;
        } else if (ctx->op.request == MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_NONE) {
            struct mod_mock_voltage_domain_operation op = {
                .request = MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_SET_MODE_ID,
                .mode_id = (enum mod_voltd_mode_id)mode_id,
            };
            status = mod_mock_voltage_domain_trigger(element_id, op);
        } else {
            status = FWK_E_BUSY;
        }
    }

    return status;
}

static int mod_mock_voltage_domain_get_info(
    fwk_id_t element_id,
    struct mod_voltd_info *info)
{
    int status;

    const struct mod_mock_voltage_domain_element_cfg *cfg;
    struct mod_mock_voltage_domain_element_ctx *ctx;

    status = mod_mock_voltage_domain_get_cfg_ctx(element_id, &cfg, &ctx);

    if (status == FWK_SUCCESS) {
        info->level_range.level_count = cfg->level_count;
        info->level_range.level_type = cfg->level_type;
        if (cfg->level_type == MOD_VOLTD_VOLTAGE_LEVEL_CONTINUOUS) {
            info->level_range.min_uv = cfg->voltage_levels[0];
            info->level_range.max_uv = cfg->voltage_levels[1];
            info->level_range.step_uv = cfg->voltage_levels[2];
        }
        info->name = fwk_module_get_element_name(element_id);
    }

    return status;
}

static int mod_mock_voltage_domain_get_level_from_index(
    fwk_id_t element_id,
    unsigned int index,
    int32_t *level_uv)
{
    int status;

    const struct mod_mock_voltage_domain_element_cfg *cfg;
    struct mod_mock_voltage_domain_element_ctx *ctx;

    status = mod_mock_voltage_domain_get_cfg_ctx(element_id, &cfg, &ctx);

    if (status == FWK_SUCCESS) {
        if (index > cfg->level_count) {
            status = FWK_E_RANGE;
        } else {
            *level_uv = cfg->voltage_levels[index];
        }
    }

    return status;
}

static const struct mod_voltd_drv_api mock_voltage_domain_driver_api = {
    .get_config = mod_mock_voltage_domain_get_config,
    .set_config = mod_mock_voltage_domain_set_config,
    .get_level = mod_mock_voltage_domain_get_level,
    .set_level = mod_mock_voltage_domain_set_level,
    .get_info = mod_mock_voltage_domain_get_info,
    .get_level_from_index = mod_mock_voltage_domain_get_level_from_index,
};

static int mod_mock_voltage_domain_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    fwk_check(data == NULL);

    mock_voltage_domain_ctx.elements = fwk_mm_calloc(
        element_count, sizeof(mock_voltage_domain_ctx.elements[0]));

    return FWK_SUCCESS;
}

static int mod_mock_voltage_domain_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_mock_voltage_domain_element_ctx *ctx;
    const struct mod_mock_voltage_domain_element_cfg *cfg = data;

    fwk_check(sub_element_count == 0U);

    ctx = mod_mock_voltage_domain_get_ctx(element_id);

    *ctx = (struct mod_mock_voltage_domain_element_ctx) {
        .mode_id = cfg->default_mode_id,
        .voltage = cfg->default_voltage,

        .op = {
            .request = MOD_MOCK_VOLTAGE_DOMAIN_REQUEST_NONE,
        },
    };

    return FWK_SUCCESS;
}

static int mod_mock_voltage_domain_bind(fwk_id_t id, unsigned int call_round)
{
    int status = FWK_SUCCESS;

    if ((call_round == 0U) && fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        const struct mod_mock_voltage_domain_element_cfg *cfg =
            fwk_module_get_data(id);

        if (!fwk_id_is_equal(cfg->async_alarm_id, FWK_ID_NONE)) {
            struct mod_mock_voltage_domain_element_ctx *ctx =
                mod_mock_voltage_domain_get_ctx(id);

            status = fwk_module_bind(
                cfg->async_alarm_id, cfg->async_alarm_api_id, &ctx->apis.alarm);
            if (status == FWK_SUCCESS) {
                status = fwk_module_bind(
                    cfg->async_response_id,
                    cfg->async_response_api_id,
                    &ctx->apis.hal);
            }
        }
    }

    return status;
}

static int mod_mock_voltage_domain_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    const struct mod_mock_voltage_domain_element_cfg *cfg;
    int status = FWK_SUCCESS;
    enum mod_mock_voltage_domain_api_idx domain_api_idx =
        (enum mod_mock_voltage_domain_api_idx)fwk_id_get_api_idx(api_id);

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT)) {
        status = FWK_E_ACCESS;
    } else if (domain_api_idx == MOD_MOCK_VOLTAGE_DOMAIN_API_IDX_VOLTD) {
        cfg = fwk_module_get_data(target_id);
        if (!fwk_id_is_equal(cfg->async_alarm_id, FWK_ID_NONE)) {
            /*
             * If we are configured to respond asynchronously, the only entity
             * we accept bind requests from is the entity that we have
             * configured to be our designated driver respondee.
             */
            if (!fwk_id_is_equal(source_id, cfg->async_response_id)) {
                status = FWK_E_ACCESS;
            }
        }

        if (status == FWK_SUCCESS) {
            *api = &mock_voltage_domain_driver_api;
        }

    } else {
        status = FWK_E_ACCESS;
    }

    return status;
}

const struct fwk_module module_mock_voltage_domain = {
    .type = FWK_MODULE_TYPE_DRIVER,

    .init = mod_mock_voltage_domain_init,
    .element_init = mod_mock_voltage_domain_element_init,
    .bind = mod_mock_voltage_domain_bind,

    .api_count = (unsigned int)MOD_MOCK_VOLTAGE_DOMAIN_API_IDX_COUNT,
    .process_bind_request = mod_mock_voltage_domain_process_bind_request,
};

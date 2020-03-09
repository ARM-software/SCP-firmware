/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_mock_psu.h>
#include <mod_psu.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum mod_mock_psu_request {
    MOD_MOCK_PSU_REQUEST_NONE,

    MOD_MOCK_PSU_REQUEST_GET_ENABLED,
    MOD_MOCK_PSU_REQUEST_SET_ENABLED,
    MOD_MOCK_PSU_REQUEST_GET_VOLTAGE,
    MOD_MOCK_PSU_REQUEST_SET_VOLTAGE,
};

struct mod_mock_psu_operation {
    enum mod_mock_psu_request request;

    union {
        bool enabled;
        uint64_t voltage;
    };
};

static struct mod_mock_psu_ctx {
    struct mod_mock_psu_element_ctx {
        struct {
            bool enabled;
            uint64_t voltage;
        } state;

        struct mod_mock_psu_operation op;

        struct {
            const struct mod_timer_alarm_api *alarm;
            const struct mod_psu_driver_response_api *hal;
        } apis;
    } *elements;
} mod_mock_psu_ctx;

static struct mod_mock_psu_element_ctx *mod_mock_psu_get_ctx(
    fwk_id_t element_id)
{
    unsigned int element_idx = fwk_id_get_element_idx(element_id);

    return &mod_mock_psu_ctx.elements[element_idx];
}


static int mod_mock_psu_get_cfg_ctx(
    fwk_id_t element_id,
    const struct mod_mock_psu_element_cfg **cfg,
    struct mod_mock_psu_element_ctx **ctx)
{
    if (fwk_id_get_module_idx(element_id) != FWK_MODULE_IDX_MOCK_PSU)
        return FWK_E_PARAM;

    if (ctx != NULL)
        *ctx = mod_mock_psu_get_ctx(element_id);

    if (cfg != NULL)
        *cfg = fwk_module_get_data(element_id);

    return FWK_SUCCESS;
}

static void mod_mock_psu_alarm_callback(uintptr_t element_idx)
{
    int status;

    fwk_id_t element_id = fwk_id_build_element_id(
        fwk_module_id_mock_psu, element_idx);

    const struct mod_mock_psu_element_cfg *cfg;
    struct mod_mock_psu_element_ctx *ctx;
    struct mod_psu_driver_response response;

    status = mod_mock_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (!fwk_expect(status == FWK_SUCCESS))
        return;

    response = (struct mod_psu_driver_response){
        .status = FWK_SUCCESS,
    };

    switch (ctx->op.request) {
    case MOD_MOCK_PSU_REQUEST_GET_ENABLED:
        response.enabled = ctx->state.enabled;

        break;

    case MOD_MOCK_PSU_REQUEST_SET_ENABLED:
        ctx->state.enabled = ctx->op.enabled;

        break;

    case MOD_MOCK_PSU_REQUEST_GET_VOLTAGE:
        response.voltage = ctx->state.voltage;

        break;

    case MOD_MOCK_PSU_REQUEST_SET_VOLTAGE:
        ctx->state.voltage = ctx->op.voltage;

        break;

    default:
        fwk_unreachable();
    }

    ctx->op.request = MOD_MOCK_PSU_REQUEST_NONE;

    ctx->apis.hal->respond(cfg->async_response_id, response);
}

static int mod_mock_psu_trigger(
    fwk_id_t element_id,
    struct mod_mock_psu_operation op)
{
    static const unsigned int ASYNC_DELAY = 1 /* ms */;

    int status;

    const struct mod_mock_psu_element_cfg *cfg;
    struct mod_mock_psu_element_ctx *ctx;

    status = mod_mock_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status != FWK_SUCCESS)
        goto exit;

    status = ctx->apis.alarm->start(
        cfg->async_alarm_id,
        ASYNC_DELAY,
        MOD_TIMER_ALARM_TYPE_ONCE,
        mod_mock_psu_alarm_callback,
        fwk_id_get_element_idx(element_id));

    if (status == FWK_SUCCESS) {
        status = FWK_PENDING;

        ctx->op = op;
    } else
        status = FWK_E_HANDLER;

exit:
    return status;
}

static int mod_mock_psu_get_enabled(
    fwk_id_t element_id,
    bool *enabled)
{
    int status;

    const struct mod_mock_psu_element_cfg *cfg;
    struct mod_mock_psu_element_ctx *ctx;

    status = mod_mock_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status != FWK_SUCCESS)
        goto exit;

    if (ctx->apis.alarm == NULL) {
        *enabled = ctx->state.enabled;

        goto exit;
    }

    if (ctx->op.request == MOD_MOCK_PSU_REQUEST_NONE) {
        struct mod_mock_psu_operation op = {
            .request = MOD_MOCK_PSU_REQUEST_GET_ENABLED,
        };

        status = mod_mock_psu_trigger(element_id, op);
    } else
        status = FWK_E_BUSY;

exit:
    return status;
}

static int mod_mock_psu_set_enabled(
    fwk_id_t element_id,
    bool enabled)
{
    int status;

    const struct mod_mock_psu_element_cfg *cfg;
    struct mod_mock_psu_element_ctx *ctx;

    status = mod_mock_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status != FWK_SUCCESS)
        goto exit;

    if (ctx->apis.alarm == NULL) {
        ctx->state.enabled = enabled;

        goto exit;
    }

    if (ctx->op.request == MOD_MOCK_PSU_REQUEST_NONE) {
        struct mod_mock_psu_operation op = {
            .request = MOD_MOCK_PSU_REQUEST_SET_ENABLED,
            .enabled = enabled,
        };

        status = mod_mock_psu_trigger(element_id, op);
    } else
        status = FWK_E_BUSY;

exit:
    return status;
}

static int mod_mock_psu_get_voltage(
    fwk_id_t element_id,
    uint64_t *voltage)
{
    int status;

    const struct mod_mock_psu_element_cfg *cfg;
    struct mod_mock_psu_element_ctx *ctx;

    status = mod_mock_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status != FWK_SUCCESS)
        goto exit;

    if (ctx->apis.alarm == NULL) {
        *voltage = ctx->state.voltage;

        goto exit;
    }

    if (ctx->op.request == MOD_MOCK_PSU_REQUEST_NONE) {
        struct mod_mock_psu_operation op = {
            .request = MOD_MOCK_PSU_REQUEST_GET_VOLTAGE,
        };

        status = mod_mock_psu_trigger(element_id, op);
    } else
        status = FWK_E_BUSY;

exit:
    return status;
}

static int mod_mock_psu_set_voltage(
    fwk_id_t element_id,
    uint64_t voltage)
{
    int status;

    const struct mod_mock_psu_element_cfg *cfg;
    struct mod_mock_psu_element_ctx *ctx;

    status = mod_mock_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status != FWK_SUCCESS)
        goto exit;

    if (ctx->apis.alarm == NULL) {
        ctx->state.voltage = voltage;

        goto exit;
    }

    if (ctx->op.request == MOD_MOCK_PSU_REQUEST_NONE) {
        struct mod_mock_psu_operation op = {
            .request = MOD_MOCK_PSU_REQUEST_SET_VOLTAGE,
            .voltage = voltage,
        };

        status = mod_mock_psu_trigger(element_id, op);
    } else
        status = FWK_E_BUSY;

exit:
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

    return FWK_SUCCESS;
}

static int mod_mock_psu_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_mock_psu_element_ctx *ctx;
    const struct mod_mock_psu_element_cfg *cfg = data;

    fwk_expect(sub_element_count == 0);

    ctx = mod_mock_psu_get_ctx(element_id);

    *ctx = (struct mod_mock_psu_element_ctx) {
        .state = {
            .enabled = cfg->default_enabled,
            .voltage = cfg->default_voltage,
        },

        .op = {
            .request = MOD_MOCK_PSU_REQUEST_NONE,
        },
    };

    return FWK_SUCCESS;
}

static int mod_mock_psu_bind(fwk_id_t id, unsigned int round)
{
    int status = FWK_SUCCESS;

    if ((round == 0) && fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        const struct mod_mock_psu_element_cfg *cfg = fwk_module_get_data(id);

        if (!fwk_id_is_equal(cfg->async_alarm_id, FWK_ID_NONE)) {
            struct mod_mock_psu_element_ctx *ctx =
                mod_mock_psu_get_ctx(id);

            status = fwk_module_bind(
                cfg->async_alarm_id,
                cfg->async_alarm_api_id,
                &ctx->apis.alarm);
            if (status != FWK_SUCCESS)
                goto exit;

            status = fwk_module_bind(
                cfg->async_response_id,
                cfg->async_response_api_id,
                &ctx->apis.hal);
        }
    }

exit:
    return status;
}

static int mod_mock_psu_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    const struct mod_mock_psu_element_cfg *cfg;

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_ACCESS;

    if (!fwk_id_is_equal(api_id, mod_mock_psu_api_id_driver))
        return FWK_E_ACCESS;

    cfg = fwk_module_get_data(target_id);

    if (!fwk_id_is_equal(cfg->async_alarm_id, FWK_ID_NONE)) {
        /*
         * If we are configured to respond asynchronously, the only entity we
         * accept bind requests from is the entity that we have configured to be
         * our designated driver respondee.
         */
        if (!fwk_id_is_equal(source_id, cfg->async_response_id))
            return FWK_E_ACCESS;
    }

    *api = &mod_mock_psu_driver_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_mock_psu = {
    .name = "mock_psu",
    .type = FWK_MODULE_TYPE_DRIVER,

    .init = mod_mock_psu_init,
    .element_init = mod_mock_psu_element_init,
    .bind = mod_mock_psu_bind,

    .api_count = MOD_MOCK_PSU_API_IDX_COUNT,
    .process_bind_request = mod_mock_psu_process_bind_request,
};

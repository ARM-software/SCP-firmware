/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_psu.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <string.h>

enum mod_psu_state {
    MOD_PSU_STATE_IDLE,
    MOD_PSU_STATE_BUSY,
};

struct mod_psu_operation {
    enum mod_psu_state state;

    unsigned int cookie;
};

enum mod_psu_impl_event_idx {
    MOD_PSU_IMPL_EVENT_IDX_RESPONSE = MOD_PSU_EVENT_IDX_COUNT,

    MOD_PSU_IMPL_EVENT_IDX_COUNT,
};

static const fwk_id_t mod_psu_impl_event_id_response =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_PSU, MOD_PSU_IMPL_EVENT_IDX_RESPONSE);

static struct mod_psu_ctx {
    struct mod_psu_element_ctx {
        const struct mod_psu_driver_api *driver;

        struct mod_psu_operation op;
    } *elements;
} mod_psu_ctx;

static struct mod_psu_element_ctx *mod_psu_get_element_ctx(fwk_id_t element_id)
{
    unsigned int element_idx = fwk_id_get_element_idx(element_id);

    return &mod_psu_ctx.elements[element_idx];
}

static int mod_psu_get_cfg_ctx(
    fwk_id_t element_id,
    const struct mod_psu_element_cfg **cfg,
    struct mod_psu_element_ctx **ctx)
{
    if (fwk_id_get_module_idx(element_id) != FWK_MODULE_IDX_PSU)
        return FWK_E_PARAM;

    if (ctx != NULL)
        *ctx = mod_psu_get_element_ctx(element_id);

    if (cfg != NULL)
        *cfg = fwk_module_get_data(element_id);

    return FWK_SUCCESS;
}

static int mod_psu_get_enabled(fwk_id_t element_id, bool *enabled)
{
    int status;

    const struct mod_psu_element_cfg *cfg;
    struct mod_psu_element_ctx *ctx;

    status = mod_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status != FWK_SUCCESS)
        goto exit;

    if (ctx->op.state != MOD_PSU_STATE_IDLE) {
        status = FWK_E_BUSY;

        goto exit;
    }

    status = ctx->driver->get_enabled(cfg->driver_id, enabled);
    if (status == FWK_PENDING) {
        struct fwk_event request = {
            .id = mod_psu_event_id_get_enabled,
            .target_id = element_id,

            .response_requested = true,
        };

        status = fwk_thread_put_event(&request);
        if (status == FWK_SUCCESS) {
            ctx->op.state = MOD_PSU_STATE_BUSY;

            status = FWK_PENDING;
        } else
            status = FWK_E_STATE;
    } else if (status != FWK_SUCCESS)
        status = FWK_E_HANDLER;

exit:
    return status;
}

static int mod_psu_set_enabled(fwk_id_t element_id, bool enabled)
{
    int status;

    const struct mod_psu_element_cfg *cfg;
    struct mod_psu_element_ctx *ctx;

    status = mod_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status != FWK_SUCCESS)
        goto exit;

    if (ctx->op.state != MOD_PSU_STATE_IDLE) {
        status = FWK_E_BUSY;

        goto exit;
    }

    status = ctx->driver->set_enabled(cfg->driver_id, enabled);
    if (status == FWK_PENDING) {
        struct fwk_event request = {
            .id = mod_psu_event_id_set_enabled,
            .target_id = element_id,

            .response_requested = true,
        };

        status = fwk_thread_put_event(&request);
        if (status == FWK_SUCCESS) {
            ctx->op.state = MOD_PSU_STATE_BUSY;

            status = FWK_PENDING;
        } else
            status = FWK_E_STATE;
    } else if (status != FWK_SUCCESS)
        status = FWK_E_HANDLER;

exit:
    return status;
}

static int mod_psu_get_voltage(fwk_id_t element_id, uint64_t *voltage)
{
    int status;

    const struct mod_psu_element_cfg *cfg;
    struct mod_psu_element_ctx *ctx;

    status = mod_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status != FWK_SUCCESS)
        goto exit;

    if (ctx->op.state != MOD_PSU_STATE_IDLE) {
        status = FWK_E_BUSY;

        goto exit;
    }

    status = ctx->driver->get_voltage(cfg->driver_id, voltage);
    if (status == FWK_PENDING) {
        struct fwk_event request = {
            .id = mod_psu_event_id_get_voltage,
            .target_id = element_id,

            .response_requested = true,
        };

        status = fwk_thread_put_event(&request);
        if (status == FWK_SUCCESS) {
            ctx->op.state = MOD_PSU_STATE_BUSY;

            status = FWK_PENDING;
        } else
            status = FWK_E_STATE;
    } else if (status != FWK_SUCCESS)
        status = FWK_E_HANDLER;

exit:
    return status;
}

static int mod_psu_set_voltage(fwk_id_t element_id, uint64_t voltage)
{
    int status;

    const struct mod_psu_element_cfg *cfg;
    struct mod_psu_element_ctx *ctx;

    status = mod_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (status != FWK_SUCCESS)
        goto exit;

    if (ctx->op.state != MOD_PSU_STATE_IDLE) {
        status = FWK_E_BUSY;

        goto exit;
    }

    status = ctx->driver->set_voltage(cfg->driver_id, voltage);
    if (status == FWK_PENDING) {
        struct fwk_event request = {
            .id = mod_psu_event_id_set_voltage,
            .target_id = element_id,

            .response_requested = true,
        };

        status = fwk_thread_put_event(&request);
        if (status == FWK_SUCCESS) {
            ctx->op.state = MOD_PSU_STATE_BUSY;

            status = FWK_PENDING;
        } else
            status = FWK_E_STATE;
    } else if (status != FWK_SUCCESS)
        status = FWK_E_HANDLER;

exit:
    return status;
}

static const struct mod_psu_device_api mod_psu_device_api = {
    .get_enabled = mod_psu_get_enabled,
    .set_enabled = mod_psu_set_enabled,
    .get_voltage = mod_psu_get_voltage,
    .set_voltage = mod_psu_set_voltage,
};

static void mod_psu_respond(
    fwk_id_t element_id,
    struct mod_psu_driver_response response)
{
    int status;

    const struct mod_psu_element_cfg *cfg;
    struct mod_psu_element_ctx *ctx;

    struct fwk_event event;

    status = mod_psu_get_cfg_ctx(element_id, &cfg, &ctx);
    if (!fwk_expect(status == FWK_SUCCESS))
        return;

    event = (struct fwk_event){
        .id = mod_psu_impl_event_id_response,

        .source_id = cfg->driver_id,
        .target_id = element_id,
    };

    memcpy(event.params, &response, sizeof(response));

    status = fwk_thread_put_event(&event);
    if (!fwk_expect(status == FWK_SUCCESS))
        ctx->op.state = MOD_PSU_STATE_IDLE;
}

static const struct mod_psu_driver_response_api mod_psu_driver_response_api = {
    .respond = mod_psu_respond,
};

static int mod_psu_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    fwk_expect(data == NULL);

    mod_psu_ctx.elements =
        fwk_mm_calloc(element_count, sizeof(mod_psu_ctx.elements[0]));

    return FWK_SUCCESS;
}

static int mod_psu_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_psu_element_ctx *ctx;

    fwk_expect(sub_element_count == 0);

    ctx = mod_psu_get_element_ctx(element_id);

    *ctx = (struct mod_psu_element_ctx){
        .op = (struct mod_psu_operation){
            .state = MOD_PSU_STATE_IDLE,
        },
    };

    return FWK_SUCCESS;
}

static int mod_psu_bind_element(fwk_id_t element_id, unsigned int round)
{
    int status = FWK_SUCCESS;

    const struct mod_psu_element_ctx *ctx;
    const struct mod_psu_element_cfg *cfg;

    if (round > 0)
        goto exit;

    ctx = mod_psu_get_element_ctx(element_id);
    cfg = fwk_module_get_data(element_id);

    status = fwk_module_bind(cfg->driver_id, cfg->driver_api_id, &ctx->driver);
    if (status != FWK_SUCCESS)
        goto exit;

    if ((ctx->driver->set_enabled == NULL) ||
        (ctx->driver->get_enabled == NULL) ||
        (ctx->driver->set_voltage == NULL) ||
        (ctx->driver->get_voltage == NULL)) {
        status = FWK_E_PANIC;
    }

exit:
    return status;
}

static int mod_psu_bind(fwk_id_t id, unsigned int round)
{
    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return mod_psu_bind_element(id, round);

    return FWK_SUCCESS;
}

static int mod_psu_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_PARAM;

    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_PSU_API_IDX_DEVICE:
        *api = &mod_psu_device_api;

        break;

    case MOD_PSU_API_IDX_DRIVER_RESPONSE:
        *api = &mod_psu_driver_response_api;

        break;
    }

    return FWK_SUCCESS;
}

static int mod_psu_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status = FWK_SUCCESS;

    struct fwk_event hal_event;

    const struct mod_psu_driver_response *params =
        (struct mod_psu_driver_response *)event->params;
    struct mod_psu_response *resp_params =
        (struct mod_psu_response *)resp_event->params;
    struct mod_psu_response *hal_params =
        (struct mod_psu_response *)&hal_event.params;

    const struct mod_psu_element_cfg *cfg;
    struct mod_psu_element_ctx *ctx;

    resp_params->status =
        mod_psu_get_cfg_ctx(event->target_id, &cfg, &ctx);
    if (resp_params->status != FWK_SUCCESS)
        goto exit;

    switch (fwk_id_get_event_idx(event->id)) {
    case MOD_PSU_EVENT_IDX_GET_ENABLED:
    case MOD_PSU_EVENT_IDX_GET_VOLTAGE:
    case MOD_PSU_EVENT_IDX_SET_ENABLED:
    case MOD_PSU_EVENT_IDX_SET_VOLTAGE:
        ctx->op.cookie = event->cookie;

        resp_event->is_delayed_response = true;

        break;

    case MOD_PSU_IMPL_EVENT_IDX_RESPONSE:
        ctx->op.state = MOD_PSU_STATE_IDLE;

        status = fwk_thread_get_delayed_response(
            event->target_id,
            ctx->op.cookie,
            &hal_event);
        if (status != FWK_SUCCESS)
            return status;

        *hal_params = (struct mod_psu_response){
            .status = params->status,
        };

        switch (fwk_id_get_event_idx(hal_event.id)) {
        case MOD_PSU_EVENT_IDX_GET_ENABLED:
            hal_params->enabled = params->enabled;

            break;

        case MOD_PSU_EVENT_IDX_GET_VOLTAGE:
            hal_params->voltage = params->voltage;

            break;

        default:
            break;
        }

        status = fwk_thread_put_event(&hal_event);

        break;

    default:
        fwk_unreachable();
    }

exit:
    return status;
}

const struct fwk_module module_psu = {
    .name = "psu",
    .type = FWK_MODULE_TYPE_HAL,

    .init = mod_psu_init,
    .element_init = mod_psu_element_init,
    .bind = mod_psu_bind,

    .api_count = MOD_PSU_API_IDX_COUNT,
    .process_bind_request = mod_psu_process_bind_request,

    .event_count = MOD_PSU_IMPL_EVENT_IDX_COUNT,
    .process_event = mod_psu_process_event,
};

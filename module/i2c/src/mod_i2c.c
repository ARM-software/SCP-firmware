/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <mod_i2c.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <stdbool.h>
#include <string.h>

enum mod_i2c_dev_state {
    MOD_I2C_DEV_IDLE,
    MOD_I2C_DEV_TX,
    MOD_I2C_DEV_RX,
    MOD_I2C_DEV_TX_RX,
    MOD_I2C_DEV_RELOAD,
    MOD_I2C_DEV_PANIC,
};

struct mod_i2c_dev_ctx {
    const struct mod_i2c_dev_config *config;
    const struct mod_i2c_driver_api *driver_api;
    struct mod_i2c_request request;
    enum mod_i2c_dev_state state;
};

static struct mod_i2c_dev_ctx *ctx_table;

enum mod_i2c_internal_event_idx {
    MOD_I2C_EVENT_IDX_REQUEST_COMPLETED = MOD_I2C_EVENT_IDX_COUNT,
    MOD_I2C_EVENT_IDX_RELOAD,
    MOD_I2C_EVENT_IDX_TOTAL_COUNT,
};

/*! Request completed event identifier */
static const fwk_id_t mod_i2c_event_id_request_completed = FWK_ID_EVENT_INIT(
    FWK_MODULE_IDX_I2C, MOD_I2C_EVENT_IDX_REQUEST_COMPLETED);

/*! Reload event identifier */
static const fwk_id_t mod_i2c_event_id_reload = FWK_ID_EVENT_INIT(
    FWK_MODULE_IDX_I2C, MOD_I2C_EVENT_IDX_RELOAD);

/*
 * Static helpers
 */
static void get_ctx(fwk_id_t id, struct mod_i2c_dev_ctx **ctx)
{
    fwk_assert(fwk_module_is_valid_element_id(id));

    *ctx = ctx_table + fwk_id_get_element_idx(id);
}

static int create_i2c_request(fwk_id_t dev_id,
                              struct mod_i2c_request *request)
{
    int status;
    struct fwk_event event;
    struct mod_i2c_request *event_param =
        (struct mod_i2c_request *)event.params;

    /* The slave address should be on 7 bits */
    if (!fwk_expect(request->slave_address < 0x80))
        return FWK_E_PARAM;

    event = (struct fwk_event) {
        .target_id = dev_id,
        .response_requested = true,
    };

    *event_param = *request;

    if ((request->transmit_byte_count > 0) &&
        (request->receive_byte_count > 0)) {
        event.id = mod_i2c_event_id_request_tx_rx;
    } else if (request->transmit_byte_count > 0) {
        event.id = mod_i2c_event_id_request_tx;
    } else if (request->receive_byte_count > 0) {
        event.id = mod_i2c_event_id_request_rx;
    }

    status = fwk_thread_put_event(&event);
    if (status == FWK_SUCCESS) {
        /*
         * The request has been successfully queued for later processing by the
         * I2C device but processing of this request has not yet begun. The
         * caller is notified that the I2C request is in progress.
         */
        return FWK_PENDING;
    }

    return status;
}

/*
 * I2C API
 */
static int transmit_as_master(fwk_id_t dev_id,
                              uint8_t slave_address,
                              uint8_t *data,
                              uint8_t byte_count)
{
    if (!fwk_expect(byte_count != 0))
        return FWK_E_PARAM;

    if (!fwk_expect(data != NULL))
        return FWK_E_PARAM;

    struct mod_i2c_request request = {
        .slave_address = slave_address,
        .transmit_data = data,
        .transmit_byte_count = byte_count,
    };

    return create_i2c_request(dev_id, &request);
}

static int receive_as_master(fwk_id_t dev_id,
                             uint8_t slave_address,
                             uint8_t *data,
                             uint8_t byte_count)
{
    if (!fwk_expect(byte_count != 0))
        return FWK_E_PARAM;

    if (!fwk_expect(data != NULL))
        return FWK_E_PARAM;

    struct mod_i2c_request request = {
        .slave_address = slave_address,
        .receive_data = data,
        .receive_byte_count = byte_count,
    };

    return create_i2c_request(dev_id, &request);
}

static int transmit_then_receive_as_master(fwk_id_t dev_id,
                                           uint8_t slave_address,
                                           uint8_t *transmit_data,
                                           uint8_t *receive_data,
                                           uint8_t transmit_byte_count,
                                           uint8_t receive_byte_count)
{
    if (!fwk_expect((transmit_byte_count != 0) && (receive_byte_count != 0)))
        return FWK_E_PARAM;

    if (!fwk_expect((transmit_data != NULL) && (receive_data != NULL)))
        return FWK_E_PARAM;

    struct mod_i2c_request request = {
        .slave_address = slave_address,
        .transmit_data = transmit_data,
        .receive_data = receive_data,
        .transmit_byte_count = transmit_byte_count,
        .receive_byte_count = receive_byte_count,
    };

    return create_i2c_request(dev_id, &request);
}

static struct mod_i2c_api i2c_api = {
    .transmit_as_master = transmit_as_master,
    .receive_as_master = receive_as_master,
    .transmit_then_receive_as_master = transmit_then_receive_as_master,
};

/*
 * Driver response API
 */
static void transaction_completed(fwk_id_t dev_id, int i2c_status)
{
    int status;
    struct fwk_event event;
    struct mod_i2c_event_param* param =
        (struct mod_i2c_event_param *)event.params;

    event = (struct fwk_event) {
        .target_id = dev_id,
        .source_id = dev_id,
        .id = mod_i2c_event_id_request_completed,
    };

    param->status = i2c_status;

    status = fwk_thread_put_event(&event);
    fwk_assert(status == FWK_SUCCESS);
}

static struct mod_i2c_driver_response_api driver_response_api = {
    .transaction_completed = transaction_completed,
};

/*
 * Framework handlers
 */
static int mod_i2c_init(fwk_id_t module_id,
                        unsigned int element_count,
                        const void *unused)
{
    ctx_table = fwk_mm_calloc(element_count, sizeof(ctx_table[0]));

    return FWK_SUCCESS;
}

static int mod_i2c_dev_init(fwk_id_t element_id,
                            unsigned int unused,
                            const void *data)
{
    struct mod_i2c_dev_ctx *ctx;

    ctx = ctx_table + fwk_id_get_element_idx(element_id);
    ctx->config = (struct mod_i2c_dev_config *)data;

    return FWK_SUCCESS;
}

static int mod_i2c_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct mod_i2c_dev_ctx *ctx;

    /*
     * Only bind in first round of calls
     * Nothing to do for module
     */
    if ((round > 0) || fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    ctx = ctx_table + fwk_id_get_element_idx(id);

    /* Bind to driver */
    status = fwk_module_bind(ctx->config->driver_id,
                             ctx->config->api_id,
                             &ctx->driver_api);
    if (status != FWK_SUCCESS)
        return status;

    if ((ctx->driver_api->transmit_as_master == NULL) ||
        (ctx->driver_api->receive_as_master == NULL))
        return FWK_E_DATA;

    return FWK_SUCCESS;
}

static int mod_i2c_process_bind_request(fwk_id_t source_id,
                                        fwk_id_t target_id,
                                        fwk_id_t api_id,
                                        const void **api)
{
    struct mod_i2c_dev_ctx *ctx;

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_PARAM;

    ctx = ctx_table + fwk_id_get_element_idx(target_id);

    if (fwk_id_is_equal(source_id, ctx->config->driver_id)) {
        if (fwk_id_is_equal(api_id, mod_i2c_api_id_driver_response))
            *api = &driver_response_api;
        else
            return FWK_E_PARAM;
    } else
        *api = &i2c_api;

    return FWK_SUCCESS;
}

static int respond_to_caller(
    fwk_id_t dev_id,
    struct mod_i2c_dev_ctx *ctx,
    int drv_status)
{
    int status;
    struct fwk_event resp;
    struct mod_i2c_event_param *param =
        (struct mod_i2c_event_param *)resp.params;

    status = fwk_thread_get_first_delayed_response(dev_id, &resp);
    if (status != FWK_SUCCESS)
        return status;

    param->status = (drv_status == FWK_SUCCESS) ? FWK_SUCCESS : FWK_E_DEVICE;

    return fwk_thread_put_event(&resp);
}

static int process_request(struct mod_i2c_dev_ctx *ctx, fwk_id_t event_id)
{
    int drv_status = FWK_E_PARAM;
    const struct mod_i2c_driver_api *driver_api = ctx->driver_api;
    fwk_id_t driver_id = ctx->config->driver_id;

    switch (fwk_id_get_event_idx(event_id)) {
    case MOD_I2C_EVENT_IDX_REQUEST_TRANSMIT:
        ctx->state = MOD_I2C_DEV_TX;
        drv_status = driver_api->transmit_as_master(driver_id, &ctx->request);
        break;

    case MOD_I2C_EVENT_IDX_REQUEST_TRANSMIT_THEN_RECEIVE:
        ctx->state = MOD_I2C_DEV_TX_RX;
        drv_status = driver_api->transmit_as_master(driver_id, &ctx->request);

        if (drv_status != FWK_SUCCESS) {
            /* The request has failed or been acknowledged */
            break;
        }
        /* fall through */

    case MOD_I2C_EVENT_IDX_REQUEST_RECEIVE:
        ctx->state = MOD_I2C_DEV_RX;
        drv_status = driver_api->receive_as_master(driver_id, &ctx->request);
        break;
    }

    return drv_status;
}

static int reload(fwk_id_t dev_id, struct mod_i2c_dev_ctx *ctx)
{
    int status;
    bool is_empty;
    struct fwk_event event;

    status = fwk_thread_is_delayed_response_list_empty(dev_id, &is_empty);
    if (status != FWK_SUCCESS)
        return status;

    if (is_empty)
        ctx->state = MOD_I2C_DEV_IDLE;
    else {
        ctx->state = MOD_I2C_DEV_RELOAD;
        event = (struct fwk_event) {
            .target_id = dev_id,
            .id = mod_i2c_event_id_reload,
        };
        status = fwk_thread_put_event(&event);
   }

   return status;
}

static int process_next_request(fwk_id_t dev_id, struct mod_i2c_dev_ctx *ctx)
{
    int status, drv_status;
    bool is_empty;
    struct fwk_event delayed_response;
    struct mod_i2c_request *request;
    struct mod_i2c_event_param *event_param;

    status = fwk_thread_is_delayed_response_list_empty(dev_id, &is_empty);
    if (status != FWK_SUCCESS)
        return status;

    if (is_empty) {
        ctx->state = MOD_I2C_DEV_IDLE;
        return FWK_SUCCESS;
    }

    status = fwk_thread_get_first_delayed_response(
        dev_id, &delayed_response);
    if (status != FWK_SUCCESS)
        return status;

    request = (struct mod_i2c_request *)delayed_response.params;
    ctx->request = *request;

    drv_status = process_request(ctx, delayed_response.id);
    if (drv_status != FWK_PENDING) {
        event_param = (struct mod_i2c_event_param *)delayed_response.params;
        event_param->status = drv_status;

        status = fwk_thread_put_event(&delayed_response);
        if (status == FWK_SUCCESS)
            status = reload(dev_id, ctx);
    }

    return status;
}

static int mod_i2c_process_event(const struct fwk_event *event,
                                 struct fwk_event *resp_event)
{
    fwk_id_t dev_id;
    int status, drv_status;
    bool is_request;
    struct mod_i2c_dev_ctx *ctx;
    struct mod_i2c_request *request;
    struct mod_i2c_event_param *event_param, *resp_param;

    dev_id = event->target_id;
    get_ctx(dev_id, &ctx);

    is_request = fwk_id_get_event_idx(event->id) < MOD_I2C_EVENT_IDX_COUNT;

    if (is_request) {
        if (ctx->state == MOD_I2C_DEV_PANIC) {
            event_param = (struct mod_i2c_event_param *)resp_event->params;
            event_param->status = FWK_E_PANIC;

            return FWK_SUCCESS;
        } else if (ctx->state != MOD_I2C_DEV_IDLE) {
            resp_event->is_delayed_response = true;

            return FWK_SUCCESS;
        }

        request = (struct mod_i2c_request *)event->params;
        ctx->request = *request;

        drv_status = process_request(ctx, event->id);

        if (drv_status == FWK_PENDING)
            resp_event->is_delayed_response = true;
        else {
            /* The request has succeeded or failed, respond now */
            resp_param = (struct mod_i2c_event_param *)resp_event->params;

            resp_param->status = (drv_status == FWK_SUCCESS) ?
                                 FWK_SUCCESS : FWK_E_DEVICE;
            ctx->state = MOD_I2C_DEV_IDLE;
        }

        return FWK_SUCCESS;
    }

    switch (fwk_id_get_event_idx(event->id)) {
    case MOD_I2C_EVENT_IDX_REQUEST_COMPLETED:
        event_param = (struct mod_i2c_event_param *)event->params;

        if ((ctx->state == MOD_I2C_DEV_TX) || (ctx->state == MOD_I2C_DEV_RX)) {
            status = respond_to_caller(dev_id, ctx, event_param->status);
            if (status == FWK_SUCCESS)
                status = process_next_request(dev_id, ctx);

        } else if (ctx->state == MOD_I2C_DEV_TX_RX) {
            drv_status = event_param->status;

            if (drv_status == FWK_SUCCESS) {
                /* The TX request succeeded, proceed with the RX */
                ctx->state = MOD_I2C_DEV_RX;

                drv_status = ctx->driver_api->receive_as_master(
                    ctx->config->driver_id,
                    &ctx->request);

                if (drv_status == FWK_PENDING) {
                    status = FWK_SUCCESS;
                    break;
                }
            }

            status = respond_to_caller(dev_id, ctx, drv_status);
            if (status == FWK_SUCCESS)
                status = process_next_request(dev_id, ctx);
        } else
            status = FWK_E_STATE;

        break;

    case MOD_I2C_EVENT_IDX_RELOAD:
        status = process_next_request(dev_id, ctx);
        break;

    default:
        status = FWK_E_PANIC;
        break;
    }

    if (status != FWK_SUCCESS)
        ctx->state = MOD_I2C_DEV_PANIC;

    return status;
}

const struct fwk_module module_i2c = {
    .name = "I2C",
    .type = FWK_MODULE_TYPE_HAL,
    .api_count = MOD_I2C_API_IDX_COUNT,
    .event_count = MOD_I2C_EVENT_IDX_TOTAL_COUNT,
    .init = mod_i2c_init,
    .element_init = mod_i2c_dev_init,
    .bind = mod_i2c_bind,
    .process_bind_request = mod_i2c_process_bind_request,
    .process_event = mod_i2c_process_event,
};

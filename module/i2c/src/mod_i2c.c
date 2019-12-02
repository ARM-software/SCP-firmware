/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdbool.h>
#include <string.h>
#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_thread.h>
#include <mod_i2c.h>

enum mod_i2c_request_state {
    MOD_I2C_REQUEST_IDLE,
    MOD_I2C_REQUEST_TX,
    MOD_I2C_REQUEST_RX,
    MOD_I2C_REQUEST_TX_RX,
};

struct mod_i2c_dev_ctx {
    const struct mod_i2c_dev_config *config;
    const struct mod_i2c_driver_api *driver_api;
    struct mod_i2c_request request;
    uint32_t cookie;
    enum mod_i2c_request_state state;
};

static struct mod_i2c_dev_ctx *ctx_table;

enum mod_i2c_internal_event_idx {
    MOD_I2C_EVENT_IDX_REQUEST_COMPLETED = MOD_I2C_EVENT_IDX_COUNT,
    MOD_I2C_EVENT_IDX_TOTAL_COUNT,
};

/*! Request completed event identifier */
static const fwk_id_t mod_i2c_event_id_request_completed = FWK_ID_EVENT_INIT(
    FWK_MODULE_IDX_I2C, MOD_I2C_EVENT_IDX_REQUEST_COMPLETED);

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
    struct mod_i2c_dev_ctx *ctx;
    struct mod_i2c_request *event_param =
        (struct mod_i2c_request *)event.params;

    get_ctx(dev_id, &ctx);

    /* The slave address should be on 7 bits */
    if (!fwk_expect(request->slave_address < 0x80))
        return FWK_E_PARAM;

    /* Check whether an I2C request is already on-going */
    if (ctx->state != MOD_I2C_REQUEST_IDLE)
        return FWK_E_BUSY;

    /* Create the request */
    event = (struct fwk_event) {
        .target_id = dev_id,
        .response_requested = true,
    };

    *event_param = *request;

    if ((request->transmit_byte_count > 0) &&
        (request->receive_byte_count > 0)) {
        ctx->state = MOD_I2C_REQUEST_TX_RX;
        event.id = mod_i2c_event_id_request_tx_rx;
    } else if (request->transmit_byte_count > 0) {
        ctx->state = MOD_I2C_REQUEST_TX;
        event.id = mod_i2c_event_id_request_tx;
    } else if (request->receive_byte_count > 0) {
        ctx->state = MOD_I2C_REQUEST_RX;
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

    ctx->state = MOD_I2C_REQUEST_IDLE;

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

    if (ctx_table == NULL)
        return FWK_E_NOMEM;

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

static int process_request(int status,
                           struct mod_i2c_dev_ctx *ctx,
                           const struct fwk_event *event,
                           struct fwk_event *resp_event)
{
    if (status == FWK_PENDING) {
        /* The request has not completed, respond later */
        ctx->cookie = event->cookie;
        resp_event->is_delayed_response = true;

        return FWK_SUCCESS;
    } else {
        /* The request has succeeded or failed, respond now */
        struct mod_i2c_event_param *resp_param =
            (struct mod_i2c_event_param *)resp_event->params;

        if (status != FWK_SUCCESS)
            status = FWK_E_DEVICE;

        resp_param->status = status;
        ctx->state = MOD_I2C_REQUEST_IDLE;

        return status;
    }
}

static int respond_to_caller(int event_status,
                             struct mod_i2c_dev_ctx *ctx,
                             const struct fwk_event *event)
{
    int status;
    struct fwk_event resp;

    ctx->state = MOD_I2C_REQUEST_IDLE;

    status = fwk_thread_get_delayed_response(event->target_id,
        ctx->cookie, &resp);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    struct mod_i2c_event_param *param =
        (struct mod_i2c_event_param *)resp.params;

    param->status = event_status;

    return fwk_thread_put_event(&resp);
}

static int mod_i2c_process_event(const struct fwk_event *event,
                                 struct fwk_event *resp_event)
{
    int status = FWK_E_PARAM;
    int drv_status;
    struct mod_i2c_dev_ctx *ctx;
    struct mod_i2c_event_param *event_param =
        (struct mod_i2c_event_param *)event->params;
    struct mod_i2c_request *request = (struct mod_i2c_request *)event->params;

    get_ctx(event->target_id, &ctx);

    switch (fwk_id_get_event_idx(event->id)) {
    case MOD_I2C_EVENT_IDX_REQUEST_TRANSMIT:
        ctx->request = *request;

        drv_status = ctx->driver_api->transmit_as_master(ctx->config->driver_id,
            &ctx->request);

        status = process_request(drv_status, ctx, event, resp_event);
        break;

    case MOD_I2C_EVENT_IDX_REQUEST_TRANSMIT_THEN_RECEIVE:
        ctx->request = *request;

        drv_status = ctx->driver_api->transmit_as_master(ctx->config->driver_id,
            &ctx->request);

        if (drv_status != FWK_SUCCESS) {
            /* The request has failed or been acknowledged */
            status = process_request(drv_status, ctx, event, resp_event);

            break;
        } else {
            /*
             * The TX request has succeeded, update state and proceed to the RX
             * request.
             */
            ctx->state = MOD_I2C_REQUEST_RX;
        }
        /* fall through */

    case MOD_I2C_EVENT_IDX_REQUEST_RECEIVE:
        ctx->request = *request;

        drv_status = ctx->driver_api->receive_as_master(ctx->config->driver_id,
            &ctx->request);

        status = process_request(drv_status, ctx, event, resp_event);

        break;

    case MOD_I2C_EVENT_IDX_REQUEST_COMPLETED:
        if ((ctx->state == MOD_I2C_REQUEST_TX) ||
            (ctx->state == MOD_I2C_REQUEST_RX)) {
            status = respond_to_caller(event_param->status, ctx, event);
        } else if (ctx->state == MOD_I2C_REQUEST_TX_RX) {
            if (event_param->status == FWK_SUCCESS) {
                /* The TX request succeeded, proceed with the RX */
                ctx->state = MOD_I2C_REQUEST_RX;

                drv_status =
                    ctx->driver_api->receive_as_master(ctx->config->driver_id,
                        &ctx->request);
                if (drv_status == FWK_PENDING)
                    status = FWK_SUCCESS;
                else
                    status = respond_to_caller(drv_status, ctx, event);
            } else {
                /* The request failed, respond */
                status = respond_to_caller(event_param->status, ctx, event);
            }
        } else
            status = FWK_E_STATE;

        break;
    }

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

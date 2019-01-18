/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdbool.h>
#include <string.h>
#include <fwk_assert.h>
#include <fwk_errno.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_thread.h>
#include <mod_i2c.h>

struct mod_i2c_dev_ctx {
    const struct mod_i2c_dev_config *config;
    const struct mod_i2c_driver_api *driver_api;
    struct mod_i2c_request request;
    bool busy;
    uint32_t cookie;
};

static struct mod_i2c_dev_ctx *ctx_table;

/*
 * Static helpers
 */
static int get_ctx(fwk_id_t id, struct mod_i2c_dev_ctx **ctx)
{
    int status;

    status = fwk_module_check_call(id);
    if (status != FWK_SUCCESS)
        return FWK_E_PARAM;

    *ctx = ctx_table + fwk_id_get_element_idx(id);

    return FWK_SUCCESS;
}

static int create_i2c_request(fwk_id_t dev_id, uint8_t slave_address,
    uint8_t *transmit_data, uint8_t *receive_data, uint8_t transmit_byte_count,
    uint8_t receive_byte_count)
{
    int status;
    struct fwk_event event;
    struct mod_i2c_dev_ctx *ctx;
    struct mod_i2c_request *event_param =
        (struct mod_i2c_request *)event.params;

    fwk_assert(fwk_module_is_valid_element_id(dev_id));

    status = get_ctx(dev_id, &ctx);
    if (status != FWK_SUCCESS)
        return status;

    /* The slave address should be on 7 bits */
    if (!fwk_expect(slave_address < 0x80))
        return FWK_E_PARAM;

    /* Check whether an I2C request is already on-going */
    if (ctx->busy)
        return FWK_E_BUSY;

    ctx->busy = true;

    /* Create the request */
    event = (struct fwk_event) {
        .id = mod_i2c_event_id_request,
        .target_id = dev_id,
        .response_requested = true,
    };

    *event_param = (struct mod_i2c_request) {
        .slave_address = slave_address,
        .transmit_data = transmit_data,
        .receive_data = receive_data,
        .transmit_byte_count = transmit_byte_count,
        .receive_byte_count = receive_byte_count,
    };

    return fwk_thread_put_event(&event);
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

    return create_i2c_request(dev_id, slave_address, data, NULL, byte_count, 0);
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

    return create_i2c_request(dev_id, slave_address, NULL, data, 0, byte_count);
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

    return create_i2c_request(dev_id, slave_address, transmit_data,
        receive_data, transmit_byte_count, receive_byte_count);
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

static int mod_i2c_process_event(const struct fwk_event *event,
                                 struct fwk_event *resp_event)
{
    int status;
    struct fwk_event resp;
    struct mod_i2c_dev_ctx *ctx;
    struct mod_i2c_event_param *param =
        (struct mod_i2c_event_param *)resp.params;
    struct mod_i2c_event_param *event_param =
        (struct mod_i2c_event_param *)event->params;
    struct mod_i2c_request *request = (struct mod_i2c_request *)event->params;

    fwk_assert(fwk_module_is_valid_element_id(event->target_id));

    status = get_ctx(event->target_id, &ctx);
    if (status != FWK_SUCCESS)
        return status;

    if (fwk_id_is_equal(event->id, mod_i2c_event_id_request)) {
        ctx->request = *request;

        if (ctx->request.transmit_byte_count != 0) {
            status = ctx->driver_api->transmit_as_master(ctx->config->driver_id,
                &ctx->request);
        } else {
            status = ctx->driver_api->receive_as_master(ctx->config->driver_id,
                &ctx->request);
        }
        ctx->cookie = event->cookie;
        resp_event->is_delayed_response = true;

    } else if (fwk_id_is_equal(event->id, mod_i2c_event_id_request_completed)) {

        if ((ctx->request.transmit_byte_count != 0) &&
            (ctx->request.receive_byte_count != 0) &&
            (event_param->status == FWK_SUCCESS)) {
            /* A receive operation needs to be performed */
            ctx->request.transmit_byte_count = 0;
            status = ctx->driver_api->receive_as_master(ctx->config->driver_id,
                &ctx->request);
        } else {
            /* The transaction is completed or aborted. */
            ctx->busy = false;
            status = fwk_thread_get_delayed_response(event->target_id,
                ctx->cookie, &resp);
            if (status != FWK_SUCCESS)
                return FWK_E_DEVICE;

            param->status = event_param->status;
            status = fwk_thread_put_event(&resp);
        }

    } else
        return FWK_E_PARAM;

    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}

const struct fwk_module module_i2c = {
    .name = "I2C",
    .type = FWK_MODULE_TYPE_HAL,
    .api_count = MOD_I2C_API_IDX_COUNT,
    .event_count = MOD_I2C_EVENT_IDX_COUNT,
    .init = mod_i2c_init,
    .element_init = mod_i2c_dev_init,
    .bind = mod_i2c_bind,
    .process_bind_request = mod_i2c_process_bind_request,
    .process_event = mod_i2c_process_event,
};

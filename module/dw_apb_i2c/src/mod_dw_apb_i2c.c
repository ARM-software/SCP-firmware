/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <dw_apb_i2c.h>

#include <mod_dw_apb_i2c.h>
#include <mod_i2c.h>
#include <mod_timer.h>

#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>

struct dw_apb_i2c_ctx {
    const struct mod_dw_apb_i2c_dev_config *config;
    const struct mod_i2c_driver_response_api *i2c_api;
    const struct mod_timer_api *timer_api;
    fwk_id_t i2c_id;
    struct dw_apb_i2c_reg *i2c_reg;
    bool read_on_going;
    uint8_t byte_count;
    uint8_t *data;
};

static struct dw_apb_i2c_ctx *ctx_table;

/*
 * Static helpers
 */
static bool is_i2c_disabled(void *param)
{
    struct dw_apb_i2c_reg *i2c_reg = (struct dw_apb_i2c_reg *)param;

    return ((i2c_reg->IC_ENABLE_STATUS & IC_ENABLE_STATUS_MASK) ==
        IC_ENABLE_STATUS_DISABLED);
}

static int disable_i2c(struct dw_apb_i2c_ctx *ctx)
{
    int status;
    fwk_id_t timer_id;
    const struct mod_timer_api *timer_api;
    struct dw_apb_i2c_reg *i2c_reg;

    timer_api = ctx->timer_api;
    timer_id = ctx->config->timer_id;
    i2c_reg = ctx->i2c_reg;

    /* Check whether the device is already disabled */
    if (is_i2c_disabled(i2c_reg))
        return FWK_SUCCESS;

    /* The bus should be idle */
    if ((ctx->i2c_reg->IC_STATUS & IC_STATUS_MST_ACTIVITY_MASK) != 0)
        return FWK_E_DEVICE;

    /* Disable the I2C device */
    ctx->i2c_reg->IC_ENABLE = IC_ENABLE_STATUS_DISABLED;

    /* Wait until the device is disabled */
    status = timer_api->wait(timer_id, I2C_TIMEOUT_US, is_i2c_disabled,
        i2c_reg);
    if (status != FWK_SUCCESS)
        return FWK_E_TIMEOUT;

    return FWK_SUCCESS;
}

static int enable_i2c(struct dw_apb_i2c_ctx *ctx, uint8_t slave_address)
{
    int status;
    struct dw_apb_i2c_reg *i2c_reg;

    i2c_reg = ctx->i2c_reg;

    /* Disable the I2C device to configure it */
    status = disable_i2c(ctx);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    /* Program the slave address */
    i2c_reg->IC_TAR = (slave_address & IC_TAR_ADDRESS);

    /* Enable STOP detected interrupt and TX aborted interrupt */
    i2c_reg->IC_INTR_MASK = (IC_INTR_STOP_DET_MASK | IC_INTR_TX_ABRT_MASK);

    /* Enable the I2C device */
    i2c_reg->IC_ENABLE = IC_ENABLE_STATUS_ENABLED;

    return FWK_SUCCESS;
}

/*
 * An IRQ is triggered if the transaction has been completed successfully or
 * if the transaction has been aborted.
 */
static void i2c_isr(uintptr_t data)
{
    unsigned int i;
    int i2c_status = FWK_E_DEVICE;
    struct dw_apb_i2c_reg *i2c_reg;
    struct dw_apb_i2c_ctx *ctx = (struct dw_apb_i2c_ctx *)data;

    i2c_reg = ctx->i2c_reg;

    /* The transaction has completed successfully */
    if (i2c_reg->IC_INTR_STAT & IC_INTR_STOP_DET_MASK) {
        i2c_reg->IC_CLR_STOP_DET;
        i2c_status = FWK_SUCCESS;
        if (ctx->read_on_going) {
            ctx->read_on_going = false;
            /* Read the data from the device buffer */
            for (i = 0; i < ctx->byte_count; i++)
                ctx->data[i] =
                    (uint8_t)(i2c_reg->IC_DATA_CMD & IC_DATA_CMD_DATA_MASK);
        }
    }

    /* The transaction has been aborted */
    if (i2c_reg->IC_INTR_STAT & IC_INTR_TX_ABRT_MASK)
        i2c_reg->IC_CLR_TX_ABRT;

    ctx->i2c_api->transaction_completed(ctx->i2c_id, i2c_status);
}

/*
 * Driver API
 */
static int transmit_as_master(fwk_id_t dev_id,
                              struct mod_i2c_request *transmit_request)
{
    int status;
    unsigned int sent_bytes;
    struct dw_apb_i2c_ctx *ctx;


    if (transmit_request->transmit_byte_count > I2C_TRANSMIT_BUFFER_LENGTH)
        return FWK_E_SUPPORT;

    if (transmit_request->slave_address == 0)
        return FWK_E_PARAM;

    ctx = ctx_table + fwk_id_get_element_idx(dev_id);

    status = enable_i2c(ctx, transmit_request->slave_address);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    /* The program of the I2C controller cannot be interrupted. */
    fwk_interrupt_global_disable();

    for (sent_bytes = 0; sent_bytes < transmit_request->transmit_byte_count;
        sent_bytes++)
        ctx->i2c_reg->IC_DATA_CMD = transmit_request->transmit_data[sent_bytes];

    fwk_interrupt_global_enable();

    /*
     * The data has been pushed to the I2C FIFO for transmission to the
     * target device. An interrupt will signal the completion of the
     * transfer. The i2c_isr() interrupt handler will be invoked to notify
     * the caller.
     */
    return FWK_PENDING;
}

static int receive_as_master(fwk_id_t dev_id,
                             struct mod_i2c_request *receive_request)
{
    int status;
    unsigned int i;
    struct dw_apb_i2c_ctx *ctx;


    if (receive_request->receive_byte_count > I2C_RECEIVE_BUFFER_LENGTH)
        return FWK_E_SUPPORT;

    if (receive_request->slave_address == 0)
        return FWK_E_PARAM;

    ctx = ctx_table + fwk_id_get_element_idx(dev_id);

    ctx->byte_count = receive_request->receive_byte_count;
    ctx->data = receive_request->receive_data;
    ctx->read_on_going = true;

    status = enable_i2c(ctx, receive_request->slave_address);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    /* The program of the I2C controller cannot be interrupted. */
    fwk_interrupt_global_disable();

    /* Program the I2C controller with the expected reply length in bytes. */
    for (i = 0; i < receive_request->receive_byte_count; i++)
        ctx->i2c_reg->IC_DATA_CMD = IC_DATA_CMD_READ;

    fwk_interrupt_global_enable();

    /*
     * The command has been sent to the I2C for requesting data from
     * the target device. An interrupt will signal the completion of the
     * transfer. The i2c_isr() interrupt handler will be invoked to notify
     * the caller.
     */
    return FWK_PENDING;
}

static const struct mod_i2c_driver_api driver_api = {
    .transmit_as_master = transmit_as_master,
    .receive_as_master = receive_as_master
};

/*
 * Framework handlers
 */
static int dw_apb_i2c_init(fwk_id_t module_id,
                               unsigned int element_count,
                               const void *data)
{
    ctx_table = fwk_mm_calloc(element_count, sizeof(*ctx_table));

    return FWK_SUCCESS;
}

static int dw_apb_i2c_element_init(fwk_id_t element_id,
                                       unsigned int sub_element_count,
                                       const void *data)
{
    struct mod_dw_apb_i2c_dev_config *config =
        (struct mod_dw_apb_i2c_dev_config *)data;

    if (config->reg == 0)
        return FWK_E_DATA;

    ctx_table[fwk_id_get_element_idx(element_id)].config = config;
    ctx_table[fwk_id_get_element_idx(element_id)].i2c_reg =
        (struct dw_apb_i2c_reg *)config->reg;

    return FWK_SUCCESS;
}

static int dw_apb_i2c_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct dw_apb_i2c_ctx *ctx;
    const struct mod_dw_apb_i2c_dev_config *config;

    if (!fwk_module_is_valid_element_id(id) || (round == 0))
        return FWK_SUCCESS;

    ctx = ctx_table + fwk_id_get_element_idx(id);
    config = ctx->config;

    status = fwk_module_bind(config->timer_id, MOD_TIMER_API_ID_TIMER,
        &ctx->timer_api);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_module_bind(ctx->i2c_id, mod_i2c_api_id_driver_response,
        &ctx->i2c_api);
}

static int dw_apb_i2c_process_bind_request(fwk_id_t source_id,
                                           fwk_id_t target_id,
                                           fwk_id_t api_id,
                                           const void **api)
{
    struct dw_apb_i2c_ctx *ctx;

    if (!fwk_module_is_valid_element_id(target_id))
        return FWK_E_PARAM;

    ctx = ctx_table + fwk_id_get_element_idx(target_id);

    if (!fwk_id_is_equal(api_id, mod_dw_apb_i2c_api_id_driver))
        return FWK_E_PARAM;

    ctx->i2c_id = source_id;

    *api = &driver_api;

    return FWK_SUCCESS;
}

static int dw_apb_i2c_start(fwk_id_t id)
{
    int status;
    struct dw_apb_i2c_ctx *ctx;
    unsigned int i2c_irq;

    /* Nothing to do for the module */
    if (!fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    ctx = ctx_table + fwk_id_get_element_idx(id);
    i2c_irq = ctx->config->i2c_irq;

    status = fwk_interrupt_set_isr_param(i2c_irq, i2c_isr, (uintptr_t)ctx);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    status = fwk_interrupt_clear_pending(i2c_irq);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    status = fwk_interrupt_enable(i2c_irq);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}

const struct fwk_module module_dw_apb_i2c = {
    .name = "DW APB I2C",
    .api_count = MOD_DW_APB_I2C_API_IDX_COUNT,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = dw_apb_i2c_init,
    .element_init = dw_apb_i2c_element_init,
    .bind = dw_apb_i2c_bind,
    .start = dw_apb_i2c_start,
    .process_bind_request = dw_apb_i2c_process_bind_request,
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP I2C Driver
 */

#include "n1sdp_core.h"

#include <internal/n1sdp_i2c.h>

#include <mod_n1sdp_i2c.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <string.h>

/* I2C divider calculation values */
#define I2C_HZ                  22
#define I2C_DIV                 1
#define I2C_DIV_MAX             0x3F
#define I2C_DIV_VAL             4
#define I2C_DIV_VAL1            3
#define I2C_DIV_SHIFT           6

/* Timeout value */
#define I2C_TRANSFER_TIMEOUT    10000U

/* Read-modify-write a register field and leave other bits intact. */
#define I2C_REG_RMW(reg, mask, shift, val) ((reg) = (((reg) & ~(mask)) | \
                                            (((val) << (shift)) & (mask))))

/* Write a single register field. */
#define I2C_REG_W(reg, mask, shift, val)   ((reg) = ((val) << (shift)) & (mask))

/* Read a register field. */
#define I2C_REG_R(reg, mask, shift)        (((reg) & (mask)) >> (shift))

/* ISR error mask */
#define I2C_ISR_ERROR_MASK      (I2C_ISR_RXUNF_MASK | \
                                 I2C_ISR_TXOVF_MASK | \
                                 I2C_ISR_RXOVF_MASK | \
                                 I2C_ISR_NACK_MASK  | \
                                 I2C_ISR_TO_MASK)

/* Device context */
struct n1sdp_i2c_dev_ctx {
    /* Pointer to the device configuration */
    const struct mod_n1sdp_i2c_device_config *config;

    /* Pointer to associated I2C registers */
    struct i2c_reg *reg;

    /* Track repeated transfer of data */
    bool perform_repeat_start;

    /* IRQ mode data context */
    struct mod_n1sdp_i2c_irq_data_ctx irq_data_ctx;

    /* I2C ID */
    unsigned int i2c_id;

    /* I2C callback pointer */
    void (*i2c_callback)(struct n1sdp_i2c_dev_ctx *ctx,
                         enum mod_n1sdp_i2c_notifications cb_type);
};

/* Module context */
struct n1sdp_i2c_ctx {
    /* Table of device contexts */
    struct n1sdp_i2c_dev_ctx *device_ctx_table;
};

static struct n1sdp_i2c_ctx i2c_ctx;

/* Callback function */
static void i2c_callback_fn(struct n1sdp_i2c_dev_ctx *ctx,
    enum mod_n1sdp_i2c_notifications cb_type)
{
    unsigned int i;
    struct fwk_event c2c_event;

    fwk_assert(ctx != NULL);

    if (!fwk_id_is_equal(ctx->config->callback_mod_id, FWK_ID_NONE)) {
        memset((void *)&c2c_event, 0, sizeof(struct fwk_event));
        switch (cb_type) {
        case MOD_N1SDP_I2C_NOTIFICATION_IDX_TX:
            c2c_event.id = mod_n1sdp_i2c_notification_id_slave_tx;
            break;
        case MOD_N1SDP_I2C_NOTIFICATION_IDX_RX:
            c2c_event.id = mod_n1sdp_i2c_notification_id_slave_rx;
            break;
        case MOD_N1SDP_I2C_NOTIFICATION_IDX_ERROR:
            c2c_event.id = mod_n1sdp_i2c_notification_id_slave_error;
            break;
        default:
            fwk_assert(false);
            return;
        }
        c2c_event.response_requested = false;
        c2c_event.source_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_N1SDP_I2C,
                                             ctx->i2c_id);
        c2c_event.target_id = ctx->config->callback_mod_id;
        fwk_notification_notify(&c2c_event, &i);
    }
}

static void clear_isr(struct n1sdp_i2c_dev_ctx *device_ctx)
{
    uint16_t reg;

    reg = I2C_REG_R(device_ctx->reg->ISR, I2C_ISR_MASK, I2C_ISR_SHIFT);
    I2C_REG_W(device_ctx->reg->ISR, I2C_ISR_MASK, I2C_ISR_SHIFT, reg);
}

static void i2c_isr(uintptr_t data)
{
    struct n1sdp_i2c_dev_ctx *ctx;
    uint16_t isr = 0;
    uint32_t i = 0;

    ctx = (struct n1sdp_i2c_dev_ctx *)data;

    /* Save ISR and clear the register */
    isr = ctx->reg->ISR;
    ctx->reg->ISR = isr;

    if (isr == 0)
        return;

    if (isr & I2C_ISR_ERROR_MASK)
        goto i2c_error;

    switch (ctx->irq_data_ctx.state) {
    case MOD_N1SDP_I2C_STATE_TX:
        /* Check for data interrupt */
        if (isr & I2C_ISR_DATA_MASK) {
            for (i = 0; (i < (I2C_TSR_TANSFER_SIZE - 2)) &&
                        (ctx->irq_data_ctx.index < ctx->irq_data_ctx.size);
                 i++) {
                ctx->reg->DR = ctx->irq_data_ctx.data[ctx->irq_data_ctx.index];
                ctx->irq_data_ctx.index++;
            }
        }
        break;

    case MOD_N1SDP_I2C_STATE_RX:
        /* Read new data from FIFO. */
        for (; (ctx->reg->SR & I2C_SR_RXDV_MASK) &&
               (ctx->irq_data_ctx.index < ctx->irq_data_ctx.size);
             ctx->irq_data_ctx.index++) {
            ctx->irq_data_ctx.data[ctx->irq_data_ctx.index] =
                (uint8_t)ctx->reg->DR;
        }

        /* Make sure transaction sizes align. */
        if (ctx->reg->SR & I2C_SR_RXDV_MASK)
            goto i2c_error;

        /* If master, reload TSR if more data expected. */
        if ((ctx->config->mode == MOD_N1SDP_I2C_MASTER_MODE) &&
            (ctx->irq_data_ctx.index < ctx->irq_data_ctx.size)) {
            if ((ctx->irq_data_ctx.size - ctx->irq_data_ctx.index) >
                I2C_TSR_TANSFER_SIZE)
                ctx->reg->TSR = I2C_TSR_TANSFER_SIZE;
            else
                ctx->reg->TSR = (ctx->irq_data_ctx.size -
                                 ctx->irq_data_ctx.index);
        }
        break;

    default:
        goto i2c_error;
    }

    /* Check for completion interrupt */
    if (isr & I2C_ISR_COMP_MASK) {
        /* Make sure the correct amount of data was transferred */
        if (ctx->irq_data_ctx.index != ctx->irq_data_ctx.size)
            goto i2c_error;

        ctx->irq_data_ctx.busy = false;
        I2C_REG_W(ctx->reg->AR, I2C_AR_ADD7_MASK, I2C_AR_ADD7_SHIFT, 0);

        /* Run completion callback */
        if ((ctx->irq_data_ctx.state == MOD_N1SDP_I2C_STATE_TX) &&
            (ctx->i2c_callback))
            ctx->i2c_callback(ctx, MOD_N1SDP_I2C_NOTIFICATION_IDX_TX);
        else if ((ctx->irq_data_ctx.state == MOD_N1SDP_I2C_STATE_RX) &&
                 (ctx->i2c_callback))
            ctx->i2c_callback(ctx, MOD_N1SDP_I2C_NOTIFICATION_IDX_RX);

        /* Reset I2C driver state */
        ctx->reg->IDR = 0xFFFF;
        I2C_REG_RMW(ctx->reg->CR, I2C_CR_CLRFIFO_MASK, I2C_CR_CLRFIFO_SHIFT,
                    I2C_CLRFIFO_ON);
        ctx->irq_data_ctx.data = NULL;
    }
    return;

i2c_error:
    if (ctx->i2c_callback)
        ctx->i2c_callback(ctx, MOD_N1SDP_I2C_NOTIFICATION_IDX_ERROR);

    ctx->reg->IDR = 0xFFFF;
    I2C_REG_RMW(ctx->reg->CR, I2C_CR_CLRFIFO_MASK, I2C_CR_CLRFIFO_SHIFT,
                I2C_CLRFIFO_ON);
    ctx->irq_data_ctx.data = NULL;
}

/*
 * Module I2C driver APIs
 */

/*
 * I2C master polled mode driver API
 */
static int i2c_master_read_polled(fwk_id_t device_id,
    uint16_t address, char *data, uint16_t length)
{
    bool complete = false;
    unsigned int timeout;
    struct n1sdp_i2c_dev_ctx *device_ctx;

    fwk_assert(length != 0);

    device_ctx = &i2c_ctx.device_ctx_table[fwk_id_get_element_idx(device_id)];

    I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_HOLD_MASK, I2C_CR_HOLD_SHIFT,
                I2C_HOLD_ON);

    I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_RW_MASK, I2C_CR_RW_SHIFT,
                I2C_RW_READ);

    if (!device_ctx->perform_repeat_start)
        I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_CLRFIFO_MASK,
                    I2C_CR_CLRFIFO_SHIFT, I2C_CLRFIFO_ON);

    clear_isr(device_ctx);

    /* Set the expected number of bytes to be received */
    I2C_REG_RMW(device_ctx->reg->TSR, I2C_TSR_SIZE_MASK, I2C_TSR_SIZE_SHIFT,
                length);

    I2C_REG_RMW(device_ctx->reg->AR, I2C_AR_ADD7_MASK, I2C_AR_ADD7_SHIFT,
                address);

    /* Wait for transfer completion */
    timeout = I2C_TRANSFER_TIMEOUT;
    while (!complete && (timeout != 0)) {
        if (I2C_REG_R(device_ctx->reg->SR, I2C_SR_RXDV_MASK,
                      I2C_SR_RXDV_SHIFT)) {
            if (I2C_REG_R(device_ctx->reg->ISR, I2C_ISR_COMP_MASK,
                          I2C_ISR_COMP_SHIFT))
                complete = true;
        }
        timeout--;
    }

    if (timeout == 0) {
        I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_HOLD_MASK, I2C_CR_HOLD_SHIFT,
                    I2C_HOLD_OFF);
        return FWK_E_STATE;
    }

    timeout = I2C_TRANSFER_TIMEOUT;
    while ((length != 0) && (timeout != 0)) {
        if (I2C_REG_R(device_ctx->reg->SR, I2C_SR_RXDV_MASK,
                      I2C_SR_RXDV_SHIFT)) {
            *data++ = I2C_REG_R(device_ctx->reg->DR, I2C_DR_DATA_MASK,
                                I2C_DR_DATA_SHIFT);
            length--;
        }
        timeout--;
    }

    clear_isr(device_ctx);

    /* Clear the hold bit to signify the end of the read sequence */
    I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_HOLD_MASK, I2C_CR_HOLD_SHIFT,
                I2C_HOLD_OFF);

    clear_isr(device_ctx);

    device_ctx->perform_repeat_start = false;

    return FWK_SUCCESS;
}

static int i2c_master_write_polled(fwk_id_t device_id, uint16_t address,
    const char *data, uint16_t length, bool stop)
{
    bool complete = false;
    unsigned int timeout;
    struct n1sdp_i2c_dev_ctx *device_ctx;
    uint16_t sr, isr;

    fwk_assert(length != 0);

    device_ctx = &i2c_ctx.device_ctx_table[fwk_id_get_element_idx(device_id)];

    I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_HOLD_MASK, I2C_CR_HOLD_SHIFT,
                I2C_HOLD_ON);

    I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_RW_MASK, I2C_CR_RW_SHIFT,
                I2C_RW_WRITE);

    clear_isr(device_ctx);

    /* Set the expected number of bytes to be transmitted */
    I2C_REG_RMW(device_ctx->reg->TSR, I2C_TSR_SIZE_MASK, I2C_TSR_SIZE_SHIFT,
                length);

    if (!device_ctx->perform_repeat_start) {
        /*
         * Preload the FIFO with a single data byte. If this write is to
         * generate a repeat start then don't write any data until the
         * address has been set.
         */
        I2C_REG_RMW(device_ctx->reg->DR, I2C_DR_DATA_MASK, I2C_DR_DATA_SHIFT,
                    *data++);
        length--;
    }

    /* Write the address, triggering the start of the transfer */
    I2C_REG_RMW(device_ctx->reg->AR, I2C_AR_ADD7_MASK, I2C_AR_ADD7_SHIFT,
                address);

    /* If there are further bytes to send, write to the FIFO sequentially */
    while (length != 0) {
        I2C_REG_RMW(device_ctx->reg->DR, I2C_DR_DATA_MASK, I2C_DR_DATA_SHIFT,
                    *data++);
        length--;
    }

    /* Wait for transfer completion */
    timeout = I2C_TRANSFER_TIMEOUT;
    while (!complete && (timeout != 0)) {
        sr = I2C_REG_R(device_ctx->reg->SR, I2C_SR_TXDV_MASK,
                       I2C_SR_TXDV_SHIFT);
        if (sr == 0) {
            isr = I2C_REG_R(device_ctx->reg->ISR, I2C_ISR_COMP_MASK,
                            I2C_ISR_COMP_SHIFT);
            if (isr != 0)
                complete = true;
        }
        timeout--;
    }

    if (timeout == 0) {
        /* Clear the hold bit to signify the end of the write sequence */
        I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_HOLD_MASK, I2C_CR_HOLD_SHIFT,
                    I2C_HOLD_OFF);
        clear_isr(device_ctx);
        return FWK_E_STATE;
    }

    clear_isr(device_ctx);

    if (stop) {
        /* Clear the hold bit to signify the end of the write sequence */
        I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_HOLD_MASK, I2C_CR_HOLD_SHIFT,
                    I2C_HOLD_OFF);
        clear_isr(device_ctx);
        device_ctx->perform_repeat_start = false;
    } else {
        /* Track repeated start so that the following read or write is aware */
        device_ctx->perform_repeat_start = true;
    }
    return FWK_SUCCESS;
}

static const struct mod_n1sdp_i2c_master_api_polled i2c_master_api_polled = {
    .read = i2c_master_read_polled,
    .write = i2c_master_write_polled,
};

/*
 * I2C slave interrupt mode driver API
 */
static int i2c_slave_write_irq(fwk_id_t device_id,
    uint8_t *data, uint8_t length)
{
    struct n1sdp_i2c_dev_ctx *device_ctx;

    device_ctx = &i2c_ctx.device_ctx_table[fwk_id_get_element_idx(device_id)];

    fwk_assert((length != 0) && (data != NULL));

    if (device_ctx->irq_data_ctx.busy)
        return FWK_E_BUSY;

    device_ctx->irq_data_ctx.state = MOD_N1SDP_I2C_STATE_TX;
    device_ctx->irq_data_ctx.data = data;
    device_ctx->irq_data_ctx.size = length;
    device_ctx->irq_data_ctx.index = 0;
    device_ctx->irq_data_ctx.busy = true;

    /* Clear FIFO */
    I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_CLRFIFO_MASK, I2C_CR_CLRFIFO_SHIFT,
                    I2C_CLRFIFO_ON);

    /* Set address in AR register */
    I2C_REG_W(device_ctx->reg->AR, I2C_AR_ADD7_MASK, I2C_AR_ADD7_SHIFT,
              device_ctx->config->slave_addr);

    /* Clear any pending IRQs and re-enable interrupts */
    device_ctx->reg->ISR = I2C_ISR_MASK;
    device_ctx->reg->IER = (I2C_IER_COMP_MASK | I2C_IER_DATA_MASK |
                            I2C_IER_NACK_MASK | I2C_IER_RXOVF_MASK |
                            I2C_IER_TXOVF_MASK | I2C_IER_RXUNF_MASK);

    /* ISR will take over from here */
    return FWK_SUCCESS;
}

static int i2c_slave_read_irq(fwk_id_t device_id,
    uint8_t *data, uint8_t length)
{
    struct n1sdp_i2c_dev_ctx *device_ctx;

    device_ctx = &i2c_ctx.device_ctx_table[fwk_id_get_element_idx(device_id)];

    fwk_assert((length != 0) && (data != NULL));

    if (device_ctx->irq_data_ctx.busy)
        return FWK_E_BUSY;

    device_ctx->irq_data_ctx.state = MOD_N1SDP_I2C_STATE_RX;
    device_ctx->irq_data_ctx.data = data;
    device_ctx->irq_data_ctx.size = length;
    device_ctx->irq_data_ctx.index = 0;
    device_ctx->irq_data_ctx.busy = true;

    /* Clear FIFO */
    I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_CLRFIFO_MASK, I2C_CR_CLRFIFO_SHIFT,
                    I2C_CLRFIFO_ON);

    /* Set address in AR register */
    I2C_REG_W(device_ctx->reg->AR, I2C_AR_ADD7_MASK, I2C_AR_ADD7_SHIFT,
              device_ctx->config->slave_addr);

    if (length < (I2C_TSR_TANSFER_SIZE - 2))
        device_ctx->reg->TSR = length;

    /* Clear any pending IRQs and re-enable interrupts */
    device_ctx->reg->ISR = I2C_ISR_MASK;
    device_ctx->reg->IER = (I2C_IER_COMP_MASK | I2C_IER_DATA_MASK |
                            I2C_IER_NACK_MASK | I2C_IER_RXOVF_MASK |
                            I2C_IER_TXOVF_MASK | I2C_IER_RXUNF_MASK);

    /* ISR will take over from here */
    return FWK_SUCCESS;
}

static const struct mod_n1sdp_i2c_slave_api_irq i2c_slave_api_irq = {
    .read = i2c_slave_read_irq,
    .write = i2c_slave_write_irq,
};

static void i2c_init(struct n1sdp_i2c_dev_ctx *device_ctx,
                     enum mod_n1sdp_i2c_speed hz)
{
    uint32_t aclk = (uint32_t)device_ctx->config->clock_rate_hz;
    uint32_t divider;

    fwk_assert(hz != 0);

    divider = (aclk / (I2C_HZ * hz)) - I2C_DIV;

    if (divider > I2C_DIV_MAX)
        divider = (divider / I2C_DIV_VAL) | (I2C_DIV_VAL1 << I2C_DIV_SHIFT);

    I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_DIV_MASK, I2C_CR_DIV_SHIFT,
                divider);
}

/*
 * Framework handlers
 */
static int n1sdp_i2c_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    if (element_count == 0)
        return FWK_E_DATA;

    i2c_ctx.device_ctx_table = fwk_mm_calloc(element_count,
        sizeof(i2c_ctx.device_ctx_table[0]));

    return FWK_SUCCESS;
}

static int n1sdp_i2c_element_init(fwk_id_t element_id, unsigned int unused,
    const void *data)
{
    struct mod_n1sdp_i2c_device_config *config;
    struct n1sdp_i2c_dev_ctx *device_ctx;

    if (data == NULL)
        return FWK_E_PARAM;

    config = (struct mod_n1sdp_i2c_device_config *)data;

    if ((config->addr_size != MOD_N1SDP_I2C_ADDRESS_7_BIT) ||
        (config->ack_en != MOD_N1SDP_I2C_ACK_ENABLE))
        return FWK_E_SUPPORT;

    device_ctx = &i2c_ctx.device_ctx_table[fwk_id_get_element_idx(element_id)];
    if (device_ctx == NULL)
        return FWK_E_DATA;

    device_ctx->config = config;
    device_ctx->reg = (struct i2c_reg *)config->reg_base;

    if (config->c2c_mode) {
        if (n1sdp_get_chipid() == 0x0) {
            I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_MS_MASK, I2C_CR_MS_SHIFT,
                        MOD_N1SDP_I2C_MASTER_MODE);
        } else {
            I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_MS_MASK, I2C_CR_MS_SHIFT,
                        MOD_N1SDP_I2C_SLAVE_MODE);
            I2C_REG_W(device_ctx->reg->AR, I2C_AR_ADD7_MASK, I2C_AR_ADD7_SHIFT,
                      config->slave_addr);
        }
    } else {
        I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_MS_MASK, I2C_CR_MS_SHIFT,
                    config->mode);
    }

    I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_NEA_MASK, I2C_CR_NEA_SHIFT,
                config->addr_size);

    I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_ACKEN_MASK, I2C_CR_ACKEN_SHIFT,
                config->ack_en);

    I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_HOLD_MASK, I2C_CR_HOLD_SHIFT,
                config->hold_mode);

    device_ctx->perform_repeat_start = false;

    i2c_init(device_ctx, config->bus_speed_hz);

    return FWK_SUCCESS;
}

static int n1sdp_i2c_process_bind_request(fwk_id_t requester_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_N1SDP_I2C_API_MASTER_POLLED:
        *api = &i2c_master_api_polled;
        break;
    case MOD_N1SDP_I2C_API_SLAVE_IRQ:
        *api = &i2c_slave_api_irq;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int n1sdp_i2c_start(fwk_id_t id)
{
    int status;
    struct n1sdp_i2c_dev_ctx *device_ctx;
    unsigned int i2c_irq;
    unsigned int i2c_element_id;

    /* Nothing to do for module */
    if (!fwk_module_is_valid_element_id(id))
        return FWK_SUCCESS;

    i2c_element_id = fwk_id_get_element_idx(id);
    device_ctx = &i2c_ctx.device_ctx_table[i2c_element_id];
    if (device_ctx == NULL)
        return FWK_E_DATA;

    device_ctx->i2c_id = i2c_element_id;

    if (device_ctx->config->c2c_mode && (n1sdp_get_chipid() != 0x0)) {
        i2c_irq = device_ctx->config->irq;
        status = fwk_interrupt_set_isr_param(i2c_irq, i2c_isr,
                                             (uintptr_t)device_ctx);
        if (status != FWK_SUCCESS)
            return FWK_E_DEVICE;

        status = fwk_interrupt_clear_pending(i2c_irq);
        if (status != FWK_SUCCESS)
            return FWK_E_DEVICE;

        status = fwk_interrupt_enable(i2c_irq);
        if (status != FWK_SUCCESS)
            return FWK_E_DEVICE;

        device_ctx->i2c_callback = i2c_callback_fn;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_n1sdp_i2c = {
    .name = "N1SDP_I2C",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_N1SDP_I2C_API_COUNT,
    .notification_count = MOD_N1SDP_I2C_NOTIFICATION_COUNT,
    .init = n1sdp_i2c_init,
    .element_init = n1sdp_i2c_element_init,
    .process_bind_request = n1sdp_i2c_process_bind_request,
    .start = n1sdp_i2c_start,
};

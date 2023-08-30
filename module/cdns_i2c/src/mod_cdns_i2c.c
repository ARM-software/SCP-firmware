/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Cadence I2C Driver
 */

#include <internal/cdns_i2c.h>

#include <mod_cdns_i2c.h>

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

/* I2C bus active timeout */
#define I2C_BA_TIMEOUT 100U

/* I2C divider calculation values */
#define I2C_HZ        22
#define I2C_DIV       1
#define I2C_DIV_MAX   0x3F
#define I2C_DIV_VAL   4
#define I2C_DIV_VAL1  3
#define I2C_DIV_SHIFT 6

/* Read-modify-write a register field and leave other bits intact. */
#define I2C_REG_RMW(reg, mask, shift, val) \
    ((reg) = (((reg) & ~(mask)) | (((val) << (shift)) & (mask))))

/* Write a single register field. */
#define I2C_REG_W(reg, mask, shift, val) ((reg) = ((val) << (shift)) & (mask))

/* Read a register field. */
#define I2C_REG_R(reg, mask, shift) (((reg) & (mask)) >> (shift))

/* ISR error mask */
#define I2C_ISR_ERROR_MASK \
    (I2C_ISR_RXUNF_MASK | I2C_ISR_TXOVF_MASK | I2C_ISR_RXOVF_MASK | \
     I2C_ISR_NACK_MASK | I2C_ISR_ARBLOST_MASK)

/* Device context */
struct cdns_i2c_dev_ctx {
    /* Pointer to the device configuration */
    const struct mod_cdns_i2c_device_config *config;

    /* Pointer to associated I2C registers */
    struct cdns_i2c_reg *reg;

    /* Track repeated transfer of data */
    bool perform_repeat_start;

    /* IRQ mode data context */
    struct mod_cdns_i2c_irq_data_ctx irq_data_ctx;

    /* I2C ID */
    unsigned int i2c_id;

    /* I2C callback pointer */
    void (*i2c_callback)(
        struct cdns_i2c_dev_ctx *ctx,
        enum mod_cdns_i2c_notifications cb_type);
};

/* Module context */
struct cdns_i2c_ctx {
    /* Table of device contexts */
    struct cdns_i2c_dev_ctx *device_ctx_table;
};

static struct cdns_i2c_ctx i2c_ctx;

/* Callback function */
static void i2c_callback_fn(
    struct cdns_i2c_dev_ctx *ctx,
    enum mod_cdns_i2c_notifications cb_type)
{
    unsigned int i;
    struct fwk_event transfer_event;

    fwk_assert(ctx != NULL);

    if (!fwk_id_is_equal(ctx->config->callback_mod_id, FWK_ID_NONE)) {
        memset((void *)&transfer_event, 0, sizeof(struct fwk_event));
        switch (cb_type) {
        case MOD_CDNS_I2C_NOTIFICATION_IDX_TX:
            transfer_event.id = mod_cdns_i2c_notification_id_target_tx;
            break;
        case MOD_CDNS_I2C_NOTIFICATION_IDX_RX:
            transfer_event.id = mod_cdns_i2c_notification_id_target_rx;
            break;
        case MOD_CDNS_I2C_NOTIFICATION_IDX_ERROR:
            transfer_event.id = mod_cdns_i2c_notification_id_target_error;
            break;
        default:
            fwk_unexpected();
            return;
        }
        transfer_event.response_requested = false;
        transfer_event.source_id =
            FWK_ID_ELEMENT(FWK_MODULE_IDX_CDNS_I2C, ctx->i2c_id);
        transfer_event.target_id = ctx->config->callback_mod_id;
        fwk_notification_notify(&transfer_event, &i);
    }
}

static void clear_isr(struct cdns_i2c_dev_ctx *device_ctx)
{
    uint16_t reg;

    reg = I2C_REG_R(device_ctx->reg->ISR, I2C_ISR_MASK, I2C_ISR_SHIFT);
    I2C_REG_W(device_ctx->reg->ISR, I2C_ISR_MASK, I2C_ISR_SHIFT, reg);
}

static void i2c_isr(uintptr_t data)
{
    struct cdns_i2c_dev_ctx *ctx;
    uint16_t isr = 0;
    uint32_t i = 0;

    ctx = (struct cdns_i2c_dev_ctx *)data;

    /* Save ISR and clear the register */
    isr = ctx->reg->ISR;
    ctx->reg->ISR = isr;

    if (isr == 0) {
        return;
    }

    if (isr & I2C_ISR_ERROR_MASK) {
        goto i2c_error;
    }

    switch (ctx->irq_data_ctx.state) {
    case MOD_CDNS_I2C_STATE_TX:
        /* Check for data interrupt */
        if (isr & I2C_ISR_DATA_MASK) {
            for (i = 0; (i < (ctx->config->max_xfr_size - 2U)) &&
                 (ctx->irq_data_ctx.index < ctx->irq_data_ctx.size);
                 i++) {
                ctx->reg->DR = ctx->irq_data_ctx.data[ctx->irq_data_ctx.index];
                ctx->irq_data_ctx.index++;
            }
        }
        break;

    case MOD_CDNS_I2C_STATE_RX:
        /* Read new data from FIFO. */
        for (; (ctx->reg->SR & I2C_SR_RXDV_MASK) &&
             (ctx->irq_data_ctx.index < ctx->irq_data_ctx.size);
             ctx->irq_data_ctx.index++) {
            ctx->irq_data_ctx.data[ctx->irq_data_ctx.index] =
                (uint8_t)ctx->reg->DR;
        }

        /* Make sure transaction sizes align. */
        if (ctx->reg->SR & I2C_SR_RXDV_MASK) {
            goto i2c_error;
        }

        /* If controller, reload TSR if more data expected. */
        if ((ctx->config->mode == MOD_CDNS_I2C_CONTROLLER_MODE) &&
            (ctx->irq_data_ctx.index < ctx->irq_data_ctx.size)) {
            if ((ctx->irq_data_ctx.size - ctx->irq_data_ctx.index) >
                ctx->config->max_xfr_size) {
                ctx->reg->TSR = ctx->config->max_xfr_size;
            } else {
                ctx->reg->TSR =
                    (ctx->irq_data_ctx.size - ctx->irq_data_ctx.index);
            }
        }
        break;

    default:
        goto i2c_error;
    }

    /* Check for completion interrupt */
    if (isr & I2C_ISR_COMP_MASK) {
        /* Make sure the correct amount of data was transferred */
        if (ctx->irq_data_ctx.index != ctx->irq_data_ctx.size) {
            goto i2c_error;
        }

        ctx->irq_data_ctx.busy = false;
        I2C_REG_W(ctx->reg->AR, I2C_AR_ADD7_MASK, I2C_AR_ADD7_SHIFT, 0);

        /* Run completion callback */
        if ((ctx->irq_data_ctx.state == MOD_CDNS_I2C_STATE_TX) &&
            (ctx->i2c_callback)) {
            ctx->i2c_callback(ctx, MOD_CDNS_I2C_NOTIFICATION_IDX_TX);
        } else if (
            (ctx->irq_data_ctx.state == MOD_CDNS_I2C_STATE_RX) &&
            (ctx->i2c_callback)) {
            ctx->i2c_callback(ctx, MOD_CDNS_I2C_NOTIFICATION_IDX_RX);
        }

        /* Reset I2C driver state */
        ctx->reg->IDR = 0xFFFF;
        I2C_REG_RMW(
            ctx->reg->CR,
            I2C_CR_CLRFIFO_MASK,
            I2C_CR_CLRFIFO_SHIFT,
            I2C_CLRFIFO_ON);
        ctx->irq_data_ctx.data = NULL;
    }
    return;

i2c_error:
    if (ctx->i2c_callback) {
        ctx->i2c_callback(ctx, MOD_CDNS_I2C_NOTIFICATION_IDX_ERROR);
    }

    ctx->reg->IDR = 0xFFFF;
    I2C_REG_RMW(
        ctx->reg->CR,
        I2C_CR_CLRFIFO_MASK,
        I2C_CR_CLRFIFO_SHIFT,
        I2C_CLRFIFO_ON);
    ctx->irq_data_ctx.data = NULL;
}

/*
 * Module I2C driver APIs
 */

/*
 * I2C controller polled mode driver API
 */

static int i2c_controller_read_polled(
    fwk_id_t device_id,
    uint16_t address,
    char *data,
    uint16_t length)
{
    struct cdns_i2c_dev_ctx *device_ctx;
    uint16_t i;
    uint8_t fifo_depth;
    uint16_t tsr_reload_size = 0;
    bool timeout = false;
    bool complete = false;
    bool xfr_error = false;
    volatile uint16_t isr;

    fwk_assert(length != 0);

    device_ctx = &i2c_ctx.device_ctx_table[fwk_id_get_element_idx(device_id)];

    /* Wait for bus to be inactive */
    for (i = I2C_BA_TIMEOUT; (i > 0) &&
         (I2C_REG_R(device_ctx->reg->SR, I2C_SR_BA_MASK, I2C_SR_BA_SHIFT) != 0);
         i--) {
        continue;
    }

    if (i == 0) {
        return FWK_E_DEVICE;
    }

    I2C_REG_RMW(
        device_ctx->reg->CR, I2C_CR_HOLD_MASK, I2C_CR_HOLD_SHIFT, I2C_HOLD_ON);

    fifo_depth = device_ctx->config->fifo_depth;

    /* Make sure any previous error state is cleared */
    I2C_REG_RMW(
        device_ctx->reg->CR,
        I2C_CR_CLRFIFO_MASK,
        I2C_CR_CLRFIFO_SHIFT,
        I2C_CLRFIFO_ON);

    /* Set the direction */
    I2C_REG_RMW(
        device_ctx->reg->CR, I2C_CR_RW_MASK, I2C_CR_RW_SHIFT, I2C_RW_READ);

    if (!device_ctx->perform_repeat_start) {
        I2C_REG_RMW(
            device_ctx->reg->CR,
            I2C_CR_CLRFIFO_MASK,
            I2C_CR_CLRFIFO_SHIFT,
            I2C_CLRFIFO_ON);
    }

    clear_isr(device_ctx);

    /*
     * The I2C controller sends a NACK to the target when transfer size register
     * reaches zero, irrespective of the hold bit.
     *
     * If receiving more than the configured TSR size, configure TSR to be 1
     * more than the FIFO depth as that stops it from dropping down to 0 when
     * the FIFO is full. That way we can reliably do the bookkeeping of bytes
     * received/to receive. Chunking the transfer length in the units of FIFO
     * depth simplifies the handling as we don't have to care for DATA bit
     * interrrupt handling.
     */
    tsr_reload_size = (length > fifo_depth) ? (fifo_depth + 1) : length;
    I2C_REG_RMW(
        device_ctx->reg->TSR,
        I2C_TSR_SIZE_MASK,
        I2C_TSR_SIZE_SHIFT,
        tsr_reload_size);

    I2C_REG_RMW(
        device_ctx->reg->AR, I2C_AR_ADD7_MASK, I2C_AR_ADD7_SHIFT, address);

    while (!complete && !timeout && !xfr_error) {
        isr = I2C_REG_R(device_ctx->reg->ISR, I2C_ISR_MASK, I2C_ISR_SHIFT);
        /*
         * If receiving more than the configured TSR size, don't let the TSR
         * drop to 0 as it would terminate the transfer.
         */
        if ((length > fifo_depth) &&
            (I2C_REG_R(
                 device_ctx->reg->TSR, I2C_TSR_SIZE_MASK, I2C_TSR_SIZE_SHIFT) ==
             1)) {
            length -= fifo_depth;

            /* Set the expected number of bytes to be received */
            tsr_reload_size = (length > fifo_depth) ? (fifo_depth + 1) : length;
            I2C_REG_RMW(
                device_ctx->reg->TSR,
                I2C_TSR_SIZE_MASK,
                I2C_TSR_SIZE_SHIFT,
                tsr_reload_size);

            for (i = 0; i < fifo_depth; i++) {
                if (I2C_REG_R(
                        device_ctx->reg->SR,
                        I2C_SR_RXDV_MASK,
                        I2C_SR_RXDV_SHIFT)) {
                    *data++ = I2C_REG_R(
                        device_ctx->reg->DR,
                        I2C_DR_DATA_MASK,
                        I2C_DR_DATA_SHIFT);
                }
            }
        }

        /* Check if we are done with the transfer */
        if ((isr & I2C_ISR_COMP_MASK) &&
            I2C_REG_R(
                device_ctx->reg->TSR, I2C_TSR_SIZE_MASK, I2C_TSR_SIZE_SHIFT) ==
                0) {
            /*
             *  Bytes remaining in the FIFO are the final bytes left to be read
             *  off the FIFO.
             */
            for (i = 0; i < fifo_depth; i++) {
                if (I2C_REG_R(
                        device_ctx->reg->SR,
                        I2C_SR_RXDV_MASK,
                        I2C_SR_RXDV_SHIFT)) {
                    *data++ = I2C_REG_R(
                        device_ctx->reg->DR,
                        I2C_DR_DATA_MASK,
                        I2C_DR_DATA_SHIFT);
                }
            }

            /* Clear the interrupt status bit */
            I2C_REG_W(
                device_ctx->reg->ISR, I2C_ISR_COMP_MASK, I2C_ISR_COMP_SHIFT, 1);

            complete = true;
        }

        /* Check for any I2C interface errors */
        if (isr & I2C_ISR_ERROR_MASK) {
            xfr_error = true;
        }

        /* Check for transfer timeout */
        if (I2C_REG_R(
                device_ctx->reg->ISR, I2C_ISR_TO_MASK, I2C_ISR_TO_SHIFT)) {
            timeout = true;
        }
    }

    if (xfr_error == true) {
        I2C_REG_RMW(
            device_ctx->reg->CR,
            I2C_CR_HOLD_MASK,
            I2C_CR_HOLD_SHIFT,
            I2C_HOLD_OFF);
        clear_isr(device_ctx);
        return FWK_E_DEVICE;
    }

    if (timeout == true) {
        I2C_REG_RMW(
            device_ctx->reg->CR,
            I2C_CR_HOLD_MASK,
            I2C_CR_HOLD_SHIFT,
            I2C_HOLD_OFF);
        clear_isr(device_ctx);
        return FWK_E_TIMEOUT;
    }

    /* Clear the hold bit to signify the end of the read sequence */
    I2C_REG_RMW(
        device_ctx->reg->CR, I2C_CR_HOLD_MASK, I2C_CR_HOLD_SHIFT, I2C_HOLD_OFF);

    clear_isr(device_ctx);

    device_ctx->perform_repeat_start = false;

    return FWK_SUCCESS;
}

static int i2c_controller_write_polled(
    fwk_id_t device_id,
    uint16_t address,
    const char *data,
    uint16_t length,
    bool stop)
{
    struct cdns_i2c_dev_ctx *device_ctx;
    volatile uint16_t isr;
    unsigned int i;
    uint8_t fifo_depth;
    uint16_t tsr_reload_size = 0;
    bool complete = false;
    bool timeout = false;
    bool xfr_error = false;

    fwk_assert(length != 0);

    device_ctx = &i2c_ctx.device_ctx_table[fwk_id_get_element_idx(device_id)];

    fifo_depth = device_ctx->config->fifo_depth;

    /* Wait for bus to be inactive */
    for (i = I2C_BA_TIMEOUT; (i > 0) &&
         (I2C_REG_R(device_ctx->reg->SR, I2C_SR_BA_MASK, I2C_SR_BA_SHIFT) != 0);
         i--) {
        continue;
    }

    if (i == 0) {
        return FWK_E_DEVICE;
    }

    I2C_REG_RMW(
        device_ctx->reg->CR, I2C_CR_HOLD_MASK, I2C_CR_HOLD_SHIFT, I2C_HOLD_ON);

    /* Make sure any previous error state is cleared */
    I2C_REG_RMW(
        device_ctx->reg->CR,
        I2C_CR_CLRFIFO_MASK,
        I2C_CR_CLRFIFO_SHIFT,
        I2C_CLRFIFO_ON);

    /* Set the direction */
    I2C_REG_RMW(
        device_ctx->reg->CR, I2C_CR_RW_MASK, I2C_CR_RW_SHIFT, I2C_RW_WRITE);

    clear_isr(device_ctx);

    /* Set the expected number of bytes to be transmitted */
    if (length > fifo_depth) {
        I2C_REG_RMW(
            device_ctx->reg->TSR,
            I2C_TSR_SIZE_MASK,
            I2C_TSR_SIZE_SHIFT,
            fifo_depth);
    } else {
        I2C_REG_RMW(
            device_ctx->reg->TSR,
            I2C_TSR_SIZE_MASK,
            I2C_TSR_SIZE_SHIFT,
            length);
    }

    if (!device_ctx->perform_repeat_start) {
        /*
         * Preload the FIFO with a single data byte. If this write is to
         * generate a repeat start then don't write any data until the
         * address has been set.
         */
        I2C_REG_RMW(
            device_ctx->reg->DR, I2C_DR_DATA_MASK, I2C_DR_DATA_SHIFT, *data++);
        length--;
    }

    /* Write the address, triggering the start of the transfer */
    I2C_REG_RMW(
        device_ctx->reg->AR, I2C_AR_ADD7_MASK, I2C_AR_ADD7_SHIFT, address);

    for (i = 0; (i < (fifo_depth - 1U)) && length; i++) {
        I2C_REG_RMW(
            device_ctx->reg->DR, I2C_DR_DATA_MASK, I2C_DR_DATA_SHIFT, *data++);
        length--;
    }

    /* Wait for transfer completion */
    while (!complete && !timeout && !xfr_error) {
        isr = I2C_REG_R(device_ctx->reg->ISR, I2C_ISR_MASK, I2C_ISR_SHIFT);
        /*
         * Check if FIFO is nearing empty and there is more data to be
         * transferred
         */
        if (isr & I2C_ISR_DATA_MASK) {
            /*
             * The FIFO may still contain 2 outstanding bytes that it needs to
             * flush out. Reload the FIFO sparing two bytes to avoid the risk of
             * overrunning the FIFO with the remaining data. Number of bytes
             * reloaded in the FIFO will be the new TSR value once it reaches 0.
             *
             */
            for (i = 0; (i < (fifo_depth - 2U)) && length; i++) {
                I2C_REG_RMW(
                    device_ctx->reg->DR,
                    I2C_DR_DATA_MASK,
                    I2C_DR_DATA_SHIFT,
                    *data++);
                length--;
            }

            tsr_reload_size = i;

            /* Clear DATA bit so we don't overflow the FIFO */
            I2C_REG_W(
                device_ctx->reg->ISR, I2C_ISR_DATA_MASK, I2C_ISR_DATA_SHIFT, 1);
        }

        /* Check for transfer complete */

        if (isr & I2C_ISR_COMP_MASK) {
            /*
             * TSR reload size is calculated based on how many bytes were
             * actually written to the FIFO. If the TSR reload size is 0, it
             * either means that the the data length to be transferred was less
             * than the I2C_TSR_SIZE_CONFIGURED, or that all the outstanding
             * data has been transferred.
             */
            if (tsr_reload_size) {
                I2C_REG_RMW(
                    device_ctx->reg->TSR,
                    I2C_TSR_SIZE_MASK,
                    I2C_TSR_SIZE_SHIFT,
                    tsr_reload_size);
            } else {
                complete = true;
            }

            /* Clear the interrupt status bit */
            I2C_REG_W(
                device_ctx->reg->ISR, I2C_ISR_COMP_MASK, I2C_ISR_COMP_SHIFT, 1);
        }

        /* Check for any I2C interface errors */
        if (isr & I2C_ISR_ERROR_MASK) {
            xfr_error = true;
        }

        /* Check for transfer timeout */
        if (isr & I2C_ISR_TO_MASK) {
            timeout = true;
        }
    }

    if (xfr_error == true) {
        I2C_REG_RMW(
            device_ctx->reg->CR,
            I2C_CR_HOLD_MASK,
            I2C_CR_HOLD_SHIFT,
            I2C_HOLD_OFF);
        clear_isr(device_ctx);
        return FWK_E_DEVICE;
    }

    if (timeout == true) {
        I2C_REG_RMW(
            device_ctx->reg->CR,
            I2C_CR_HOLD_MASK,
            I2C_CR_HOLD_SHIFT,
            I2C_HOLD_OFF);
        clear_isr(device_ctx);
        return FWK_E_TIMEOUT;
    }

    clear_isr(device_ctx);

    if (stop) {
        /* Clear the hold bit to signify the end of the write sequence */
        I2C_REG_RMW(
            device_ctx->reg->CR,
            I2C_CR_HOLD_MASK,
            I2C_CR_HOLD_SHIFT,
            I2C_HOLD_OFF);
        clear_isr(device_ctx);
        device_ctx->perform_repeat_start = false;
    } else {
        /* Track repeated start so that the following read or write is aware */
        device_ctx->perform_repeat_start = true;
    }
    return FWK_SUCCESS;
}

static const struct mod_cdns_i2c_controller_api_polled
    i2c_controller_api_polled = {
        .read = i2c_controller_read_polled,
        .write = i2c_controller_write_polled,
    };

/*
 * I2C target interrupt mode driver API
 */
static int i2c_target_write_irq(
    fwk_id_t device_id,
    uint8_t *data,
    uint8_t length)
{
    struct cdns_i2c_dev_ctx *device_ctx;

    device_ctx = &i2c_ctx.device_ctx_table[fwk_id_get_element_idx(device_id)];

    fwk_assert((length != 0) && (data != NULL));

    if (device_ctx->irq_data_ctx.busy) {
        return FWK_E_BUSY;
    }

    device_ctx->irq_data_ctx.state = MOD_CDNS_I2C_STATE_TX;
    device_ctx->irq_data_ctx.data = data;
    device_ctx->irq_data_ctx.size = length;
    device_ctx->irq_data_ctx.index = 0;
    device_ctx->irq_data_ctx.busy = true;

    /* Clear FIFO */
    I2C_REG_RMW(
        device_ctx->reg->CR,
        I2C_CR_CLRFIFO_MASK,
        I2C_CR_CLRFIFO_SHIFT,
        I2C_CLRFIFO_ON);

    /* Set address in AR register */
    I2C_REG_W(
        device_ctx->reg->AR,
        I2C_AR_ADD7_MASK,
        I2C_AR_ADD7_SHIFT,
        device_ctx->config->target_addr);

    /* Clear any pending IRQs and re-enable interrupts */
    device_ctx->reg->ISR = I2C_ISR_MASK;
    device_ctx->reg->IER =
        (I2C_IER_COMP_MASK | I2C_IER_DATA_MASK | I2C_IER_NACK_MASK |
         I2C_IER_RXOVF_MASK | I2C_IER_TXOVF_MASK | I2C_IER_RXUNF_MASK);

    /* ISR will take over from here */
    return FWK_SUCCESS;
}

static int i2c_target_read_irq(
    fwk_id_t device_id,
    uint8_t *data,
    uint8_t length)
{
    struct cdns_i2c_dev_ctx *device_ctx;

    device_ctx = &i2c_ctx.device_ctx_table[fwk_id_get_element_idx(device_id)];

    fwk_assert((length != 0) && (data != NULL));

    if (device_ctx->irq_data_ctx.busy) {
        return FWK_E_BUSY;
    }

    device_ctx->irq_data_ctx.state = MOD_CDNS_I2C_STATE_RX;
    device_ctx->irq_data_ctx.data = data;
    device_ctx->irq_data_ctx.size = length;
    device_ctx->irq_data_ctx.index = 0;
    device_ctx->irq_data_ctx.busy = true;

    /* Clear FIFO */
    I2C_REG_RMW(
        device_ctx->reg->CR,
        I2C_CR_CLRFIFO_MASK,
        I2C_CR_CLRFIFO_SHIFT,
        I2C_CLRFIFO_ON);

    /* Set address in AR register */
    I2C_REG_W(
        device_ctx->reg->AR,
        I2C_AR_ADD7_MASK,
        I2C_AR_ADD7_SHIFT,
        device_ctx->config->target_addr);

    if (length < (device_ctx->config->max_xfr_size - 2)) {
        device_ctx->reg->TSR = length;
    }

    /* Clear any pending IRQs and re-enable interrupts */
    device_ctx->reg->ISR = I2C_ISR_MASK;
    device_ctx->reg->IER =
        (I2C_IER_COMP_MASK | I2C_IER_DATA_MASK | I2C_IER_NACK_MASK |
         I2C_IER_RXOVF_MASK | I2C_IER_TXOVF_MASK | I2C_IER_RXUNF_MASK);

    /* ISR will take over from here */
    return FWK_SUCCESS;
}

static const struct mod_cdns_i2c_target_api_irq i2c_target_api_irq = {
    .read = i2c_target_read_irq,
    .write = i2c_target_write_irq,
};

static void i2c_init(
    struct cdns_i2c_dev_ctx *device_ctx,
    enum mod_cdns_i2c_speed hz)
{
    uint32_t aclk = (uint32_t)device_ctx->config->clock_rate_hz;
    uint32_t divider;

    fwk_assert(hz != 0);

    divider = (aclk / (I2C_HZ * hz)) - I2C_DIV;

    if (divider > I2C_DIV_MAX) {
        divider = (divider / I2C_DIV_VAL) | (I2C_DIV_VAL1 << I2C_DIV_SHIFT);
    }

    I2C_REG_RMW(
        device_ctx->reg->CR, I2C_CR_DIV_MASK, I2C_CR_DIV_SHIFT, divider);
}

/*
 * Framework handlers
 */
static int cdns_i2c_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (element_count == 0) {
        return FWK_E_DATA;
    }

    i2c_ctx.device_ctx_table =
        fwk_mm_calloc(element_count, sizeof(i2c_ctx.device_ctx_table[0]));

    return FWK_SUCCESS;
}

static int cdns_i2c_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    struct mod_cdns_i2c_device_config *config;
    struct cdns_i2c_dev_ctx *device_ctx;

    if (data == NULL) {
        return FWK_E_PARAM;
    }

    config = (struct mod_cdns_i2c_device_config *)data;

    if ((config->addr_size != MOD_CDNS_I2C_ADDRESS_7_BIT) ||
        (config->ack_en != MOD_CDNS_I2C_ACK_ENABLE)) {
        return FWK_E_SUPPORT;
    }

    device_ctx = &i2c_ctx.device_ctx_table[fwk_id_get_element_idx(element_id)];
    if (device_ctx == NULL) {
        return FWK_E_DATA;
    }

    device_ctx->config = config;
    device_ctx->reg = (struct cdns_i2c_reg *)config->reg_base;

    I2C_REG_RMW(
        device_ctx->reg->CR, I2C_CR_MS_MASK, I2C_CR_MS_SHIFT, config->mode);

    if (config->mode == MOD_CDNS_I2C_TARGET_MODE) {
        I2C_REG_W(
            device_ctx->reg->AR,
            I2C_AR_ADD7_MASK,
            I2C_AR_ADD7_SHIFT,
            config->target_addr);
    }

    I2C_REG_RMW(
        device_ctx->reg->CR,
        I2C_CR_NEA_MASK,
        I2C_CR_NEA_SHIFT,
        config->addr_size);

    I2C_REG_RMW(
        device_ctx->reg->CR,
        I2C_CR_ACKEN_MASK,
        I2C_CR_ACKEN_SHIFT,
        config->ack_en);

    I2C_REG_RMW(
        device_ctx->reg->CR,
        I2C_CR_HOLD_MASK,
        I2C_CR_HOLD_SHIFT,
        config->hold_mode);

    I2C_REG_RMW(
        device_ctx->reg->TOR,
        I2C_TOR_TIMEOUT_MASK,
        I2C_TOR_TIMEOUT_SHIFT,
        I2C_TOR_TIMEOUT_VALUE);

    device_ctx->perform_repeat_start = false;

    i2c_init(device_ctx, config->bus_speed_hz);

    return FWK_SUCCESS;
}

static int cdns_i2c_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_CDNS_I2C_API_CONTROLLER_POLLED:
        *api = &i2c_controller_api_polled;
        break;
    case MOD_CDNS_I2C_API_TARGET_IRQ:
        *api = &i2c_target_api_irq;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int cdns_i2c_start(fwk_id_t id)
{
    int status;
    struct cdns_i2c_dev_ctx *device_ctx;
    unsigned int i2c_irq;
    unsigned int i2c_element_id;

    /* Nothing to do for module */
    if (!fwk_module_is_valid_element_id(id)) {
        return FWK_SUCCESS;
    }

    i2c_element_id = fwk_id_get_element_idx(id);
    device_ctx = &i2c_ctx.device_ctx_table[i2c_element_id];
    if (device_ctx == NULL) {
        return FWK_E_DATA;
    }

    device_ctx->i2c_id = i2c_element_id;

    if (device_ctx->config->irq) {
        i2c_irq = device_ctx->config->irq;
        status = fwk_interrupt_set_isr_param(
            i2c_irq, i2c_isr, (uintptr_t)device_ctx);
        if (status != FWK_SUCCESS) {
            return FWK_E_DEVICE;
        }

        status = fwk_interrupt_clear_pending(i2c_irq);
        if (status != FWK_SUCCESS) {
            return FWK_E_DEVICE;
        }

        status = fwk_interrupt_enable(i2c_irq);
        if (status != FWK_SUCCESS) {
            return FWK_E_DEVICE;
        }

        device_ctx->i2c_callback = i2c_callback_fn;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_cdns_i2c = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_CDNS_I2C_API_COUNT,
    .notification_count = MOD_CDNS_I2C_NOTIFICATION_COUNT,
    .init = cdns_i2c_init,
    .element_init = cdns_i2c_element_init,
    .process_bind_request = cdns_i2c_process_bind_request,
    .start = cdns_i2c_start,
};

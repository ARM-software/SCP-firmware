/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP I2C Driver
 */

#include <fwk_assert.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>
#include <internal/n1sdp_i2c.h>
#include <mod_log.h>
#include <mod_n1sdp_i2c.h>
#include <mod_power_domain.h>
#include <stdbool.h>

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

/* Device context */
struct n1sdp_i2c_dev_ctx {
    /* Pointer to the device configuration */
    const struct mod_n1sdp_i2c_device_config *config;

    /* Pointer to associated I2C registers */
    struct i2c_reg *reg;

    /* Track repeated transfer of data */
    bool perform_repeat_start;
};

/* Module context */
struct n1sdp_i2c_ctx {
    /* Table of device contexts */
    struct n1sdp_i2c_dev_ctx *device_ctx_table;

    /* Log API pointer */
    const struct mod_log_api *log_api;
};

static struct n1sdp_i2c_ctx i2c_ctx;

static void clear_isr(struct n1sdp_i2c_dev_ctx *device_ctx)
{
    uint16_t reg;

    reg = I2C_REG_R(device_ctx->reg->ISR, I2C_ISR_MASK, I2C_ISR_SHIFT);
    I2C_REG_W(device_ctx->reg->ISR, I2C_ISR_MASK, I2C_ISR_SHIFT, reg);
}

/*
 * Module I2C driver API
 */
static int i2c_master_read(fwk_id_t device_id, uint16_t address, char *data,
                           uint16_t length)
{
    bool complete = false;
    unsigned int timeout;
    int status;
    struct n1sdp_i2c_dev_ctx *device_ctx;

    status = fwk_module_check_call(device_id);
    if (status != FWK_SUCCESS)
        return status;

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
        i2c_ctx.log_api->log(MOD_LOG_GROUP_INFO, "read: timeout expired\n");
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

static int i2c_master_write(fwk_id_t device_id, uint16_t address,
                            const char *data, uint16_t length, bool stop)
{
    bool complete = false;
    unsigned int timeout;
    int status;
    struct n1sdp_i2c_dev_ctx *device_ctx;
    uint16_t sr, isr;

    status = fwk_module_check_call(device_id);
    if (status != FWK_SUCCESS)
        return status;

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
        i2c_ctx.log_api->log(MOD_LOG_GROUP_INFO, "write: timeout expired\n");
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

static const struct mod_n1sdp_i2c_master_api driver_api = {
    .read = i2c_master_read,
    .write = i2c_master_write,
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
    if (i2c_ctx.device_ctx_table == NULL)
        return FWK_E_NOMEM;

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

    if ((config->mode != MOD_N1SDP_I2C_MASTER_MODE) ||
        (config->addr_size != MOD_N1SDP_I2C_ADDRESS_7_BIT) ||
        (config->ack_en != MOD_N1SDP_I2C_ACK_ENABLE) ||
        (config->hold_mode != MOD_N1SDP_I2C_HOLD_ON))
        return FWK_E_SUPPORT;

    device_ctx = &i2c_ctx.device_ctx_table[fwk_id_get_element_idx(element_id)];
    if (device_ctx == NULL)
        return FWK_E_DATA;

    device_ctx->config = config;
    device_ctx->reg = (struct i2c_reg *)config->reg_base;

    I2C_REG_RMW(device_ctx->reg->CR, I2C_CR_MS_MASK, I2C_CR_MS_SHIFT,
                config->mode);

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

static int n1sdp_i2c_bind(fwk_id_t id, unsigned int round)
{
    if ((round == 0) && (fwk_id_get_type(id) == FWK_ID_TYPE_MODULE)) {
        return fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_LOG),
            MOD_LOG_API_ID, &i2c_ctx.log_api);
    }
    return FWK_SUCCESS;
}

static int n1sdp_i2c_process_bind_request(fwk_id_t requester_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    *api = &driver_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_n1sdp_i2c = {
    .name = "N1SDP_I2C",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = n1sdp_i2c_init,
    .element_init = n1sdp_i2c_element_init,
    .bind = n1sdp_i2c_bind,
    .process_bind_request = n1sdp_i2c_process_bind_request,
};

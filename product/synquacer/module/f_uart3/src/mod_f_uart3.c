/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "f_uart3.h"

#include <mod_f_uart3.h>

#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

#include <stdbool.h>

struct mod_f_uart3_element_ctx {
    /* Whether the device has an open file stream */
    bool open;
};

static struct mod_f_uart3_ctx {
    bool initialized; /* Whether the context has been initialized */

    struct mod_f_uart3_element_ctx *elements; /* Element context table */
} mod_f_uart3_ctx = {
    .initialized = false,
};

static const struct {
    uint8_t DLM;
    uint8_t DLL;
} mod_f_uart3_divider_latch_table[] = {
    [MOD_F_UART3_BAUD_RATE_9600] = {
       .DLM = F_UART3_DLM_9600,
       .DLL = F_UART3_DLL_9600,
    },

    [MOD_F_UART3_BAUD_RATE_19200] = {
       .DLM = F_UART3_DLM_19200,
       .DLL = F_UART3_DLL_19200,
    },

    [MOD_F_UART3_BAUD_RATE_38400] = {
        .DLM = F_UART3_DLM_38400,
        .DLL = F_UART3_DLL_38400,
    },

    [MOD_F_UART3_BAUD_RATE_57600] = {
        .DLM = F_UART3_DLM_57600,
        .DLL = F_UART3_DLL_57600,
    },

    [MOD_F_UART3_BAUD_RATE_115200] = {
        .DLM = F_UART3_DLM_115200,
        .DLL = F_UART3_DLL_115200,
    },

    [MOD_F_UART3_BAUD_RATE_230400] = {
        .DLM = F_UART3_DLM_230400,
        .DLL = F_UART3_DLL_230400,
    },
};

static int mod_f_uart3_init_ctx(struct mod_f_uart3_ctx *ctx)
{
    size_t element_count;

    fwk_assert(!mod_f_uart3_ctx.initialized);

    element_count = fwk_module_get_element_count(fwk_module_id_f_uart3);
    if (element_count == 0)
        return FWK_SUCCESS;

    ctx->elements = fwk_mm_alloc(element_count, sizeof(ctx->elements[0]));
    if (!fwk_expect(ctx->elements != NULL))
        return FWK_E_NOMEM;

    for (size_t i = 0; i < element_count; i++) {
        ctx->elements[i] = (struct mod_f_uart3_element_ctx){
            .open = false, /* Assume the device is always powered */
        };
    }

    mod_f_uart3_ctx.initialized = true;

    return FWK_SUCCESS;
}

static void mod_f_uart3_enable(fwk_id_t id)
{
    const struct mod_f_uart3_element_cfg *cfg = fwk_module_get_data(id);

    struct f_uart3_reg *reg;
    struct f_uart3_dla_reg *dla_reg;

    fwk_assert(cfg != NULL);

    reg = (struct f_uart3_reg *)cfg->reg_base;
    dla_reg = (struct f_uart3_dla_reg *)cfg->dla_reg_base;

    /* Set DLAB */
    reg->LCR |= F_UART3_LCR_DLAB;

    __DSB();

    dla_reg->DLL = mod_f_uart3_divider_latch_table[cfg->baud_rate].DLL;
    dla_reg->DLM = mod_f_uart3_divider_latch_table[cfg->baud_rate].DLM;

    __DSB();

    /* Clear DLAB */
    reg->LCR &= ~F_UART3_LCR_DLAB;

    __DSB();

    reg->LCR = F_UART3_LCR_WLS_8;
    if (cfg->parity_enable_flag) {
        reg->LCR |= F_UART3_LCR_PEN;
        if (cfg->even_parity_flag)
            reg->LCR |= F_UART3_LCR_EPS;
    }

    __DSB();

    /* Disable interrupt */
    reg->IER = 0x0;

    __DSB();

    /* FIFO Reset */
    reg->IIR_FCR = (F_UART3_FCR_RXFRST | F_UART3_FCR_TXFRST);

    __DSB();
}

static void mod_f_uart3_putch(fwk_id_t id, char ch)
{
    const struct mod_f_uart3_element_cfg *cfg = fwk_module_get_data(id);

    struct f_uart3_reg *reg = (void *)cfg->reg_base;

    while ((reg->LSR & F_UART3_LSR_THRE) == 0x0)
        continue;

    reg->RFR_TFR = ch;
}

static void mod_f_uart3_flush(fwk_id_t id)
{
    const struct mod_f_uart3_element_cfg *cfg = fwk_module_get_data(id);

    struct f_uart3_reg *reg = (void *)cfg->reg_base;

    while ((reg->LSR & F_UART3_LSR_TEMT) == 0x0)
        continue;
}

static int mod_f_uart3_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (mod_f_uart3_ctx.initialized)
        return FWK_SUCCESS;

    return mod_f_uart3_init_ctx(&mod_f_uart3_ctx);
}

static int mod_f_uart3_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    return FWK_SUCCESS;
}

static int mod_f_uart3_io_open(const struct fwk_io_stream *stream)
{
    int status;

    struct mod_f_uart3_element_ctx *ctx;

    if (!fwk_id_is_type(stream->id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_SUPPORT;

    if (!mod_f_uart3_ctx.initialized) {
        status = mod_f_uart3_init_ctx(&mod_f_uart3_ctx);
        if (status != FWK_SUCCESS)
            return FWK_E_STATE;
    }

    ctx = &mod_f_uart3_ctx.elements[fwk_id_get_element_idx(stream->id)];
    if (ctx->open) /* Refuse to open the same device twice */
        return FWK_E_BUSY;

    ctx->open = true;

    mod_f_uart3_enable(stream->id); /* Enable the device */

    return FWK_SUCCESS;
}

int mod_f_uart3_io_putch(const struct fwk_io_stream *stream, char ch)
{
    const struct mod_f_uart3_element_ctx *ctx =
        &mod_f_uart3_ctx.elements[fwk_id_get_element_idx(stream->id)];

    fwk_assert(ctx->open);

    mod_f_uart3_putch(stream->id, ch);

    return FWK_SUCCESS;
}

int mod_f_uart3_close(const struct fwk_io_stream *stream)
{
    struct mod_f_uart3_element_ctx *ctx;

    fwk_assert(stream != NULL); /* Validated by the framework */
    fwk_assert(fwk_module_is_valid_element_id(stream->id));

    mod_f_uart3_flush(stream->id);

    ctx = &mod_f_uart3_ctx.elements[fwk_id_get_element_idx(stream->id)];
    fwk_assert(ctx->open);

    ctx->open = false;

    return FWK_SUCCESS;
}

const struct fwk_module module_f_uart3 = {
    .type = FWK_MODULE_TYPE_DRIVER,

    .init = mod_f_uart3_init,
    .element_init = mod_f_uart3_element_init,

    .adapter =
        (struct fwk_io_adapter){
            .open = mod_f_uart3_io_open,
            .putch = mod_f_uart3_io_putch,
            .close = mod_f_uart3_close,
        },
};

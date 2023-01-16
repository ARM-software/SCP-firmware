/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2023, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scif.h"

#include <mmio.h>
#include <system_mmap.h>

#include <mod_rcar_scif.h>
#include <mod_rcar_system.h>

#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <assert.h>

struct mod_rcar_scif_element_ctx {
    /* Whether the device has an open file stream */
    bool open;
};

static struct mod_rcar_scif_ctx {
    bool initialized; /* Whether the context has been initialized */

    struct mod_rcar_scif_element_ctx *elements; /* Element context table */
} mod_rcar_scif_ctx = {
    .initialized = false,
};

static const struct mod_rcar_scif_element_cfg *current_cfg;

static int mod_rcar_scif_init_ctx(struct mod_rcar_scif_ctx *ctx)
{
    size_t element_count;

    fwk_assert(!mod_rcar_scif_ctx.initialized);

    element_count = fwk_module_get_element_count(fwk_module_id_rcar_scif);
    if (element_count == 0)
        return FWK_SUCCESS;

    ctx->elements = fwk_mm_alloc(element_count, sizeof(ctx->elements[0]));
    if (!fwk_expect(ctx->elements != NULL))
        return FWK_E_NOMEM;

    for (size_t i = 0; i < element_count; i++) {
        ctx->elements[i] = (struct mod_rcar_scif_element_ctx){
            .open = false,
        };
    }

    mod_rcar_scif_ctx.initialized = true;

    return FWK_SUCCESS;
}

/*
 * For details on the constants and equations used to calculate the baud rate
 * settings, please consult the SCIF TRM.
 * However, this version does not support it.
 */
static int mod_rcar_scif_set_baud_rate(
    const struct mod_rcar_scif_element_cfg *cfg)
{
    struct scif_reg *reg = (void *)cfg->reg_base;
    uint32_t status;
    int i;

    assert(reg);
    current_cfg = cfg;

    if (reg == (void *)BOARD_UART1_BASE) {
        /* SCSI-1 */
        status = mmio_read_32(CPG_SMSTPCR2);
        status = (status & ~MSTP26);
        mmio_write_32(CPG_CPGWPR, ~status);
        mmio_write_32(CPG_SMSTPCR2, status);

        while (mmio_read_32(CPG_MSTPSR2) & MSTP26)
            continue;
    } else if (reg == (void *)BOARD_UART2_BASE) {
        /* SCSI-2 */
        status = mmio_read_32(CPG_SMSTPCR3);
        status = (status & ~MSTP310);
        mmio_write_32(CPG_CPGWPR, ~status);
        mmio_write_32(CPG_SMSTPCR3, status);

        while (mmio_read_32(CPG_MSTPSR3) & MSTP310)
            continue;
    } else {
        return FWK_E_PARAM;
    }

    /* Clear bits TE and RE in SCSCR to 0 */
    reg->SCSCR = (SCSCR_TE_DIS | SCSCR_RE_DIS);
    /* Set bits TFRST and RFRST in SCFCR to 1 */
    reg->SCFCR |= (SCFCR_TFRST_EN | SCFCR_RFRS_EN);
    /* Read flags of ER, DR, BRK, */
    reg->SCFSR = SCFSR_INIT_DATA;
    reg->SCLSR = 0;
    /* Set bits CKE[1:0] in SCSCR */
    reg->SCSCR = (reg->SCSCR & ~SCSCR_CKE_MASK) | SCSCR_CKE_INT_CLK;
    /* Set data transfer format in SCSMR */
    reg->SCSMR = SCSMR_INIT_DATA;
    /* Set value in SCBRR */
#if SCIF_CLK == SCIF_INTERNAL_CLK
    if ((mmio_read_32(PRR) & (PRR_PRODUCT_MASK | PRR_CUT_MASK)) ==
        PRR_PRODUCT_H3_VER_10) {
        /* H3 Ver.1.0 sets clock to doubling */
        reg->SCBRR = SCBRR_230400BPS;
    } else {
        reg->SCBRR = SCBRR_115200BPS;
    }
#else
    reg->DL = DL_INIT_DATA;
    reg->CKS = CKS_INIT_DATA;
#endif /* SCIF_CLK == SCIF_INTERNAL_CLK */
    for (i = 100; i; i--)
        ;
    reg->SCFCR = SCFCR_INIT_DATA;
    reg->SCSCR = (reg->SCSCR | (SCSCR_TE_EN | SCSCR_RE_EN));
    return FWK_SUCCESS;
}

static int mod_rcar_scif_resume(void)
{
    return mod_rcar_scif_set_baud_rate(current_cfg);
}

static void mod_rcar_scif_putch(fwk_id_t id, char c)
{
    const struct mod_rcar_scif_element_cfg *cfg = fwk_module_get_data(id);
    struct scif_reg *reg = (void *)cfg->reg_base;

    /* Check if the transmit FIFO is full */
    while (GET_SCFDR_T(reg) >= FIFO_FULL)
        continue;

    reg->SCFTDR = c;
}

static bool mod_rcar_scif_getch(fwk_id_t id, char *ch)
{
    return false; /* Not implemented yet */
}

static void mod_rcar_scif_flush(fwk_id_t id)
{
    const struct mod_rcar_scif_element_cfg *cfg = fwk_module_get_data(id);
    struct scif_reg *reg = (void *)cfg->reg_base;

    /* Check if the transmit data is available */
    while (GET_SCFDR_T(reg))
        continue;
}

/*
 * Framework module handlers
 */
static int mod_rcar_scif_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (mod_rcar_scif_ctx.initialized)
        return FWK_SUCCESS;

    return mod_rcar_scif_init_ctx(&mod_rcar_scif_ctx);
}

static int mod_rcar_scif_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    return FWK_SUCCESS;
}

static int mod_rcar_scif_start(fwk_id_t id)
{
    return FWK_SUCCESS;
}

/*
 * Framework io handlers
 */
static int mod_rcar_scif_io_open(const struct fwk_io_stream *stream)
{
    int status;
    struct mod_rcar_scif_element_ctx *ctx;
    const struct mod_rcar_scif_element_cfg *cfg;

    if (!fwk_id_is_type(stream->id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_SUPPORT;

    if (!mod_rcar_scif_ctx.initialized) {
        status = mod_rcar_scif_init_ctx(&mod_rcar_scif_ctx);
        if (status != FWK_SUCCESS)
            return FWK_E_STATE;
    }

    ctx = &mod_rcar_scif_ctx.elements[fwk_id_get_element_idx(stream->id)];
    if (ctx->open) /* Refuse to open the same device twice */
        return FWK_E_BUSY;

    ctx->open = true;

    /* Enable the device if possible */
    cfg = fwk_module_get_data(stream->id);
    status = mod_rcar_scif_set_baud_rate(cfg);

    return status;
}

static int mod_rcar_scif_io_getch(
    const struct fwk_io_stream *restrict stream,
    char *restrict ch)
{
    const struct mod_rcar_scif_element_ctx *ctx =
        &mod_rcar_scif_ctx.elements[fwk_id_get_element_idx(stream->id)];

    bool ok = true;

    fwk_assert(ctx->open);

    ok = mod_rcar_scif_getch(stream->id, ch);
    if (!ok)
        return FWK_PENDING;

    return FWK_SUCCESS;
}

static int mod_rcar_scif_io_putch(const struct fwk_io_stream *stream, char ch)
{
    const struct mod_rcar_scif_element_ctx *ctx =
        &mod_rcar_scif_ctx.elements[fwk_id_get_element_idx(stream->id)];

    fwk_assert(ctx->open);

    mod_rcar_scif_putch(stream->id, ch);

    return FWK_SUCCESS;
}

static int mod_rcar_scif_io_close(const struct fwk_io_stream *stream)
{
    struct mod_rcar_scif_element_ctx *ctx;

    fwk_assert(stream != NULL); /* Validated by the framework */
    fwk_assert(fwk_module_is_valid_element_id(stream->id));

    mod_rcar_scif_flush(stream->id);

    ctx = &mod_rcar_scif_ctx.elements[fwk_id_get_element_idx(stream->id)];
    fwk_assert(ctx->open);

    ctx->open = false;

    return FWK_SUCCESS;
}

static const struct mod_rcar_system_drv_api api_system = {
    .resume = mod_rcar_scif_resume,
};

static int scif_process_bind_request(fwk_id_t requester_id, fwk_id_t target_id,
    fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_RCAR_SCIF_API_TYPE_SYSTEM:
        *api = &api_system;
        break;
    default:
        break;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_rcar_scif = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_RCAR_SCIF_API_COUNT,
    .init = mod_rcar_scif_init,
    .element_init = mod_rcar_scif_element_init,
    .start = mod_rcar_scif_start,
    .adapter =
        (struct fwk_io_adapter){
            .open = mod_rcar_scif_io_open,
            .getch = mod_rcar_scif_io_getch,
            .putch = mod_rcar_scif_io_putch,
            .close = mod_rcar_scif_io_close,
        },
    .process_bind_request = scif_process_bind_request,
};

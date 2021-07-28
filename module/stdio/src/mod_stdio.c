/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_stdio.h>

#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>

struct mod_stdio_element_ctx {
    FILE *stream;
};

static struct {
    bool initialized;

    struct mod_stdio_element_ctx *elements;
} mod_stdio_ctx = {
    .initialized = false,
};

static int mod_stdio_init_ctx(void)
{
    size_t element_count = fwk_module_get_element_count(fwk_module_id_stdio);

    mod_stdio_ctx.elements =
        fwk_mm_calloc(element_count, sizeof(*mod_stdio_ctx.elements));
    if (mod_stdio_ctx.elements == NULL)
        return FWK_E_NOMEM;

    mod_stdio_ctx.initialized = true;

    return FWK_SUCCESS;
}

static int mod_stdio_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *specific_config)
{
    if (!mod_stdio_ctx.initialized)
        return mod_stdio_init_ctx();

    return FWK_SUCCESS;
}

static int mod_stdio_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    return FWK_SUCCESS;
}

static int mod_stdio_open(const struct fwk_io_stream *stream)
{
    int status = FWK_SUCCESS;

    const struct mod_stdio_element_cfg *cfg;
    struct mod_stdio_element_ctx *ctx;

    unsigned int element_idx;

    if (!fwk_id_is_type(stream->id, FWK_ID_TYPE_ELEMENT))
        return FWK_E_SUPPORT;

    if (!mod_stdio_ctx.initialized) {
        status = mod_stdio_init_ctx();
        if (status != FWK_SUCCESS)
            return status;
    }

    element_idx = fwk_id_get_element_idx(stream->id);

    cfg = fwk_module_get_data(stream->id);
    ctx = &mod_stdio_ctx.elements[element_idx];

    if (cfg->type == MOD_STDIO_ELEMENT_TYPE_PATH) {
        ctx->stream = fopen(cfg->file.path, cfg->file.mode);
        if (ctx->stream == NULL)
            status = FWK_E_OS;
    } else {
        ctx->stream = cfg->stream;
    }

    return status;
}

static int mod_stdio_getc(const struct fwk_io_stream *stream, char *ch)
{
    struct mod_stdio_element_ctx *ctx =
        &mod_stdio_ctx.elements[fwk_id_get_element_idx(stream->id)];

    int ich = fgetc(ctx->stream);

    *ch = (char)ich;

    if (ferror(ctx->stream))
        return FWK_E_OS;
    else if (feof(ctx->stream))
        return FWK_PENDING;

    return FWK_SUCCESS;
}

static int mod_stdio_putc(const struct fwk_io_stream *stream, char ch)
{
    struct mod_stdio_element_ctx *ctx =
        &mod_stdio_ctx.elements[fwk_id_get_element_idx(stream->id)];

    fputc(ch, ctx->stream);

    if (ferror(ctx->stream))
        return FWK_E_OS;

    return FWK_SUCCESS;
}

static int mod_stdio_close(const struct fwk_io_stream *stream)
{
    int status = FWK_SUCCESS;

    const struct mod_stdio_element_cfg *cfg;
    struct mod_stdio_element_ctx *ctx;

    unsigned int element_idx;

    element_idx = fwk_id_get_element_idx(stream->id);

    cfg = fwk_module_get_data(stream->id);
    ctx = &mod_stdio_ctx.elements[element_idx];

    if (cfg->type == MOD_STDIO_ELEMENT_TYPE_PATH)
        status = (fclose(ctx->stream) == 0) ? FWK_SUCCESS : FWK_E_OS;

    return status;
}

const struct fwk_module module_stdio = {
    .type = FWK_MODULE_TYPE_DRIVER,

    .init = mod_stdio_init,
    .element_init = mod_stdio_element_init,

    .adapter = {
        .open = mod_stdio_open,
        .getch = mod_stdio_getc,
        .putch = mod_stdio_putc,
        .close = mod_stdio_close,
    },
};

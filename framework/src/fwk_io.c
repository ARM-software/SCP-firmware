/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_attributes.h>
#include <fwk_io.h>
#include <fwk_mm.h>
#include <fwk_module.h>

#include <stddef.h>
#include <stdio.h>

int fwk_io_null_open(const struct fwk_io_stream *stream)
{
    return FWK_SUCCESS;
}

int fwk_io_null_getch(const struct fwk_io_stream *stream, char *ch)
{
    return FWK_PENDING;
}

int fwk_io_null_putch(const struct fwk_io_stream *stream, char ch)
{
    return FWK_SUCCESS;
}

int fwk_io_null_close(const struct fwk_io_stream *stream)
{
    return FWK_SUCCESS;
}

static struct fwk_io_stream fwk_io_null = {
    .adapter =
        &(const struct fwk_io_adapter){
            .open = fwk_io_null_open,
            .getch = fwk_io_null_getch,
            .putch = fwk_io_null_putch,
            .close = fwk_io_null_close,
        },

    .id = FWK_ID_NONE,
    .mode = (enum fwk_io_mode)(
        FWK_IO_MODE_READ | FWK_IO_MODE_WRITE | FWK_IO_MODE_BINARY),
};

struct fwk_io_stream *fwk_io_stdin = &fwk_io_null;
struct fwk_io_stream *fwk_io_stdout = &fwk_io_null;

int fwk_io_init(void)
{
    static struct fwk_io_stream stdin_stream;
    static struct fwk_io_stream stdout_stream;

    int status = FWK_SUCCESS;

    bool configure_stdin = !fwk_id_is_equal(FMW_IO_STDIN_ID, FWK_ID_NONE);
    bool configure_stdout = !fwk_id_is_equal(FMW_IO_STDOUT_ID, FWK_ID_NONE);

    bool stdout_is_stdin = fwk_id_is_equal(FMW_IO_STDIN_ID, FMW_IO_STDOUT_ID);

    if (configure_stdin) {
        if (stdout_is_stdin) {
            /*
             * If stdin and stdout share the same entity, we only want to open
             * it once and we want to do it with both read and write modes
             * enabled, rather than opening the same entity twice with different
             * modes.
             */

            status = fwk_io_open(
                &stdin_stream,
                FMW_IO_STDIN_ID,
                (enum fwk_io_mode)(
                    ((unsigned int)FWK_IO_MODE_READ) |
                    ((unsigned int)FWK_IO_MODE_WRITE)));
            if (fwk_expect(status == FWK_SUCCESS)) {
                fwk_io_stdin = &stdin_stream;
                fwk_io_stdout = &stdin_stream;
            }
        } else {
            status =
                fwk_io_open(&stdin_stream, FMW_IO_STDIN_ID, FWK_IO_MODE_READ);
            if (fwk_expect(status == FWK_SUCCESS)) {
                fwk_io_stdin = &stdin_stream;
            }
        }
    }

    if (configure_stdout && !stdout_is_stdin) {
        status =
            fwk_io_open(&stdout_stream, FMW_IO_STDOUT_ID, FWK_IO_MODE_WRITE);
        if (fwk_expect(status == FWK_SUCCESS)) {
            fwk_io_stdout = &stdout_stream;
        }
    }

    if (status != FWK_SUCCESS) {
        status = FWK_E_DEVICE;
    }

    return status;
}

int fwk_io_open(
    struct fwk_io_stream *restrict stream,
    fwk_id_t id,
    enum fwk_io_mode mode)
{
    int status;

    bool read = (((unsigned int)mode & (unsigned int)FWK_IO_MODE_READ) != 0U);
    bool write = (((unsigned int)mode & (unsigned int)FWK_IO_MODE_WRITE) != 0U);

    if (stream == NULL) {
        return FWK_E_PARAM;
    }

    *stream = (struct fwk_io_stream){
        .adapter = NULL,
        .id = id,
        .mode = mode,
    };

    if (!read && !write) {
        return FWK_E_PARAM; /* Neither reading nor writing requested */
    }

    status = fwk_module_adapter(&stream->adapter, id);
    if (status != FWK_SUCCESS) {
        return FWK_E_PARAM; /* System entity doesn't exist */
    }

    fwk_assert(stream->adapter != NULL);

    if (stream->adapter->open == NULL) {
        return FWK_E_SUPPORT; /* Stream adapter is not implemented */
    }

    if ((read && (stream->adapter->getch == NULL))) {
        return FWK_E_SUPPORT; /* Reads requested but no read interface */
    }

    if (write && (stream->adapter->putch == NULL)) {
        return FWK_E_SUPPORT; /* Writes requested but no write interface */
    }

    status = stream->adapter->open(stream);
    if (status != FWK_SUCCESS) {
        return FWK_E_HANDLER;
    }

    return FWK_SUCCESS;
}

int fwk_io_getch(const struct fwk_io_stream *stream, char *ch)
{
    int status;

    if (stream == NULL) {
        return FWK_E_PARAM;
    }

    if (ch == NULL) {
        return FWK_E_PARAM;
    }

    *ch = '\0';

    if (stream->adapter == NULL) {
        return FWK_E_STATE; /* The stream is not open */
    }

    if ((((unsigned int)stream->mode) & ((unsigned int)FWK_IO_MODE_READ)) ==
        0U) {
        return FWK_E_SUPPORT; /* Stream not open for read operations */
    }

    fwk_assert(stream->adapter->getch != NULL);

    status = stream->adapter->getch(stream, ch);
    if (status == FWK_PENDING) {
        return FWK_PENDING;
    } else if (status != FWK_SUCCESS) {
        return FWK_E_HANDLER;
    }

    return FWK_SUCCESS;
}

int fwk_io_putch(const struct fwk_io_stream *stream, char ch)
{
    int status;

    if (stream == NULL) {
        return FWK_E_PARAM;
    }

    if (stream->adapter == NULL) {
        return FWK_E_STATE; /* The stream is not open */
    }

    if ((((unsigned int)stream->mode) & ((unsigned int)FWK_IO_MODE_WRITE)) ==
        0U) {
        return FWK_E_SUPPORT; /* Stream not open for write operations */
    }

    fwk_assert(stream->adapter->putch != NULL);

    do {
        /* Wait for the adapter to accept new characters */
        status = stream->adapter->putch(stream, ch);
    } while (status == FWK_E_BUSY);

    if (status != FWK_SUCCESS) {
        return FWK_E_HANDLER;
    }

    return status;
}

int fwk_io_putch_nowait(const struct fwk_io_stream *stream, char ch)
{
    int status;

    if (stream == NULL) {
        return FWK_E_PARAM;
    }

    if (stream->adapter == NULL) {
        return FWK_E_STATE; /* The stream is not open */
    }

    if ((((unsigned int)stream->mode) & ((unsigned int)FWK_IO_MODE_WRITE)) ==
        0U) {
        return FWK_E_SUPPORT; /* Stream not open for write operations */
    }

    fwk_assert(stream->adapter->putch != NULL);

    status = stream->adapter->putch(stream, ch);

    if ((status != FWK_SUCCESS) && (status != FWK_E_BUSY)) {
        return FWK_E_HANDLER;
    }

    return status;
}

int fwk_io_read(
    const struct fwk_io_stream *restrict stream,
    size_t *restrict read,
    void *restrict buffer,
    size_t size,
    size_t count)
{
    int status = FWK_SUCCESS;

    char *cbuffer = buffer;

    if (read != NULL) {
        *read = 0;
    }

    for (size_t i = 0; (i < count) && (status == FWK_SUCCESS); i++) {
        for (size_t j = 0; (j < size) && (status == FWK_SUCCESS); j++) {
            status = fwk_io_getch(stream, cbuffer++);
        }

        if ((status == FWK_SUCCESS) && (read != NULL)) {
            *read += 1;
        }
    }

    if ((status == FWK_PENDING) && (read == NULL)) {
        return FWK_E_DATA; /* Reached end-of-stream */
    }

    return status;
}

int fwk_io_write(
    const struct fwk_io_stream *restrict stream,
    size_t *restrict written,
    const void *restrict buffer,
    size_t size,
    size_t count)
{
    int status = FWK_SUCCESS;

    const char *cbuffer = buffer;

    if (cbuffer == NULL) {
        return FWK_E_PARAM;
    }

    if (written != NULL) {
        *written = 0;
    }

    for (size_t i = 0; (i < count) && (status == FWK_SUCCESS); i++) {
        for (size_t j = 0; (j < size) && (status == FWK_SUCCESS); j++) {
            status = fwk_io_putch(stream, *cbuffer++);
        }

        if ((status == FWK_SUCCESS) && (written != NULL)) {
            *written += 1;
        }
    }

    return status;
}

int fwk_io_close(struct fwk_io_stream *stream)
{
    int status;

    if (stream == NULL) {
        return FWK_E_PARAM;
    }

    if (stream->adapter == NULL) {
        return FWK_SUCCESS; /* The stream is not open */
    }

    if (stream->adapter->close == NULL) {
        return FWK_SUCCESS; /* Nothing else to do */
    }

    status = stream->adapter->close(stream);

    *stream = (struct fwk_io_stream){
        .adapter = NULL,
        .id = FWK_ID_NONE,
        .mode = 0,
    };

    if (status != FWK_SUCCESS) {
        return FWK_E_HANDLER;
    }

    return FWK_SUCCESS;
}

int fwk_io_puts(
    const struct fwk_io_stream *restrict stream,
    const char *restrict str)
{
    if (str == NULL) {
        return FWK_E_PARAM;
    }

    return fwk_io_write(stream, NULL, str, sizeof(char), strlen(str));
}

int fwk_io_vprintf(
    const struct fwk_io_stream *restrict stream,
    const char *restrict format,
    va_list args)
{
    int status = FWK_SUCCESS;

    int length;
    va_list length_args;

    char *buffer;

    if (format == NULL) {
        return FWK_E_PARAM;
    }

    va_copy(length_args, args);
    length = vsnprintf(NULL, 0, format, length_args);
    va_end(length_args);

    if (length < 0) { /* Possibly invalid format string? */
        return FWK_E_STATE;
    }

    buffer = fwk_mm_alloc_notrap(sizeof(buffer[0]), (size_t)(length + 1));
    if (buffer == NULL) { /* Not enough memory for the string buffer */
        return FWK_E_NOMEM;
    }

    length = vsnprintf(buffer, (size_t)(length + 1), format, args);
    if (length >= 0) {
        status = fwk_io_puts(stream, buffer); /* Write out the buffer */
    } else {
        status = FWK_E_STATE;
    }

    fwk_mm_free(buffer);

    return status;
}

int fwk_io_printf(
    const struct fwk_io_stream *restrict stream,
    const char *restrict format,
    ...)
{
    int status;

    va_list args;

    va_start(args, format);
    status = fwk_io_vprintf(stream, format, args);
    va_end(args);

    return status;
}

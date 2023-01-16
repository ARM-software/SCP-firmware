/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_attributes.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_ring.h>
#include <fwk_status.h>
#include <fwk_time.h>

#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static const char FWK_LOG_TERMINATOR[] = FMW_LOG_ENDLINE_STR;

static struct {
    unsigned int dropped; /* Count of messages lost */

#ifdef FWK_LOG_BUFFERED
    struct fwk_ring ring; /* Buffer for formatted messages */

    unsigned char remaining; /* Remaining characters in the current message */
#endif
} fwk_log_ctx = { 0 };

static struct fwk_io_stream *fwk_log_stream;

#ifdef FWK_LOG_BUFFERED
static FWK_CONSTRUCTOR void fwk_log_stream_init(void)
{
    static char storage[FMW_LOG_BUFFER_SIZE];

    fwk_ring_init(&fwk_log_ctx.ring, storage, sizeof(storage));
}
#endif

int fwk_log_init(void)
{
    static struct fwk_io_stream stream;

    int status = FWK_SUCCESS;

    if (fwk_id_is_equal(FMW_LOG_DRAIN_ID, FMW_IO_STDOUT_ID)) {
        fwk_log_stream = fwk_io_stdout;
    } else if (!fwk_id_is_equal(FMW_LOG_DRAIN_ID, FWK_ID_NONE)) {
        status = fwk_io_open(&stream, FMW_LOG_DRAIN_ID, FWK_IO_MODE_WRITE);
        if (fwk_expect(status == FWK_SUCCESS)) {
            fwk_log_stream = &stream;
        }
    }

    return status;
}

#ifdef FWK_LOG_BUFFERED
static bool fwk_log_buffer(struct fwk_ring *ring, const char *message)
{
    unsigned char length = strlen(message) + 1; /* +1 for null terminator */

    /*
     * Log messages are stored in the ring buffer prefixed with their length
     * (including the null terminator). Care must be taken to ensure the length
     * of each message does not exceed `UCHAR_MAX`.
     */

    if ((sizeof(length) + length) > fwk_ring_get_free(ring)) {
        return false; /* Not enough buffer space */
    }

    fwk_ring_push(ring, (char *)&length, sizeof(length));
    fwk_ring_push(ring, message, length);

    return true;
}
#endif

static void fwk_log_vsnprintf(
    size_t buffer_size,
    char buffer[buffer_size],
    const char *format,
    va_list *args)
{
    fwk_timestamp_t timestamp = 0;
    fwk_duration_ns_t duration = 0;

    uint32_t duration_s = 0;
    uint32_t duration_us = 0;

    size_t length = 0;

    char *newline;

    buffer_size -= strlen(FWK_LOG_TERMINATOR);

    /*
     * We start by generating a timestamp for the message using the number of
     * nanoseconds since boot.
     */

    timestamp = fwk_time_current();
    duration = fwk_time_stamp_duration(timestamp);

    /*
     * Newlib support for printing 64-bit integers with `printf()` is
     * optional and actually disabled by default in GNU Arm Embedded. To
     * print the timestamp values under this configuration we need to
     * truncate the timestamp values to 32 bits.
     *
     * This gives us a theoretical maximum of 136 years' worth of seconds on the
     * timestamp, which is still plenty.
     */

    duration_s = (uint32_t)fwk_time_duration_s(duration);
    duration_us = (uint32_t)fwk_time_duration_us(duration % FWK_S(1));

    /* Generate the timestamp at the beginning of the buffer */
    length = (size_t)snprintf(
        buffer,
        buffer_size,
        "[%5" PRIu32 ".%06" PRIu32 "] ",
        duration_s,
        duration_us);
    fwk_assert(length < buffer_size);

    /*
     * We then need to `snprintf()` the message into a temporary buffer because
     * we need to manipulate it before we print or store it.
     */

    length += vsnprintf(buffer + length, buffer_size - length, format, *args);
    length = FWK_MIN(length, buffer_size - 1);

    /*
     * Figure out if the user has included a newline, in which case we consider
     * that to be the end of the message. This stops us from being able to
     * create multi-line messages, but means we can properly generate timestamps
     * on a line-by-line basis.
     */

    newline = strstr(buffer, FWK_LOG_TERMINATOR);
    if (newline == NULL) {
        newline = buffer + length;
    }

    /*
     * Lastly, we follow through on the termination with a proper carriage
     * return and newline. Terminals that don't care about the carriage return
     * will generally ignore it, but most terminals require it in order to start
     * the next line at the first column.
     */

    (void)memcpy(newline, FWK_LOG_TERMINATOR, sizeof(FWK_LOG_TERMINATOR));
}

static void fwk_log_snprintf(
    size_t buffer_size,
    char buffer[buffer_size],
    const char *format,
    ...)
{
    va_list args;

    va_start(args, format);
    fwk_log_vsnprintf(buffer_size, buffer, format, &args);
    va_end(args);
}

static bool fwk_log_banner(void)
{
    char buffer[FMW_LOG_COLUMNS + sizeof(FWK_LOG_TERMINATOR)];

#ifndef FMW_LOG_MINIMAL_BANNER
    const char *banner[] = {
        " ___  ___ ___      __ _",
        "/ __|/ __| _ \\___ / _(_)_ _ _ ____ __ ____ _ _ _ ___",
        "\\__ | (__|  _|___|  _| | '_| '  \\ V  V / _` | '_/ -_)",
        "|___/\\___|_|     |_| |_|_| |_|_|_\\_/\\_/\\__,_|_| \\___|",
        "",
        BUILD_VERSION_DESCRIBE_STRING,
        "",
    };
#else
    const char *banner[] = { "SCP-firmware " BUILD_VERSION_DESCRIBE_STRING,
                             "" };
#endif
    for (unsigned int i = 0; i < FWK_ARRAY_SIZE(banner); i++) {
        fwk_log_snprintf(
            sizeof(buffer), buffer, "%s%s", banner[i], FWK_LOG_TERMINATOR);
        if (fwk_io_puts(fwk_log_stream, buffer) != FWK_SUCCESS) {
            return false;
        }
    }

    return true;
}

void fwk_log_printf(const char *format, ...)
{
    unsigned int flags;
    static bool banner = false;

    char buffer[FMW_LOG_COLUMNS + sizeof(FWK_LOG_TERMINATOR)];

    va_list args;

    flags = fwk_interrupt_global_disable(); /* Facilitate reentrancy */

    /*
     * We don't have any way for the log drain entity to communicate that it is
     * ready to accept prints, so our best bet for printing the banner is just
     * to keep trying to do it before every call to this function, until it
     * succeeds.
     */

    if (!banner) {
        banner = fwk_log_banner();
    }

    va_start(args, format);
    fwk_log_vsnprintf(sizeof(buffer), buffer, format, &args);
    va_end(args);

#ifdef FWK_LOG_BUFFERED
    /*
     * Buffer the message that we've received so that the scheduler can choose
     * when we do the heavy-lifting (typically once we're in an idle state).
     */

    bool dropped = !fwk_log_buffer(&fwk_log_ctx.ring, buffer);
    if (dropped) {
        /*
         * If we don't have enough room left in the buffer, then we're out of
         * luck. We don't want to spend what are likely to be precious cycles
         * printing on the always-on backend, so our best option is simply to
         * mark the message as dropped and move on.
         */

        fwk_log_ctx.dropped++;
    }
#else
    int status = fwk_io_puts(fwk_log_stream, buffer);
    if (status != FWK_SUCCESS) {
        fwk_log_ctx.dropped++;
    }
#endif

    (void)fwk_interrupt_global_enable(flags);
}

int fwk_log_unbuffer(void)
{
    int status = FWK_SUCCESS;

#ifdef FWK_LOG_BUFFERED
    unsigned int flags;
    unsigned char fetched;
    char ch;

    flags = fwk_interrupt_global_disable();

    if (fwk_log_ctx.remaining == 0) {
        /*
         * We've finished printing whatever message we were previously on, so we
         * need to try and fetch the next one.
         */

        bool empty = !fwk_ring_pop(
            &fwk_log_ctx.ring,
            (char *)&fwk_log_ctx.remaining,
            sizeof(fwk_log_ctx.remaining));

        if (empty) {
            /*
             * At this point we've cleared the buffer of any remaining messages.
             * If we were forced to drop any messages prior to this point, now
             * is a good time to let the user know.
             */

            if (fwk_log_ctx.dropped > 0) {
                fwk_log_printf(
                    "[FWK] ... and %u more messages...", fwk_log_ctx.dropped);

                fwk_log_ctx.dropped = 0;

                status = FWK_PENDING;
            }

            goto exit;
        }
    }

    /*
     * Grab the next character from the ring buffer and try to print it.
     * Printing the character successfully will result in a pending return value
     * even if it is the last character in the message - the next call to this
     * function will run the logic above to finalize the message.
     */

    fetched = fwk_ring_pop(&fwk_log_ctx.ring, &ch, sizeof(ch));
    fwk_assert(fetched == sizeof(char));

    status = fwk_io_putch(fwk_log_stream, ch);
    if (status == FWK_SUCCESS) {
        fwk_log_ctx.remaining--;

        status = FWK_PENDING;
    }

exit:
    fwk_interrupt_global_enable(flags);
#endif

    return status;
}

void fwk_log_flush(void)
{
#ifdef FWK_LOG_BUFFERED
    unsigned int flags;
    int status;

    flags = fwk_interrupt_global_disable();

    do {
        status = fwk_log_unbuffer();
    } while (status == FWK_PENDING);

    fwk_interrupt_global_enable(flags);
#endif
}

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_ring.h>
#include <fwk_status.h>

#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static const char FWK_LOG_TERMINATOR[] = { '\r', '\n', '\0' };

static struct {
    unsigned int dropped; /* Count of messages lost */

    const struct fwk_log_backend *aon_backend; /* Always-on logging backend */
    const struct fwk_log_backend *backend; /* Dynamic logging backend */

#ifdef FWK_LOG_BUFFERED
    struct fwk_ring ring; /* Buffer for formatted messages */

    unsigned char remaining; /* Remaining characters in the current message */
#endif
} fwk_log_ctx = { 0 };

static int fwk_log_print(const struct fwk_log_backend *backend, char ch)
{
    int status;

    if (ch == '\0') {
        /*
         * The null terminator indicates the end of the message. We don't want
         * to send that to the terminal - it doesn't mean anything - but we can
         * take this opportunity to flush the message out, ensuring the user
         * doesn't see just half of a message.
         */

        if (backend->flush == NULL)
            status = FWK_SUCCESS;
        else
            status = backend->flush();
    } else
        status = backend->print(ch);

    if (status != FWK_SUCCESS)
        status = FWK_E_DEVICE;

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

    if ((sizeof(length) + length) > fwk_ring_get_free(ring))
        return false; /* Not enough buffer space */

    fwk_ring_push(ring, (char *)&length, sizeof(length));
    fwk_ring_push(ring, message, length);

    return true;
}
#endif

static void fwk_log_format(
    char (*buffer)[FMW_LOG_COLUMNS + sizeof(FWK_LOG_TERMINATOR)],
    const char *format,
    va_list *args)
{
    char *newline;

    /*
     * Firstly, we need to `snprintf()` the message into a temporary buffer
     * because we need to manipulate it before we store it in the ring buffer.
     */

    (void)vsnprintf(
        *buffer,
        sizeof(*buffer) - sizeof(FWK_LOG_TERMINATOR) + 1,
        format,
        *args);

    /*
     * Secondly, we need to figure out if the user has accidentally included
     * their own newline, in which case we consider that to be the end of the
     * message.
     */

    newline = strchr(*buffer, '\n');
    if (newline != NULL)
        *newline = '\0';
    else
        newline = *buffer + strlen(*buffer);

    /*
     * Lastly, we follow through on that termination with a proper carriage
     * return and newline. Terminals that don't care about the carriage return
     * will generally ignore it, but most terminals require it in order to start
     * the next line at the first column.
     */

    memcpy(newline, FWK_LOG_TERMINATOR, sizeof(FWK_LOG_TERMINATOR));
}

void fwk_log_snprintf(const char *format, ...)
{
    const struct fwk_log_backend *backend = NULL;
    bool buffered = false;

    char buffer[FMW_LOG_COLUMNS + sizeof(FWK_LOG_TERMINATOR)];

    va_list args;

    va_start(args, format);
    fwk_log_format(&buffer, format, &args);
    va_end(args);

    if (fwk_log_ctx.backend != NULL) {
        backend = fwk_log_ctx.backend;

#ifdef FWK_LOG_BUFFERED
        buffered = true;
#endif
    } else if (fwk_log_ctx.aon_backend != NULL)
        backend = fwk_log_ctx.aon_backend;
    else {
#ifdef FWK_LOG_BUFFERED
        buffered = true;
#endif
    }

    fwk_interrupt_global_disable(); /* Facilitate reentrancy */

    if (buffered) {
#ifdef FWK_LOG_BUFFERED
        /*
         * Buffer the message that we've received so that the scheduler can
         * choose when we do the heavy-lifting (typically once we're in an idle
         * state).
         */

        bool dropped = !fwk_log_buffer(&fwk_log_ctx.ring, buffer);

        if (dropped) {
            /*
             * If we don't have enough room left in the buffer, then we're out
             * of luck. We don't want to spend what are likely to be precious
             * cycles printing on the always-on backend, so our best option is
             * simply to mark the message as dropped and move on.
             */

            fwk_log_ctx.dropped++;
        }
#endif
    } else if (backend != NULL) {
        /*
         * Print the message right now. This is used if buffering is not
         * enabled, or when a dynamic backend hasn't been registered but an
         * always-on backend has.
         */

        char *ch = buffer;

        while (*ch != '\0') {
            int status = fwk_log_print(backend, *ch++);

            if (status == FWK_PENDING)
                break;
        }
    } else {
        /*
         * We can't buffer the message, and we haven't been given a backend
         * through which we can print the message now, so we just have to drop
         * this message.
         */

        fwk_log_ctx.dropped++;
    }

    fwk_interrupt_global_enable();
}

void fwk_log_init(void)
{
#ifdef FWK_LOG_BUFFERED
    static char storage[FMW_LOG_BUFFER_SIZE];

    fwk_ring_init(&fwk_log_ctx.ring, storage, sizeof(storage));
#endif
}

int fwk_log_unbuffer(void)
{
    int status = FWK_SUCCESS;

#ifdef FWK_LOG_BUFFERED
    unsigned char fetched;
    char ch;

    fwk_interrupt_global_disable();

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
                fwk_log_snprintf(
                    "[FWK] ... and %u more messages...", fwk_log_ctx.dropped);

                fwk_log_ctx.dropped = 0;

                status = FWK_PENDING;
            }

            goto exit;
        }
    }

    /*
     * Before we try to print anything, we should ensure that we still have a
     * dynamic backend available on which to print - it may have been
     * deregistered.
     */

    if (fwk_log_ctx.backend == NULL) {
        status = FWK_PENDING;

        goto exit;
    }

    /*
     * Grab the next character from the ring buffer and try to print it.
     * Printing the character successfully will result in a pending return value
     * even if it is the last character in the message - the next call to this
     * function will run the logic above to finalize the message.
     */

    fetched = fwk_ring_pop(&fwk_log_ctx.ring, &ch, sizeof(ch));
    fwk_assert(fetched == sizeof(char));

    status = fwk_log_print(fwk_log_ctx.backend, ch);
    if (status == FWK_SUCCESS) {
        fwk_log_ctx.remaining--;

        status = FWK_PENDING;
    }

exit:
    fwk_interrupt_global_enable();
#endif

    return status;
}

void fwk_log_flush(void)
{
#ifdef FWK_LOG_BUFFERED
    int status;

    fwk_interrupt_global_disable();

    do {
        status = fwk_log_unbuffer();
    } while (status == FWK_PENDING);

    fwk_interrupt_global_enable();
#endif

    if (fwk_log_ctx.backend != NULL)
        fwk_log_ctx.backend->flush();

    if (fwk_log_ctx.aon_backend != NULL)
        fwk_log_ctx.aon_backend->flush();
}

int fwk_log_register_aon(const struct fwk_log_backend *backend)
{
    if (fwk_log_ctx.aon_backend != NULL)
        return FWK_E_INIT;

    fwk_log_ctx.aon_backend = backend;

    return FWK_SUCCESS;
}

const struct fwk_log_backend *fwk_log_deregister_aon(void)
{
    const struct fwk_log_backend *backend = NULL;

    backend = fwk_log_ctx.aon_backend;

    fwk_log_ctx.aon_backend = NULL;

    return backend;
}

int fwk_log_register(const struct fwk_log_backend *backend)
{
    if (fwk_log_ctx.backend != NULL)
        return FWK_E_INIT;

    fwk_log_ctx.backend = backend;

    return FWK_SUCCESS;
}

const struct fwk_log_backend *fwk_log_deregister(void)
{
    const struct fwk_log_backend *backend = NULL;

    backend = fwk_log_ctx.backend;

    fwk_log_ctx.backend = NULL;

    return backend;
}

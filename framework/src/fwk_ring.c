/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_ring.h>

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

static size_t fwk_ring_offset(const struct fwk_ring *ring, size_t idx)
{
    return (idx % ring->capacity);
}

void fwk_ring_init(struct fwk_ring *ring, char *storage, size_t storage_size)
{
    fwk_assert(ring != NULL);
    fwk_assert(storage != NULL);
    fwk_assert(storage_size > 0);

    *ring = (struct fwk_ring){
        .storage = storage,
        .capacity = storage_size,
    };

    fwk_ring_clear(ring);
}

size_t fwk_ring_get_capacity(const struct fwk_ring *ring)
{
    fwk_assert(ring != NULL);

    return ring->capacity;
}

size_t fwk_ring_get_length(const struct fwk_ring *ring)
{
    fwk_assert(ring != NULL);

    if ((ring->tail >= ring->head) && !ring->full)
        return (ring->tail - ring->head);
    else
        return ((ring->capacity - ring->head) + ring->tail);
}

size_t fwk_ring_get_free(const struct fwk_ring *ring)
{
    fwk_assert(ring != NULL);

    return ring->capacity - fwk_ring_get_length(ring);
}

bool fwk_ring_is_full(const struct fwk_ring *ring)
{
    fwk_assert(ring != NULL);

    return ring->full;
}

bool fwk_ring_is_empty(const struct fwk_ring *ring)
{
    fwk_assert(ring != NULL);

    return ((ring->head == ring->tail) && !ring->full);
}

size_t fwk_ring_pop(struct fwk_ring *ring, char *buffer, size_t buffer_size)
{
    fwk_assert(ring != NULL);

    buffer_size = (buffer == NULL) ?
        FWK_MIN(buffer_size, fwk_ring_get_length(ring)) :
        fwk_ring_peek(ring, buffer, buffer_size);

    if (buffer_size > 0) {
        ring->head = fwk_ring_offset(ring, ring->head + buffer_size);

        ring->full = false;
    }

    return buffer_size;
}

size_t fwk_ring_peek(
    const struct fwk_ring *ring,
    char *buffer,
    size_t buffer_size)
{
    fwk_assert(ring != NULL);
    fwk_assert(buffer != NULL);

    buffer_size = FWK_MIN(buffer_size, fwk_ring_get_length(ring));
    if (buffer_size == 0)
        return buffer_size;

    if (fwk_ring_offset(ring, ring->head + buffer_size) > ring->head)
        memcpy(buffer, ring->storage + ring->head, buffer_size);
    else {
        size_t chunk_size = ring->capacity - ring->head;

        memcpy(buffer, ring->storage + ring->head, chunk_size);
        memcpy(buffer + chunk_size, ring->storage, buffer_size - chunk_size);
    }

    return buffer_size;
}

size_t fwk_ring_push(
    struct fwk_ring *ring,
    const char *buffer,
    size_t buffer_size)
{
    size_t remaining;

    fwk_assert(ring != NULL);
    fwk_assert(buffer != NULL);

    if (buffer_size == 0)
        return buffer_size;

    if (buffer_size > ring->capacity) {
        /*
         * If the user has requested that we write more bytes than the ring
         * buffer can hold, capture only the trailing bytes.
         */

        buffer += (buffer_size - ring->capacity);
        buffer_size = ring->capacity;
    }

    remaining = fwk_ring_get_free(ring);

    if (fwk_ring_offset(ring, ring->tail + buffer_size) > ring->tail)
        memcpy(ring->storage + ring->tail, buffer, buffer_size);
    else {
        size_t chunk_size = ring->capacity - ring->tail;

        memcpy(ring->storage + ring->tail, buffer, chunk_size);
        memcpy(ring->storage, buffer + chunk_size, buffer_size - chunk_size);
    }

    ring->tail = fwk_ring_offset(ring, ring->tail + buffer_size);

    if (buffer_size >= remaining) {
        ring->head = ring->tail;
        ring->full = true;
    }

    /*
     * Note that if the user tried to write more than the ring buffer could hold
     * then the length returned will be the amount of data actually written,
     * which is the capacity of the ring buffer.
     */

    return buffer_size;
}

void fwk_ring_clear(struct fwk_ring *ring)
{
    fwk_assert(ring != NULL);

    ring->head = 0;
    ring->tail = 0;

    ring->full = false;
}

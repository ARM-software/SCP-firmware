/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Memory management.
 */

#include <fwk_assert.h>
#include <fwk_mm.h>

#include <stdlib.h>

void *fwk_mm_alloc(size_t num, size_t size)
{
    void *ptr = malloc(num * size);

    if (ptr == NULL) {
        fwk_trap();
    }

    return ptr;
}

void *fwk_mm_alloc_notrap(size_t num, size_t size)
{
    return malloc(num * size);
}

void *fwk_mm_alloc_aligned(size_t alignment, size_t num, size_t size)
{
    void *ptr = aligned_alloc(alignment, num * size);

    if (ptr == NULL) {
        fwk_trap();
    }

    return ptr;
}

void *fwk_mm_calloc(size_t num, size_t size)
{
    void *ptr = calloc(num, size);
    if (ptr == NULL) {
        fwk_trap();
    }

    return ptr;
}

void *fwk_mm_calloc_aligned(size_t alignment, size_t num, size_t size)
{
    void *ptr = fwk_mm_alloc_aligned(alignment, num, size);

    if (ptr != NULL) {
        (void)memset(ptr, 0, num * size);

        return ptr;
    }

    fwk_trap();
}

void *fwk_mm_realloc(void *ptr, size_t num, size_t size)
{
    return realloc(ptr, num * size);
}

void fwk_mm_free(void *ptr)
{
    return free(ptr);
}

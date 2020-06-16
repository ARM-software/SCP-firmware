/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Memory management.
 */

#include <fwk_mm.h>

#include <stdlib.h>

void *fwk_mm_alloc(size_t num, size_t size)
{
    return malloc(num * size);
}

void *fwk_mm_alloc_aligned(size_t alignment, size_t num, size_t size)
{
    return aligned_alloc(alignment, num * size);
}

void *fwk_mm_calloc(size_t num, size_t size)
{
    return calloc(num, size);
}

void *fwk_mm_calloc_aligned(size_t alignment, size_t num, size_t size)
{
    void *ptr = fwk_mm_alloc_aligned(alignment, num, size);

    if (ptr != NULL)
        memset(ptr, 0, num * size);

    return ptr;
}

void *fwk_mm_realloc(void *ptr, size_t num, size_t size)
{
    return realloc(ptr, num * size);
}

void fwk_mm_free(void *ptr)
{
    return free(ptr);
}

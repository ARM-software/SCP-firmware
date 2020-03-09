/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Memory management.
 */

#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_status.h>

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static bool initialized;
static bool mm_locked;
static uintptr_t heap_free;
static uintptr_t heap_end;

/*
 * Initialize the memory management component.
 *
 * This function is not exposed by the memory management component but is
 * used by the framework during its initialization routine.
 *
 * \retval FWK_SUCCESS Initialization was successful.
 * \retval FWK_E_STATE The component has already been initialized.
 * \retval FWK_E_RANGE There is a problem with the memory layout provided.
 */
int fwk_mm_init(uintptr_t start, size_t size)
{
    if (initialized)
        return FWK_E_STATE;

    if ((start == 0) || (size == 0))
        return FWK_E_RANGE;

    heap_free = start;
    heap_end = start + size;

    initialized = true;

    return FWK_SUCCESS;
}

void fwk_mm_lock(void)
{
    mm_locked = true;
}

void *fwk_mm_alloc(size_t num, size_t size)
{
    return fwk_mm_alloc_aligned(num, size, FWK_MM_DEFAULT_ALIGNMENT);
}

void *fwk_mm_alloc_aligned(size_t num, size_t size, unsigned int alignment)
{
    uintptr_t start;
    size_t total_size;
    bool overflow;

    if (mm_locked || !num || !size || !alignment || !initialized)
        goto error;

    /* Ensure 'alignment' is a power of two */
    if (alignment & (alignment - 1))
        goto error;

    overflow = __builtin_mul_overflow(num, size, &total_size);

    /* Ensure the computation of 'total_size' has not overflowed */
    if (overflow)
        goto error;

    start = FWK_ALIGN_NEXT(heap_free, alignment);

    /* Ensure there is no overflow during the alignment */
    if (start < heap_free)
        goto error;

    /* Ensure 'total_size' fits in the remaining heap area */
    if (total_size > (heap_end - start))
        goto error;

    heap_free = start + total_size;

    return (void *)start;

error:
    fwk_trap();
}

void *fwk_mm_calloc(size_t num, size_t size)
{
    return fwk_mm_calloc_aligned(num, size, FWK_MM_DEFAULT_ALIGNMENT);
}

void *fwk_mm_calloc_aligned(size_t num, size_t size, unsigned int alignment)
{
    void *start;

    start = fwk_mm_alloc_aligned(num, size, alignment);
    if (start != NULL)
        memset(start, 0, num * size);

    return start;
}

#ifdef __NEWLIB__
void *_sbrk(intptr_t increment)
{
    if (increment == 0) {
        return (void *)heap_end;
    } else {
        errno = ENOMEM;

        return (void *)-1;
    }
}
#endif

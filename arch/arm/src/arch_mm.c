/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Memory initialization.
 */

#include <fwk_macros.h>

#include <stddef.h>
#include <stdint.h>

#if FWK_HAS_INCLUDE(<sys/features.h>)
#    include <sys/features.h>
#endif

#ifdef __NEWLIB__
#    include <errno.h>
#    include <malloc.h>

extern char __stackheap_start__;
extern char __stackheap_end__;

/*!
 * \brief Architecture memory manager context.
 */
static struct arch_mm_ctx_str {
    /*!
     * \brief Current heap break address.
     */
    uintptr_t heap_break;
} arch_mm_ctx = {
    .heap_break = ((uintptr_t)&__stackheap_start__),
};

int posix_memalign(void **memptr, size_t alignment, size_t size)
{
    if (alignment == 0) {
        return EINVAL;
    }

    /* Enforce power-of-two alignment */
    if ((alignment & (alignment - 1)) != 0) {
        return EINVAL;
    }

    if ((alignment % sizeof(void *)) != 0) {
        return EINVAL;
    }

    if (size == 0) {
        *memptr = NULL;
    } else {
        *memptr = _memalign_r(_REENT, alignment, size);

        if (*memptr == NULL) {
            return ENOMEM;
        }
    }

    return 0;
}

void *_sbrk(intptr_t increment)
{
    if (increment == 0) {
        return (void *)arch_mm_ctx.heap_break;
    } else {
        uintptr_t heap_old = arch_mm_ctx.heap_break;
        uintptr_t heap_new = arch_mm_ctx.heap_break + increment;

        if (heap_new > ((uintptr_t)&__stackheap_end__)) {
            errno = ENOMEM;

            return (void *)-1;
        } else {
            arch_mm_ctx.heap_break = heap_new;

            return (void *)heap_old;
        }
    }
}
#endif

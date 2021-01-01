/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>

#ifdef __ARMCC_VERSION
void *aligned_alloc(size_t alignment, size_t size)
{
    int error;
    void *memptr;

    error = posix_memalign(&memptr, alignment, size);

    return (error == 0 ? memptr : NULL);
}
#endif

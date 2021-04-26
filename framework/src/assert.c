/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#if defined(__ARMCC_VERSION)
void __aeabi_assert(const char *expr, const char *file, int line)
{
    while (1) {
        continue;
    }
}
#elif defined(__NEWLIB__)
void __assert_func(
    const char *file,
    int line,
    const char *function,
    const char *assertion)
{
    while (1) {
        continue;
    }
}
#endif

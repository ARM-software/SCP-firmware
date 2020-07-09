/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_attributes.h>
#include <fwk_io.h>

#include <assert.h>

#if defined(__ARMCC_VERSION)
void __aeabi_assert(const char *expr, const char *file, int line)
{
    (void)fwk_io_printf(
        fwk_io_stdout, "Assertion failed at %s:%d\n", file, line);

    while (1) {
        continue;
    }
}
#elif defined(__NEWLIB__)
FWK_WEAK void __assert_func(
    const char *file,
    int line,
    const char *function,
    const char *assertion)
{
    (void)fwk_io_printf(
        fwk_io_stdout,
        "Assertion failed in %s at %s:%d\n",
        function,
        file,
        line);

    while (1) {
        continue;
    }
}
#endif

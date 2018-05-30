/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdlib.h>
#include <fwk_assert.h>
#include <fwk_interrupt.h>

void fwk_trap(void)
{
    #ifdef MODE_DEBUG
        /*
         * Disable interrupts to ensure the program state cannot be disturbed by
         * interrupt handlers.
         */
        fwk_interrupt_global_disable();

        while (true)
            continue;
    #else
        __builtin_trap();
    #endif
}

void fwk_unreachable(void)
{
    #ifdef MODE_DEBUG
        fwk_trap();
    #else
        /*
         * Let the optimizer know that anything after this point is unreachable.
         */
        __builtin_unreachable();
    #endif
}

bool fwk_expect(bool condition)
{
    /* Failed expectations are potentially recoverable */
    #if defined(MODE_DEBUG) && !defined(BUILD_TESTS)
        if (!condition)
            fwk_trap();
    #endif

    return condition;
}

void fwk_assert(bool condition)
{
    /* Failed invariants are unrecoverable */
    if (!condition)
        fwk_unreachable();
}

#ifdef __NEWLIB__
void __assert_func(const char *file, int line,
    const char *function, const char *assertion)
{
    fwk_trap();
}
#endif

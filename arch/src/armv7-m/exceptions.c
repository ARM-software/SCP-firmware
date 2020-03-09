/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      ARMv7-M exception handlers.
 */

#include "exceptions.h"

#include <stdint.h>
#include <string.h>

#ifdef __NEWLIB__
/*
 * This function overloads a weak definition provided by Newlib. It is called
 * during initialization of the C runtime just after .bss has been zeroed.
 */
void software_init_hook(void)
{
    extern char __data_load__;
    extern char __data_start__;
    extern char __data_end__;

    char *load = &__data_load__;
    char *start = &__data_start__;
    char *end = &__data_end__;

    if (load != start)
        memcpy(start, load, end - start);
}
#endif

enum {
    EXCEPTION_RESET,
    EXCEPTION_NMI,
    EXCEPTION_HARDFAULT,
    EXCEPTION_MEMMANAGE,
    EXCEPTION_BUSFAULT,
    EXCEPTION_USAGEFAULT,
    EXCEPTION_RESERVED0,
    EXCEPTION_RESERVED1,
    EXCEPTION_RESERVED2,
    EXCEPTION_RESERVED3,
    EXCEPTION_SVCALL,
    EXCEPTION_DEBUGMONITOR,
    EXCEPTINO_RESERVED4,
    EXCEPTION_PENDSV,
    EXCEPTION_SYSTICK,
};

#ifdef __ARMCC_VERSION
extern char Image$$ARM_LIB_STACKHEAP$$ZI$$Limit;

#   define arm_exception_stack (&Image$$ARM_LIB_STACKHEAP$$ZI$$Limit)
#else
extern char __stackheap_end__;

#   define arm_exception_stack (&__stackheap_end__)
#endif

/*
 * Set up the exception table. The structure below is is added to the
 * .exceptions section which will be explicitly placed at the beginning of the
 * binary by the linker script.
 */

const struct {
    uintptr_t stack;
    uintptr_t exceptions[15];
} arm_exceptions __attribute__((section(".exceptions"))) = {
    .stack = (uintptr_t)(arm_exception_stack),
    .exceptions = {
        [EXCEPTION_RESET] = (uintptr_t)(arm_exception_reset),
        [EXCEPTION_NMI] = (uintptr_t)(arm_exception_invalid),
        [EXCEPTION_HARDFAULT] = (uintptr_t)(arm_exception_invalid),
        [EXCEPTION_MEMMANAGE] = (uintptr_t)(arm_exception_invalid),
        [EXCEPTION_BUSFAULT] = (uintptr_t)(arm_exception_invalid),
        [EXCEPTION_USAGEFAULT] = (uintptr_t)(arm_exception_invalid),
        [EXCEPTION_SVCALL] = (uintptr_t)(arm_exception_invalid),
        [EXCEPTION_DEBUGMONITOR] = (uintptr_t)(arm_exception_invalid),
        [EXCEPTION_PENDSV] = (uintptr_t)(arm_exception_invalid),
        [EXCEPTION_SYSTICK] = (uintptr_t)(arm_exception_invalid),
    }
};

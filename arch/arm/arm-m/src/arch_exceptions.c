/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      ARM-M exception handlers.
 */

#include "arch_exceptions.h"

#include <fwk_attributes.h>

#include <fmw_cmsis.h>

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

    if (load != start) {
        (void)memcpy(start, load, (size_t)(end - start));
    }
}
#endif

#ifdef __ARMCC_VERSION
extern char Image$$ARM_LIB_STACKHEAP$$ZI$$Limit;

#    define arch_exception_stack (&Image$$ARM_LIB_STACKHEAP$$ZI$$Limit)
#else
extern char __stackheap_end__;

#    define arch_exception_stack (&__stackheap_end__)
#endif

/*
 * Set up the exception table. The structure below is added to the
 * .exceptions section which will be explicitly placed at the beginning of the
 * binary by the linker script.
 */

const struct {
    uintptr_t stack;
    uintptr_t exceptions[NVIC_USER_IRQ_OFFSET - 1];
} arch_exceptions FWK_SECTION(".exceptions") = {
    .stack = (uintptr_t)(arch_exception_stack),
    .exceptions = {
        [NVIC_USER_IRQ_OFFSET + Reset_IRQn - 1] =
            (uintptr_t)(arch_exception_reset),
        [NonMaskableInt_IRQn +  (NVIC_USER_IRQ_OFFSET - 1)] =
            (uintptr_t)(arch_exception_invalid),
        [NVIC_USER_IRQ_OFFSET + HardFault_IRQn - 1] =
            (uintptr_t)(arch_exception_invalid),
        [NVIC_USER_IRQ_OFFSET + MemoryManagement_IRQn - 1] =
            (uintptr_t)(arch_exception_invalid),
        [NVIC_USER_IRQ_OFFSET + BusFault_IRQn - 1] =
            (uintptr_t)(arch_exception_invalid),
        [NVIC_USER_IRQ_OFFSET + UsageFault_IRQn - 1] =
            (uintptr_t)(arch_exception_invalid),
#ifdef ARMV8M
        [NVIC_USER_IRQ_OFFSET + SecureFault_IRQn - 1] =
            (uintptr_t)(arch_exception_invalid),
#endif
        [NVIC_USER_IRQ_OFFSET + DebugMonitor_IRQn - 1] =
            (uintptr_t)(arch_exception_invalid),

        [NVIC_USER_IRQ_OFFSET + SVCall_IRQn - 1] =
            (uintptr_t)(arch_exception_invalid),
        [NVIC_USER_IRQ_OFFSET + PendSV_IRQn - 1] =
            (uintptr_t)(arch_exception_invalid),
        [NVIC_USER_IRQ_OFFSET + SysTick_IRQn - 1] =
            (uintptr_t)(arch_exception_invalid),
    },
};

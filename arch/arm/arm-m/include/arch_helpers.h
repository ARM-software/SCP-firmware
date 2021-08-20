/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_HELPERS_H
#define ARCH_HELPERS_H

#include <fmw_cmsis.h>

/*
 * This variable is used to ensure spurious nested calls won't
 * enable interrupts. This is been defined in arch_main.c
 */
extern unsigned int critical_section_nest_level;

/*!
 * \brief Enables global CPU interrupts.
 *
 * \note inline is necessary as this call can be used in performance sensitive
 *     path
 */
inline static void arch_interrupts_enable(unsigned int not_used)
{
    /* Decrement critical_section_nest_level only if in critical section */
    if (critical_section_nest_level > 0) {
        critical_section_nest_level--;
    }

    /* Enable interrupts globally if now outside critical section */
    if (critical_section_nest_level == 0) {
        __enable_irq();
    }
}

/*!
 * \brief Disables global CPU interrupts.

 * \note inline is necessary as this call can be used in performance sensitive
 *     path
 */
inline static unsigned int arch_interrupts_disable(void)
{
    critical_section_nest_level++;

    /* If now in outer-most critical section, disable interrupts globally */
    if (critical_section_nest_level == 1) {
        __disable_irq();
    }

    return 0;
}

/*!
 * \brief Suspend execution of current CPU.

 * \note CPU will be woken up by receiving interrupts.
 *
 */
inline static void arch_suspend(void)
{
    __asm volatile("wfe");
}

#endif /* ARCH_HELPERS_H */

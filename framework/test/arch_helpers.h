/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_HELPERS_H
#define ARCH_HELPERS_H
/*
 * This variable is used to ensure spurious nested calls won't
 * enable interrupts. This is been defined in fwk_test.c
 */
extern unsigned int critical_section_nest_level;

/*!
 * \brief Enables global CPU interrupts. (stub)
 *
 */
inline static void arch_interrupts_enable(unsigned int not_used)
{
    /* Decrement critical_section_nest_level only if in critical section */
    if (critical_section_nest_level > 0) {
        critical_section_nest_level--;
    }
}

/*!
 * \brief Disables global CPU interrupts. (stub)
 *
 */
inline static unsigned int arch_interrupts_disable(void)
{
    critical_section_nest_level++;

    return 0;
}

/*!
 * \brief Suspend execution of current CPU.
 *
 */
inline static void arch_suspend(void)
{
}

#endif /* ARCH_HELPERS_H */

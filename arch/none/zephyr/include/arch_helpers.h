/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_HELPERS_H
#define ARCH_HELPERS_H

#include <arch_interrupt.h>

/*!
 * \brief Enables global CPU interrupts. (stub)
 *
 */
inline static void arch_interrupts_enable(unsigned int flags)
{
    zephyr_irq_unlock(flags);
}

/*!
 * \brief Disables global CPU interrupts. (stub)
 *
 */
inline static unsigned int arch_interrupts_disable(void)
{
    return zephyr_irq_lock();
}

/*!
 * \brief Suspend execution of current CPU.
 *
 */
inline static void arch_suspend(void)
{
}

#endif /* ARCH_HELPERS_H */

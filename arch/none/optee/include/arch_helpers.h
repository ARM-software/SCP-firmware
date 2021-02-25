/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_HELPERS_H
#define ARCH_HELPERS_H

/*!
 * \brief Enables global CPU interrupts. (stub)
 *
 */
inline static void arch_interrupts_enable(unsigned int not_used)
{
}

/*!
 * \brief Disables global CPU interrupts. (stub)
 *
 */
inline static unsigned int arch_interrupts_disable(void)
{
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

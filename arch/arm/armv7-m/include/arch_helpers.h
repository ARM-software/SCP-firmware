/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef ARCH_HELPERS_H
#define ARCH_HELPERS_H

/*!
 * \brief Enables global CPU interrupts.
 *
 * \note inline is necessary as this call can be used in performance sensitive
 *     path
 */
inline static void arch_interrupts_enable(void)
{
    __asm volatile("cpsie i" : : : "memory");
}

/*!
 * \brief Disables global CPU interrupts.

 * \note inline is necessary as this call can be used in performance sensitive
 *     path
 */
inline static void arch_interrupts_disable(void)
{
    __asm volatile("cpsid i" : : : "memory");
}

#endif /* ARCH_HELPERS_H */

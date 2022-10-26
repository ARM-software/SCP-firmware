/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cmsis_compiler.h>

#include <fwk_arch.h>
#include <fwk_assert.h>
#include <fwk_macros.h>

#include <arch_nvic.h>

#include <fmw_cmsis.h>

#include <stdbool.h>
#include <stdint.h>

/*
 * This variable is used to ensure spurious nested calls won't
 * enable interrupts. This is been accessed from inline function defined in
 * arch_helpers.h
 */
unsigned int critical_section_nest_level;

#ifdef __NEWLIB__
/* Platform-dependent backend for the _Exit() function */
void _exit(int status)
{
    while (true) {
        __WFI();
    }
}
#endif

static const struct fwk_arch_init_driver arch_init_driver = {
    .interrupt = arch_nvic_init,
};

#ifndef ARMV6M
static void arch_init_ccr(void)
{
    /*
     * Set up the Configuration Control Register (CCR) in the System Control
     * Block (1) by setting the following flag bits:
     *
     * DIV_0_TRP   [4]: Enable trapping on division by zero. (1)(2)
     * STKALIGN    [9]: Enable automatic DWORD stack-alignment on exception
     *                  entry (3).
     *
     * All other bits are left in their default state.
     *
     * (1) ARM® v7-M Architecture Reference Manual, section B3.2.8.
     * (2) Arm® v8-M Architecture Reference Manual, section D1.2.9.
     * (3) ARM® v7-M Architecture Reference Manual, section B1.5.7.
     */

    SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;
#ifdef ARMV7M
    SCB->CCR |= SCB_CCR_STKALIGN_Msk;
#endif
}
#endif

int main(void)
{
#ifndef ARMV6M
    arch_init_ccr();
#endif

    return fwk_arch_init(&arch_init_driver);
}

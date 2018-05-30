/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

    .syntax unified

    .text

    .global exception_reset
    .thumb_func
    .func exception_reset
exception_reset:
    /*
     * Set up the Configuration Control Register (CCR) in the System Control
     * Block (1) by setting the following flag bits:
     *
     * UNALIGN_TRP [3]: Enable trapping on unaligned word or halfword accesses.
     * DIV_0_TRP   [4]: Enable trapping on division by zero.
     * STKALIGN    [9]: Enable automatic DWORD stack-alignment on exception
     *                  entry (2).
     *
     * All other bits are left in their default state.
     *
     * (1) ARM® v7-M Architecture Reference Manual, section B3.2.8.
     * (2) ARM® v7-M Architecture Reference Manual, section B1.5.7.
     */
    ldr r0, =0xE000ED14 /* Load CCR address (architecture-defined) */
    ldr r1, [r0]        /* Load existing CCR value */
    orr r1, #0x218      /* Set flag bits */
    str r1, [r0]        /* Store modified value back to the CCR */

    bl      start
    .pool
    .endfunc

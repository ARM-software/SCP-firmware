/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      C runtime initialization for Newlib.
 */

    .syntax unified

    .text

    .global exception_reset
    .thumb_func
    .type exception_reset, %function
exception_reset:
    b       _start /* Enter Newlib's crt0 */
    .pool

/*
 * The following *_init_hook functions are called from _start and provide an
 * early opportunity to do some critical runtime setup. From the point of view
 * of the runtime, there is no difference between the hardware and software
 * hook, but the hardware hook is called before the software hook. Accesses to
 * mem1 are invalid until the data residing in it has been initialized by the
 * software hook.
 */

    .global hardware_init_hook
    .thumb_func
    .type hardware_init_hook, %function
hardware_init_hook:
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

    bx  lr
    .pool

    .global software_init_hook
    .thumb_func
    .type software_init_hook, %function
software_init_hook:
    ldr r0, =__data_start__
    ldr r1, =__data_lma_start__
    ldr r2, =__data_size__

    cmp r0, r1
    bne memcpy /* Copy .data from ROM to RAM if necessary */
    bx  lr /* Otherwise return */
    .pool

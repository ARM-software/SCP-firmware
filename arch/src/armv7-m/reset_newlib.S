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

    .global arm_exception_reset
    .thumb_func
    .type arm_exception_reset, %function
arm_exception_reset:
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
    b arm_init_ccr /* Initialize the CCR register */
    .pool

    .global software_init_hook
    .thumb_func
    .type software_init_hook, %function
software_init_hook:
    ldr r0, =__data_start_vma__
    ldr r1, =__data_start_lma__
    ldr r2, =__data_size__

    cmp r0, r1
    bne memcpy /* Copy .data from ROM to RAM if necessary */
    bx  lr /* Otherwise return */
    .pool

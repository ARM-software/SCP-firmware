/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

    .syntax unified

    .text

/*
 * noreturn void mod_bootloader_boot(
 *     uint8_t *destination,
 *     uint8_t *source,
 *     uint32_t size,
 *     uint32_t *vtor);
 */
    .thumb
    .thumb_func
    .global mod_bootloader_boot
    .type mod_bootloader_boot, %function
mod_bootloader_boot:
    movs r4, r0 /* Save the destination - it soon points to the vector table */

1:
    ldrb r5, [r1], #1 /* Load next byte from source */
    strb r5, [r0], #1 /* Store next byte at destination */

    subs r2, #1 /* Decrement the size, which we use as the counter... */
    bne 1b /* ... until it reaches zero */

    str r4, [r3] /* Store vector table address in SCB->VTOR (if it exists) */

    ldr r0, [r4] /* Grab new stack pointer from vector table... */
    msr msp, r0 /* ... and update the main stack pointer with it */

    ldr r0, [r4, #4] /* Load the reset address from the vector table... */
    bx r0 /* ... and take a leap of faith */

    .pool

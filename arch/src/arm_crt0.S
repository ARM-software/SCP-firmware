/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

    .syntax unified

    .text

    .global start
    .thumb_func
    .func start
start:
    /* Load .data, which is 4-byte aligned */
    ldr     r1, =__DATA_SIZE__
    cmp     r1, #0
    beq     6f
    ldr     r0, =__DATA_LMA_START__
    ldr     r2, =__DATA_START__
    cmp     r0, r2
    beq     6f
5:
    ldr     r3, [r0], #4
    str     r3, [r2], #4
    subs    r1, #4
    bne     5b

6:
    /* Clear .bss, which is also 4-byte aligned */
    ldr     r1, =__BSS_SIZE__
    cmp     r1, #0
    beq     8f
    ldr     r0, =__BSS_START__
    mov     r2, #0
7:
    str     r2, [r0], #4
    subs    r1, #4
    bne     7b

8:
    dsb
    bl      arm_main

9:
    wfi
    b 9b
    .pool
    .endfunc

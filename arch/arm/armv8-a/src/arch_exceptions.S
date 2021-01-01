/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

    .globl _vector_table
    .globl _freertos_vector_table
    .extern _entrypoint

.section .vectors, "ax"
.balign 2048
_vector_table:

.set VBAR, _vector_table
    /* -----------------------------------------------------
     * Current EL with SP0 : 0x0 - 0x200 (at EL3)
     * -----------------------------------------------------
     */
.org VBAR
    b   _entrypoint
.org (VBAR + 0x80)
    b   .
.org (VBAR + 0x100)
    b   .
.org (VBAR + 0x180)
    b   .

    /* -----------------------------------------------------
     * Current EL with SPx: 0x200 - 0x400 (at EL3)
     * -----------------------------------------------------
     */
.org (VBAR + 0x200)
    b   .
.org (VBAR + 0x280)
    b   .
.org (VBAR + 0x300)
    b   .
.org (VBAR + 0x380)
    b   .

    /* -----------------------------------------------------
     * Lower EL using AArch64 : 0x400 - 0x600 (at EL3)
     * -----------------------------------------------------
     */
.org (VBAR + 0x400)
    b   .
.org (VBAR + 0x480)
    b   .
.org (VBAR + 0x500)
    b   .
.org (VBAR + 0x580)
    b   .

    /* -----------------------------------------------------
     * Lower EL using AArch32 : 0x600 - 0x800 (at EL3)
     * -----------------------------------------------------
     */
.org (VBAR + 0x600)
    b   .
.org (VBAR + 0x680)
    b   .
.org (VBAR + 0x700)
    b   .
.org (VBAR + 0x780)
    b   .

/******************************************************************************
 * Vector table to use when FreeRTOS is running.
 *****************************************************************************/
.set    FREERTOS_VBAR, (VBAR+0x0800)

    /* -----------------------------------------------------
     * Current EL with SP0 : 0x0 - 0x200 (at EL3)
     * -----------------------------------------------------
     */
.org(FREERTOS_VBAR)
_freertos_vector_table:
    b   FreeRTOS_SWI_Handler
.org (FREERTOS_VBAR + 0x80)
    b   FreeRTOS_IRQ_Handler
.org (FREERTOS_VBAR + 0x100)
    b   FreeRTOS_IRQ_Handler
.org (FREERTOS_VBAR + 0x180)
    b   .

    /* -----------------------------------------------------
     * Current EL with SPx: 0x200 - 0x400 (at EL3)
     * -----------------------------------------------------
     */
.org (FREERTOS_VBAR + 0x200)
    b   FreeRTOS_SWI_Handler
.org (FREERTOS_VBAR + 0x280)
    b   FreeRTOS_IRQ_Handler
.org (FREERTOS_VBAR + 0x300)
    b   FreeRTOS_IRQ_Handler
.org (FREERTOS_VBAR + 0x380)
    b   .

    /* -----------------------------------------------------
     * Lower EL using AArch64 : 0x400 - 0x600 (at EL3)
     * -----------------------------------------------------
     */
.org (FREERTOS_VBAR + 0x400)
    b   .
.org (FREERTOS_VBAR + 0x480)
    b   .
.org (FREERTOS_VBAR + 0x500)
    b   .
.org (FREERTOS_VBAR + 0x580)
    b   .

    /* -----------------------------------------------------
     * Lower EL using AArch32 : 0x600 - 0x800 (at EL3)
     * -----------------------------------------------------
     */
.org (FREERTOS_VBAR + 0x600)
    b   .
.org (FREERTOS_VBAR + 0x680)
    b   .
.org (FREERTOS_VBAR + 0x700)
    b   .
.org (FREERTOS_VBAR + 0x780)
    b   .

    .end

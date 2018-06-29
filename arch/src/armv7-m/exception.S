/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

    .syntax unified

    .section .exception, "a", %progbits

/*
 * We set up the exception table here. The table is put inside the ".exception"
 * section, which is marked as PROGBITS (takes up space in the binary). The
 * "exceptions" symbol is given the same ELF attributes as a standard C
 * structure (allocatable object) in order to ensure it is properly exported
 * or loaded. See the GNU assembler documentation for the exact meaning behind
 * the directives provided here.
 */

    .global exceptions
exceptions:
    .word __stack
    .word exception_reset   /* Reset */
    .word exception_invalid /* NMI */
    .word exception_invalid /* HardFault */
    .word exception_invalid /* MemManage */
    .word exception_invalid /* Bus Fault */
    .word exception_invalid /* UsageFault */
    .word exception_invalid /* Reserved */
    .word exception_invalid /* Reserved */
    .word exception_invalid /* Reserved */
    .word exception_invalid /* Reserved */
    .word exception_invalid /* SVCall */
    .word exception_invalid /* DebugMonitor */
    .word exception_invalid /* Reserved */
    .word exception_invalid /* PendSV */
    .word exception_invalid /* SysTick */

    .type exceptions, %object
    .size exceptions, . - exceptions

    .section .text

    .global exception_invalid
    .thumb_func
    .type exception_invalid, %function
exception_invalid:
    wfi
    b       exception_invalid

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
 * the directives provided here. See the .exception section described in the GNU
 * linker script for the first two entries of the exception table.
 */

    .global exceptions
exceptions:
    .word arm_exception_invalid /* NMI */
    .word arm_exception_invalid /* HardFault */
    .word arm_exception_invalid /* MemManage */
    .word arm_exception_invalid /* Bus Fault */
    .word arm_exception_invalid /* UsageFault */
    .word arm_exception_invalid /* Reserved */
    .word arm_exception_invalid /* Reserved */
    .word arm_exception_invalid /* Reserved */
    .word arm_exception_invalid /* Reserved */
    .word arm_exception_invalid /* SVCall */
    .word arm_exception_invalid /* DebugMonitor */
    .word arm_exception_invalid /* Reserved */
    .word arm_exception_invalid /* PendSV */
    .word arm_exception_invalid /* SysTick */

    .type exceptions, %object
    .size exceptions, . - exceptions

    .section .text

    .global arm_exception_invalid
    .thumb_func
    .type arm_exception_invalid, %function
    .eabi_attribute Tag_ABI_align_preserved, 1
arm_exception_invalid:
    wfi
    b       arm_exception_invalid

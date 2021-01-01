/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_system.h>
#include <asm_macros.S>

    .section .entrypoint

    .globl _entrypoint
    .globl _restore_system
    .globl _save_system
    .globl _boot_flag
    .globl _shutdown_request

    .extern _vector_table
    .extern arm_main
    .extern rcar_pwrc_set_suspend_to_ram
#ifdef BUILD_HAS_NOTIFICATION
    .extern __fwk_notification_reset
#endif

func _entrypoint
    ldr w0, _boot_flag
    ldr w1, =R_WARMBOOT
    cmp w1, w0
    beq _restore_system

    ldr x0,  =__STACK_TOP__
    mov sp, x0 /* SP_EL3 */

    adr x0, _vector_table
    msr vbar_el3, x0
    isb

    msr spsel, #0
    ldr x0, =__STACK_SP0_TOP__
    mov sp, x0 /* SP_EL0 */
    stp x29, x30, [sp, #-32]!

    adr x0, __RW_START__
    adr x1, __RW_END__
    sub x1, x1, x0
    bl  inv_dcache_range

    ldr x0, =__BSS_START__
    ldr x1, =__BSS_SIZE__
    bl  zeromem

#if USE_COHERENT_MEM
    ldr x0, =__COHERENT_RAM_START__
    ldr x1, =__COHERENT_RAM_UNALIGNED_SIZE__
    bl  zeromem
#endif

    mrs   x0, scr_el3
    /* RW[10]=1, HCE[8]=0, SMD[7]=0, EA[3]=1, FIQ[2]=1, IRQ[1]=1, NS[0]=0 */
    mov   x0, #(1<<10 | 0<<8 | 0<<7 | 1<<3 | 1<<2 | 1<<1 | 0<<0)
    msr   scr_el3, x0

    /* --------------------------------------------------
     * Initialize platform and jump to our c-entry point
     * for this type of reset.
     * --------------------------------------------------
     */
#ifdef BUILD_HAS_NOTIFICATION
    bl  __fwk_notification_reset
#endif
    bl  arm_main

    mov x0, 1
    ldp x29, x30, [sp], #32

    ret

endfunc _entrypoint

func _save_system /* EL3t */
    stp x2, x3, [sp, #-0x10]!
    ldr x2, =_save_area_top
    stp x0, x1, [x2, #-0x10]!
    mov x0, x2
    ldp x2, x3, [sp], #0x10
    stp x2, x3, [x0, #-0x10]!
    stp x4, x5, [x0, #-0x10]!
    stp x6, x7, [x0, #-0x10]!
    stp x8, x9, [x0, #-0x10]!
    stp x10, x11, [x0, #-0x10]!
    stp x12, x13, [x0, #-0x10]!
    stp x14, x15, [x0, #-0x10]!
    stp x16, x17, [x0, #-0x10]!
    stp x18, x19, [x0, #-0x10]!
    stp x20, x21, [x0, #-0x10]!
    stp x22, x23, [x0, #-0x10]!
    stp x24, x25, [x0, #-0x10]!
    stp x26, x27, [x0, #-0x10]!
    stp x28, x29, [x0, #-0x10]!
    stp x30, xzr, [x0, #-0x10]!

    mov x2, sp
    msr spsel, #1
    mov x3, sp
    msr spsel, #0
    stp x2, x3, [x0, #-0x10]!   /* Save SP_EL0, SP_EL3 */

    mrs x3, scr_el3
    mov x2, x30
    stp x2, x3, [x0, #-0x10]!   /* Save elr_el3(lr), scr_el3 */

    mrs x2, nzcv
    mrs x3, daif
    orr x2, x2, x3
    mrs x3, CurrentEL
    orr x2, x2, x3
    mrs x3, SPSel
    orr x2, x2, x3
    mrs x3, vbar_el3
    stp x2, x3, [x0, #-0x10]!   /* Save spsr_el3(psr), vbar_el3 */

    bl rcar_pwrc_set_suspend_to_ram
1:
    wfi
    b 1b
endfunc _save_system

func _restore_system /* EL3h */
    ldr x0, =_save_area_bottom

    ldp x2, x3, [x0], #0x10     /* Restore spsr_el3(psr), vbar_el3 */
    msr spsr_el3, x2
    msr vbar_el3, x3

    ldp x2, x3, [x0], #0x10     /* Restore elr_el3(lr), scr_el3 */
    msr elr_el3, x2
    msr scr_el3, x3

    ldp x2, x3, [x0], #0x10     /* Restore SP_EL0, SP_EL3 */
    mov sp, x3
    msr spsel, #0
    mov sp, x2

    ldp x30, xzr, [x0], #0x10
    ldp x28, x29, [x0], #0x10
    ldp x26, x27, [x0], #0x10
    ldp x24, x25, [x0], #0x10
    ldp x22, x23, [x0], #0x10
    ldp x20, x21, [x0], #0x10
    ldp x18, x19, [x0], #0x10
    ldp x16, x17, [x0], #0x10
    ldp x14, x15, [x0], #0x10
    ldp x12, x13, [x0], #0x10
    ldp x10, x11, [x0], #0x10
    ldp x8, x9, [x0], #0x10
    ldp x6, x7, [x0], #0x10
    ldp x4, x5, [x0], #0x10
    ldp x2, x3, [x0], #0x10
    stp x2, x3, [sp, #-0x10]!
    mov x2, x0
    ldp x0, x1, [x2], #0x10
    ldp x2, x3, [sp], #0x10

    msr spsel, #1
    eret
endfunc _restore_system

    .section .data.context
    .align 4
_boot_flag:
    .long 0

    .align 4
_shutdown_request:
    .long 0

    .align 4
_save_area_bottom:
    .rept 38
    .long 0, 0
    .endr
_save_area_top:

    .end

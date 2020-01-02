/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      GNU LD linker script.
 */

#include "scatter.h"

ENTRY(arm_exception_reset)

MEMORY {
#if FIRMWARE_MEM_MODE == FWK_MEM_MODE_SINGLE_REGION
    /*
     * Single region memory layout:
     *  - MEM0 accepts:
     *      - Read-only sections
     *      - Read-write sections
     *      - Executable sections
     */

    mem0 (rwx) : ORIGIN = FIRMWARE_MEM0_BASE, LENGTH = FIRMWARE_MEM0_SIZE
#elif FIRMWARE_MEM_MODE == FWK_MEM_MODE_DUAL_REGION_RELOCATION
    /*
     * Dual region memory layout with initialized data relocation:
     *  - MEM0 accepts:
     *      - Read-only sections
     *      - Executable sections
     *
     *  - MEM1 accepts:
     *      - Read-write sections
     */

    mem0 (rx) : ORIGIN = FIRMWARE_MEM0_BASE, LENGTH = FIRMWARE_MEM0_SIZE
    mem1 (w) : ORIGIN = FIRMWARE_MEM1_BASE, LENGTH = FIRMWARE_MEM1_SIZE
#elif FIRMWARE_MEM_MODE == FWK_MEM_MODE_DUAL_REGION_NO_RELOCATION
    /*
     * Dual region memory layout without initialized data relocation:
     *  - MEM0 accepts:
     *      - Executable sections
     *
     *  - MEM1 accepts:
     *      - Read-only sections
     *      - Read-write sections
     */

    mem0 (x) : ORIGIN = FIRMWARE_MEM0_BASE, LENGTH = FIRMWARE_MEM0_SIZE
    mem1 (rw) : ORIGIN = FIRMWARE_MEM1_BASE, LENGTH = FIRMWARE_MEM1_SIZE
#endif
}

#if FIRMWARE_MEM_MODE == FWK_MEM_MODE_SINGLE_REGION
    REGION_ALIAS("r", mem0);
    REGION_ALIAS("w", mem0);
    REGION_ALIAS("x", mem0);
#elif FIRMWARE_MEM_MODE == FWK_MEM_MODE_DUAL_REGION_RELOCATION
    REGION_ALIAS("r", mem0);
    REGION_ALIAS("w", mem1);
    REGION_ALIAS("x", mem0);
#elif FIRMWARE_MEM_MODE == FWK_MEM_MODE_DUAL_REGION_NO_RELOCATION
    REGION_ALIAS("r", mem1);
    REGION_ALIAS("w", mem1);
    REGION_ALIAS("x", mem0);
#endif

SECTIONS {
    /*
     * Variables defined here:
     *   - __data_load__: Load address of .data
     *   - __data_start__: Start address of .data
     *   - __data_end__: End address of .data and .data-like orphans
     *   - __bss_start__: Start address of .bss
     *   - __bss_end__: End address of .bss and .bss-like orphans
     *   - __stackheap_start__: Start address of .stackheap
     *   - __stackheap_end__: End address of .stackheap
     *   - __stack: Initial stack pointer
     */

    .exceptions : {
        KEEP(*(.exceptions))
    } > x

    .text : {
        *(.text .text.*)
    } > x

    /*
     * .init and .fini below refer to sections containing functions meant to
     * run before program startup and after program shutdown. While we don't use
     * these functions, Newlib refers to them unconditionally and they are,
     * perhaps counterintuitively, stripped from the binary unless marked as
     * KEEP.
     */

    .init : {
        KEEP(*(.init))
    } > x

    .fini : {
        KEEP(*(.fini))
    } > x

    .init_array : {
        __init_array_start = .;
        KEEP(*(.init_array))
        __init_array_end = .;
    } > r

    .rodata : {
        *(.rodata .rodata.*)
    } > r

    .data : {
        __data_load__ = LOADADDR(.data);
        __data_start__ = ABSOLUTE(.);

        *(.data .data.*)
    } > w AT> r

    .bss (NOLOAD) : {
        __bss_start__ = ABSOLUTE(.);

        *(.bss .bss.*)
    } > w

    .stackheap (NOLOAD) : {
        __stackheap_start__ = ABSOLUTE(.);

        . = ORIGIN(w) + LENGTH(w);

        __stackheap_end__ = ABSOLUTE(.);
    } > w

    /*
     * By default the linker places orphan sections after the section with the
     * closest matching attributes. Calculating the end of a section based on
     * the beginning of the next one that we know about means we can include BSS
     * and DATA sections that we don't know about in these values.
     */

    __data_end__ = __bss_start__;
    __bss_end__ = __stackheap_start__;

    __stack = __stackheap_end__;
}

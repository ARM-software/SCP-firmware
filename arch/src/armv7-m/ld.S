/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
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

    .rodata : {
        *(.rodata .rodata.*)
    } > r

    .data : {
        *(.data .data.*)
    } > w AT> r

    .bss : {
        *(.bss .bss.*)
    } > w

    .stackheap ALIGN(HEAP_ALIGNMENT) : {
        . = ORIGIN(w) + LENGTH(w) - (STACK_ALIGNMENT - 1);
        . = ALIGN(STACK_ALIGNMENT);
    } > w

    __data_load__ = LOADADDR(.data);
    __data_start__ = ADDR(.data);
    __data_end__ = ADDR(.data) + SIZEOF(.data);

    __bss_start__ = ADDR(.bss);
    __bss_end__ = ADDR(.bss) + SIZEOF(.bss);

    __stackheap_start__ = ADDR(.stackheap);
    __stackheap_end__ = ADDR(.stackheap) + SIZEOF(.stackheap);

    __stack = __stackheap_end__;
}

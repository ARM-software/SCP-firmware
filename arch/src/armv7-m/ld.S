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
    mem0 : ORIGIN = FIRMWARE_MEM0_BASE, LENGTH = FIRMWARE_MEM0_SIZE
#else
    mem0 (rx) : ORIGIN = FIRMWARE_MEM0_BASE, LENGTH = FIRMWARE_MEM0_SIZE
    mem1 (!rx) : ORIGIN = FIRMWARE_MEM1_BASE, LENGTH = FIRMWARE_MEM1_SIZE
#endif
}

#if FIRMWARE_MEM_MODE == FWK_MEM_MODE_SINGLE_REGION
    REGION_ALIAS("mem1", mem0);
#endif

REGION_ALIAS("vexception", mem0);
REGION_ALIAS("vtext", mem0);
REGION_ALIAS("vrodata", mem0);
REGION_ALIAS("vdata", mem1);
REGION_ALIAS("vbss", mem1);
REGION_ALIAS("vstackheap", mem1);

#if FIRMWARE_MEM_MODE == FWK_MEM_MODE_DUAL_REGION_NO_RELOCATION
    REGION_ALIAS("ldata", mem1); /* .data is loaded into mem1 by a loader */
#else
    REGION_ALIAS("ldata", mem0); /* .data is loaded into mem1 by crt0 */
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
    } > vexception

    .text : {
        *(.text .text.*)
    } > vtext

    .rodata : {
        *(.rodata .rodata.*)
    } > vrodata

    .data : {
        *(.data .data.*)
    } > vdata AT>ldata

    .bss : {
        *(.bss .bss.*)
    } > vbss

    .stackheap ALIGN(HEAP_ALIGNMENT) : {
        . = ORIGIN(vstackheap) + LENGTH(vstackheap) - (STACK_ALIGNMENT - 1);
        . = ALIGN(STACK_ALIGNMENT);
    } > vstackheap

    __data_load__ = LOADADDR(.data);
    __data_start__ = ADDR(.data);
    __data_end__ = ADDR(.data) + SIZEOF(.data);

    __bss_start__ = ADDR(.bss);
    __bss_end__ = ADDR(.bss) + SIZEOF(.bss);

    __stackheap_start__ = ADDR(.stackheap);
    __stackheap_end__ = ADDR(.stackheap) + SIZEOF(.stackheap);

    __stack = __stackheap_end__;
}

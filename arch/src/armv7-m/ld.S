/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      GNU LD linker script.
 *
 * There are three supported memory layouts for the ARMv7-M architecture:
 *
 * Layout 1 - Single region:
 * - All sections are placed in one contiguous region.
 * - This layout uses only the mem0 memory region.
 * - The memory is considered RXW by the linker, but the sections can be
 *   configured later on with different attributes using the MPU.
 * - The main stack is placed at the end of mem0.
 * - This layout is mainly used by second-stage firmware that is loaded directly
 *   into a single RAM.
 *
 * Layout 2 - Dual region with relocation:
 * - One region is used for .text and .data (storage).
 * - A second region is used for the remaining sections.
 * - This layout uses memory regions mem0 and mem1 as the first and second
 *   regions, respectively.
 * - The main stack is placed at the end of mem1.
 * - This layout is mainly used by ROM firmware which uses part of the RAM for
 *   the data sections.
 *
 * Layout 3 - Dual region without relocation
 * - One region is used only for the .text section.
 * - A second region is used for all data sections.
 * - This layout uses memory regions mem0 and mem1 as the first and second
 *   regions, respectively.
 * - The main stack is placed at the end of mem1.
 * - The main difference from layout 2 is that there is no relocation of the
 *   .data section.
 * - This layout is mainly used by second-stage firmware loaded directly into
 *   two RAM regions. One of the RAM regions is attached to the instruction bus,
 *   which improves the performance as data and instruction accesses are
 *   independent.
 *
 */

#define FWK_MEM_MODE_INVALID                   0
#define FWK_MEM_MODE_SINGLE_REGION             1
#define FWK_MEM_MODE_DUAL_REGION_RELOCATION    2
#define FWK_MEM_MODE_DUAL_REGION_NO_RELOCATION 3

#include <fmw_memory.ld.S>

/* Align the stack on an 8-byte boundary as CCR.STKALIGN is enabled */
#define STACK_ALIGNMENT 8

/* Align the heap on a 4-byte boundary to avoid unaligned accesses */
#define HEAP_ALIGNMENT 4

/*
 * Input validation
 */

#ifndef FIRMWARE_MEM_MODE
    #error "FIRMWARE_MEM_MODE has not been configured"
#endif

#ifndef FIRMWARE_MEM0_BASE
    #error "FIRMWARE_MEM0_BASE has not been configured"
#endif

#ifndef FIRMWARE_MEM0_SIZE
    #error "FIRMWARE_MEM0_SIZE has not been configured"
#endif

#if ((FIRMWARE_MEM_MODE != FWK_MEM_MODE_SINGLE_REGION) && \
     (FIRMWARE_MEM_MODE != FWK_MEM_MODE_DUAL_REGION_RELOCATION) && \
     (FIRMWARE_MEM_MODE != FWK_MEM_MODE_DUAL_REGION_NO_RELOCATION))
    #error "FIRMWARE_MEM_MODE has been configured improperly"
#endif

#if FIRMWARE_MEM_MODE != FWK_MEM_MODE_SINGLE_REGION
    #ifndef FIRMWARE_MEM1_BASE
        #error "FIRMWARE_MEM1_BASE has not been configured"
    #endif

    #ifndef FIRMWARE_MEM1_SIZE
        #error "FIRMWARE_MEM1_SIZE has not been configured"
    #endif
#endif

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

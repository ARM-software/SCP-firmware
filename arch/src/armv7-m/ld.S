/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
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

#define STACK_ALIGNMENT 8

/*
 * Input validation
 */

#ifndef FIRMWARE_MEM_MODE
    #error "FIRMWARE_MEM_MODE has not been configured"
#endif

#ifndef FIRMWARE_STACK_SIZE
    #error "FIRMWARE_STACK_SIZE has not been configured"
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

/*
 * Calculate stack region in the data memory.
 */

#if FIRMWARE_MEM_MODE == FWK_MEM_MODE_SINGLE_REGION
    ASSERT(FIRMWARE_STACK_SIZE < FIRMWARE_MEM0_SIZE,
        "FIRMWARE_STACK_SIZE does not fit in MEM0")
    #define UNALIGNED_STACK_BASE \
        (FIRMWARE_MEM0_BASE + FIRMWARE_MEM0_SIZE - FIRMWARE_STACK_SIZE)
#else
    ASSERT(FIRMWARE_STACK_SIZE < FIRMWARE_MEM1_SIZE,
        "FIRMWARE_STACK_SIZE does not fit in MEM1")
    #define UNALIGNED_STACK_BASE \
        (FIRMWARE_MEM1_BASE + FIRMWARE_MEM1_SIZE - FIRMWARE_STACK_SIZE)
#endif

#define STACK_BASE \
    ( \
        ((UNALIGNED_STACK_BASE + STACK_ALIGNMENT - 1) / STACK_ALIGNMENT) \
            * STACK_ALIGNMENT \
    )

#define STACK_SIZE \
    (( \
        ((STACK_BASE + FIRMWARE_STACK_SIZE) / STACK_ALIGNMENT) \
            * STACK_ALIGNMENT \
    ) - STACK_BASE)

ASSERT(STACK_SIZE > 0, "FIRMWARE_STACK_SIZE is too small")

ENTRY(exception_reset)

MEMORY {
#if FIRMWARE_MEM_MODE == FWK_MEM_MODE_SINGLE_REGION
    /* Only one memory region with read, execute and write attributes */
    mem0 (rxw): ORIGIN = FIRMWARE_MEM0_BASE, LENGTH = FIRMWARE_MEM0_SIZE - \
                                                      FIRMWARE_STACK_SIZE
#else
    mem0 (rx):  ORIGIN = FIRMWARE_MEM0_BASE, LENGTH = FIRMWARE_MEM0_SIZE
    mem1 (rxw): ORIGIN = FIRMWARE_MEM1_BASE, LENGTH = FIRMWARE_MEM1_SIZE - \
                                                      FIRMWARE_STACK_SIZE
#endif

    stack (rw):  ORIGIN = STACK_BASE, LENGTH = STACK_SIZE
}

#if FIRMWARE_MEM_MODE == FWK_MEM_MODE_SINGLE_REGION
    REGION_ALIAS("mem1", mem0);
#endif

REGION_ALIAS("vtext", mem0);
REGION_ALIAS("vdata", mem1);
REGION_ALIAS("vbss", mem1);
REGION_ALIAS("vstack", stack);

#if FIRMWARE_MEM_MODE == FWK_MEM_MODE_DUAL_REGION_NO_RELOCATION
    REGION_ALIAS("ldata", mem1); /* .data is loaded into mem1 by a loader */
#else
    REGION_ALIAS("ldata", mem0); /* .data is loaded into mem1 by crt0 */
#endif

SECTIONS {
    .text : {
        KEEP(*(.exception))
        *(.text*)
        *(.rodata*)
    } > vtext

    .data : {
        . = ALIGN(4);
        *(.data*)
        . = ALIGN(4);
    } > vdata AT>ldata

    .bss : {
        . = ALIGN(4);
        *(.bss*)
        . = ALIGN(4);
    } > vbss

    .stack : {
        . = . + STACK_SIZE;
    } > vstack

    __text_start__ = LOADADDR(.text);
    __text_size__ = SIZEOF(.text);
    __text_end__ = __text_start__ + __text_size__;

    __stack_start__ = LOADADDR(.stack);
    __stack_size__ = SIZEOF(.stack);
    __stack_end__ = __stack_start__ + __stack_size__;

    __data_lma_start__ = LOADADDR(.data);
    __data_start__ = ADDR(.data);
    __data_size__ = SIZEOF(.data);

    __bss_start__ = ADDR(.bss);
    __bss_size__ = SIZEOF(.bss);
    __bss_end__ = __bss_start__ + __bss_size__;

    __heap_start__ = __bss_start__ + __bss_size__;
    __heap_end__ = __stack_start__;
    __heap_size__ = __heap_end__ - __heap_start__;

    __stack = __stack_end__;
}

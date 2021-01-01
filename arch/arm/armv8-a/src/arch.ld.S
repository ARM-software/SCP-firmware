/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2019-2021, Renesas Electronics Corporation. All rights
 * reserved.
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

#define ARCH_MEM_MODE_SINGLE_REGION             0
#define ARCH_MEM_MODE_DUAL_REGION_RELOCATION    1
#define ARCH_MEM_MODE_DUAL_REGION_NO_RELOCATION 2

#include <fmw_memory.h>

#define STACK_ALIGNMENT 8

/*
 * Input validation
 */

#ifndef FMW_MEM_MODE
    #error "FMW_MEM_MODE has not been configured"
#endif

#ifndef FMW_STACK_SIZE
    #error "FMW_STACK_SIZE has not been configured"
#endif

#ifndef FMW_MEM0_BASE
    #error "FMW_MEM0_BASE has not been configured"
#endif

#ifndef FMW_MEM0_SIZE
    #error "FMW_MEM0_SIZE has not been configured"
#endif

#if ((FMW_MEM_MODE != ARCH_MEM_MODE_SINGLE_REGION) && \
     (FMW_MEM_MODE != ARCH_MEM_MODE_DUAL_REGION_RELOCATION) && \
     (FMW_MEM_MODE != ARCH_MEM_MODE_DUAL_REGION_NO_RELOCATION))
    #error "FMW_MEM_MODE has been configured improperly"
#endif

#if FMW_MEM_MODE != ARCH_MEM_MODE_SINGLE_REGION
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

#if FMW_MEM_MODE == ARCH_MEM_MODE_SINGLE_REGION
    ASSERT(FMW_STACK_SIZE < FMW_MEM0_SIZE,
        "FMW_STACK_SIZE does not fit in MEM0")
    #define UNALIGNED_STACK_BASE \
        (FMW_MEM0_BASE + FMW_MEM0_SIZE - FMW_STACK_SIZE)
#else
    ASSERT(FMW_STACK_SIZE < FIRMWARE_MEM1_SIZE,
        "FMW_STACK_SIZE does not fit in MEM1")
    #define UNALIGNED_STACK_BASE \
        (FIRMWARE_MEM1_BASE + FIRMWARE_MEM1_SIZE - FMW_STACK_SIZE)
#endif

#define STACK_BASE \
    ( \
        ((UNALIGNED_STACK_BASE + STACK_ALIGNMENT - 1) / STACK_ALIGNMENT) \
            * STACK_ALIGNMENT \
    )

#define STACK_SIZE \
    (( \
        ((STACK_BASE + FMW_STACK_SIZE) / STACK_ALIGNMENT) \
            * STACK_ALIGNMENT \
    ) - STACK_BASE)

ASSERT(STACK_SIZE > 0, "FMW_STACK_SIZE is too small")

ENTRY(_entrypoint)

MEMORY {
#if FMW_MEM_MODE == ARCH_MEM_MODE_SINGLE_REGION
    /* Only one memory region with read, execute and write attributes */
    mem0 (rxw): ORIGIN = FMW_MEM0_BASE, LENGTH = FMW_MEM0_SIZE - \
                                                      FMW_STACK_SIZE
#else
    mem0 (rx):  ORIGIN = FMW_MEM0_BASE, LENGTH = FMW_MEM0_SIZE
    mem1 (rxw): ORIGIN = FIRMWARE_MEM1_BASE, LENGTH = FIRMWARE_MEM1_SIZE - \
                                                      FMW_STACK_SIZE
#endif
    stack (rw):  ORIGIN = STACK_BASE, LENGTH = STACK_SIZE
    sram (rxw): ORIGIN = ((0xE6302000)), LENGTH = (0x00001000)
}

SECTIONS {
    .text : {
        *(.vectors)
        *(.entrypoint)
        *(.text*)
        *(.rodata*)
        *(.note.gnu.build-id)
    } > mem0

    __text__ = .;

    __sram_copy_start__ = .;
    .system_ram : {
        __system_ram_start__ = .;
        *(.system_ram*)
        *iic_dvfs.o(.rodata)
        __system_ram_end__ = .;
    } > sram AT> mem0

    . = __text__ + SIZEOF(.system_ram);

    /*
     * Define a linker symbol to mark start of the RW memory area for this
     * image.
     */
    __RW_START__ = . ;

    .data : {
        . = ALIGN(4);
        *(.data*)
        . = ALIGN(4);
#if FMW_MEM_MODE == ARCH_MEM_MODE_SINGLE_REGION
    } > mem0 /* .data follows .text in mem0 */
#elif FMW_MEM_MODE == ARCH_MEM_MODE_DUAL_REGION_NO_RELOCATION
    } > mem1 /* .data is the first section in mem1 */
#elif FMW_MEM_MODE == ARCH_MEM_MODE_DUAL_REGION_RELOCATION
    } > mem1 AT>mem0  /* Run-time image is at mem1, but loaded from mem0 */
#else
    ASSERT(0, "Unrecognized FMW_MEM_MODE")
#endif

    .bss : {
        . = ALIGN(4);
        *(.bss*)
        . = ALIGN(4);
#if FMW_MEM_MODE == ARCH_MEM_MODE_SINGLE_REGION
    } > mem0  /* Run-time image is at mem1, but loaded from mem0 */
#else
    } > mem1 /* .bss follows .data in mem1 */
#endif

    .stack (NOLOAD) : {
        . = . + STACK_SIZE;
    } > stack
    __RW_END__ = .;

    /*
     * Define a linker symbol to mark end of the RW memory area for this
     * image.
     */

    __TEXT_START__ = LOADADDR(.text);
    __TEXT_SIZE__ = SIZEOF(.text);
    __TEXT_END__ = __TEXT_START__ + __TEXT_SIZE__;

    __STACK_START__ = LOADADDR(.stack);
    __STACK_SIZE__ = SIZEOF(.stack);
    __STACK_END__ = __STACK_START__ + __STACK_SIZE__;
    __STACK_TOP__ = __STACK_END__;
    __STACK_SP3_SIZE__ = 0x400;
    __STACK_SP0_TOP__ = __STACK_END__ - __STACK_SP3_SIZE__;

    __DATA_LMA_START__ = LOADADDR(.data);
    __DATA_START__ = ADDR(.data);
    __DATA_SIZE__ = SIZEOF(.data);

    __BSS_START__ = ADDR(.bss);
    __BSS_SIZE__ = SIZEOF(.bss);
    __BSS_END__ = __BSS_START__ + __BSS_SIZE__;

    __HEAP_START__ = __BSS_START__ + __BSS_SIZE__;
    __HEAP_END__ = __STACK_START__;
    __HEAP_SIZE__ = __HEAP_END__ - __HEAP_START__;
}

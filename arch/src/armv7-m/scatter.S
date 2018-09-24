/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Arm Compiler 6 scatter file.
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

#define FIRMWARE_MEM0_LIMIT (FIRMWARE_MEM0_BASE + FIRMWARE_MEM0_SIZE)

#if FIRMWARE_MEM_MODE == FWK_MEM_MODE_SINGLE_REGION
    #define FIRMWARE_MEM1_BASE +0
    #define FIRMWARE_MEM1_LIMIT (FIRMWARE_MEM0_BASE + FIRMWARE_MEM0_SIZE)
    #define FIRMWARE_MEM1_SIZE (FIRMWARE_MEM1_LIMIT - ImageLimit(ER_RODATA))
#else
    #define FIRMWARE_MEM1_LIMIT (FIRMWARE_MEM1_BASE + FIRMWARE_MEM1_SIZE)
#endif

LR_MEM0 FIRMWARE_MEM0_BASE FIRMWARE_MEM0_SIZE {
    ER_EXCEPTION +0 {
        *(.exceptions)
    }

    ER_RODATA +0 {
        *(+RO)
    }

    ER_RWDATA FIRMWARE_MEM1_BASE {
        *(+RW, +ZI)
    }

    ARM_LIB_STACKHEAP +0 EMPTY (FIRMWARE_MEM1_LIMIT - ImageLimit(ER_RWDATA)) { }
}

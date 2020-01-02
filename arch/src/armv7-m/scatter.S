/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Arm Compiler 6 scatter file.
 */

#include "scatter.h"

#if FIRMWARE_MEM_MODE == FWK_MEM_MODE_SINGLE_REGION
    /*
     * Single region memory layout:
     *  - MEM0 accepts:
     *      - Read-only sections
     *      - Read-write sections
     *      - Executable sections
     */

    #define FIRMWARE_X_BASE +0
    #define FIRMWARE_R_BASE +0
    #define FIRMWARE_W_BASE +0
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

    #define FIRMWARE_X_BASE +0
    #define FIRMWARE_R_BASE +0

    #define FIRMWARE_W_BASE FIRMWARE_MEM1_BASE
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

    #define FIRMWARE_X_BASE +0

    #define FIRMWARE_R_BASE FIRMWARE_MEM1_BASE
    #define FIRMWARE_W_BASE +0
#endif

#if FIRMWARE_MEM_MODE == FWK_MEM_MODE_SINGLE_REGION
    #define FIRMWARE_W_LIMIT FIRMWARE_MEM0_LIMIT
#else
    #define FIRMWARE_W_LIMIT FIRMWARE_MEM1_LIMIT
#endif

LR_FIRMWARE FIRMWARE_MEM0_BASE {
    ER_EXCEPTIONS FIRMWARE_X_BASE {
        *(.exceptions)
    }

    ER_TEXT +0 {
        *(+CODE)
    }

    ER_RODATA FIRMWARE_R_BASE {
        *(+CONST)
    }

    ER_DATA FIRMWARE_W_BASE {
        *(+DATA)
    }

    ER_BSS +0 {
        *(+BSS)
    }

    ARM_LIB_STACKHEAP +0 EMPTY (FIRMWARE_W_LIMIT - +0) { }
}

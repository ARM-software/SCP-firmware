/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Arm Compiler 6 scatter file.
 */

#include <arch_scatter.h>

#if FMW_MEM_MODE == ARCH_MEM_MODE_SINGLE_REGION
/*
 * Single region memory layout:
 *  - MEM0 accepts:
 *      - Read-only sections
 *      - Read-write sections
 *      - Executable sections
 */

#    define ARCH_X_BASE +0
#    define ARCH_R_BASE +0
#    define ARCH_W_BASE +0
#elif FMW_MEM_MODE == ARCH_MEM_MODE_DUAL_REGION_RELOCATION
/*
 * Dual region memory layout with initialized data relocation:
 *  - MEM0 accepts:
 *      - Read-only sections
 *      - Executable sections
 *
 *  - MEM1 accepts:
 *      - Read-write sections
 */

#    define ARCH_X_BASE +0
#    define ARCH_R_BASE +0

#    define ARCH_W_BASE FMW_MEM1_BASE
#elif FMW_MEM_MODE == ARCH_MEM_MODE_DUAL_REGION_NO_RELOCATION
    /*
     * Dual region memory layout without initialized data relocation:
     *  - MEM0 accepts:
     *      - Executable sections
     *
     *  - MEM1 accepts:
     *      - Read-only sections
     *      - Read-write sections
     */

#    define ARCH_X_BASE +0

#    define ARCH_R_BASE FMW_MEM1_BASE
#    define ARCH_W_BASE +0
#endif

#if FMW_MEM_MODE == ARCH_MEM_MODE_SINGLE_REGION
#    define ARCH_W_LIMIT ARCH_MEM0_LIMIT
#else
#    define ARCH_W_LIMIT ARCH_MEM1_LIMIT
#endif

LR_FIRMWARE FMW_MEM0_BASE {
    ER_EXCEPTIONS ARCH_X_BASE {
        *(.exceptions)
    }

    ER_TEXT +0 {
        *(+CODE)
    }

    ER_RODATA ARCH_R_BASE {
        *(+CONST)
    }

    ER_DATA ARCH_W_BASE {
        *(+DATA)
    }

    ER_BSS +0 {
        *(+BSS)
    }

    ARM_LIB_STACKHEAP +0 EMPTY (ARCH_W_LIMIT - +0) { }
}

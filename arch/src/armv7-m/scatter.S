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

#if FMW_MEM_MODE == FWK_MEM_MODE_SINGLE_REGION
/*
 * Single region memory layout:
 *  - MEM0 accepts:
 *      - Read-only sections
 *      - Read-write sections
 *      - Executable sections
 */

#    define FMW_X_BASE +0
#    define FMW_R_BASE +0
#    define FMW_W_BASE +0
#elif FMW_MEM_MODE == FWK_MEM_MODE_DUAL_REGION_RELOCATION
/*
 * Dual region memory layout with initialized data relocation:
 *  - MEM0 accepts:
 *      - Read-only sections
 *      - Executable sections
 *
 *  - MEM1 accepts:
 *      - Read-write sections
 */

#    define FMW_X_BASE +0
#    define FMW_R_BASE +0

#    define FMW_W_BASE FMW_MEM1_BASE
#elif FMW_MEM_MODE == FWK_MEM_MODE_DUAL_REGION_NO_RELOCATION
    /*
     * Dual region memory layout without initialized data relocation:
     *  - MEM0 accepts:
     *      - Executable sections
     *
     *  - MEM1 accepts:
     *      - Read-only sections
     *      - Read-write sections
     */

#    define FMW_X_BASE +0

#    define FMW_R_BASE FMW_MEM1_BASE
#    define FMW_W_BASE +0
#endif

#if FMW_MEM_MODE == FWK_MEM_MODE_SINGLE_REGION
#    define FMW_W_LIMIT FMW_MEM0_LIMIT
#else
#    define FMW_W_LIMIT FMW_MEM1_LIMIT
#endif

LR_FIRMWARE FMW_MEM0_BASE {
    ER_EXCEPTIONS FMW_X_BASE {
        *(.exceptions)
    }

    ER_TEXT +0 {
        *(+CODE)
    }

    ER_RODATA FMW_R_BASE {
        *(+CONST)
    }

    ER_DATA FMW_W_BASE {
        *(+DATA)
    }

    ER_BSS +0 {
        *(+BSS)
    }

    ARM_LIB_STACKHEAP +0 EMPTY (FMW_W_LIMIT - +0) { }
}

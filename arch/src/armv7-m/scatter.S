/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Arm Compiler 6 scatter file.
 */

#include "scatter.h"

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

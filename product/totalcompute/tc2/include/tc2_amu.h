/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TC2_AMU_H
#define TC2_AMU_H

enum hunter_amu_counter {
    HUNTER_AMEVCNTR0_CORE,
    HUNTER_AMEVCNTR0_CONST,
    HUNTER_AMEVCNTR0_INST_RET,
    HUNTER_AMEVCNTR0_MEM_STALL,
    HUNTER_AMEVCNTR1_AUX0,
    HUNTER_AMEVCNTR1_AUX1,
    HUNTER_AMEVCNTR1_AUX2,
    HUNTER_AMEVCNTR_COUNT,
};

#endif /* TC2_AMU_H */

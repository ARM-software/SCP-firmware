/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TC2_AMU_H
#define TC2_AMU_H

enum tc2_core_amu_counter {
    AMEVCNTR0_CORE,
    AMEVCNTR0_CONST,
    AMEVCNTR0_INST_RET,
    AMEVCNTR0_MEM_STALL,
    AMEVCNTR1_AUX0,
    AMEVCNTR1_AUX1,
    AMEVCNTR1_AUX2,
    AMEVCNTR_COUNT,
};

#endif /* TC2_AMU_H */

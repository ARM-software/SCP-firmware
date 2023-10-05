/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_mmap.h"

#include <tc2_amu.h>

#include <mod_amu_mmap.h>

#include <interface_amu.h>

#include <fwk_element.h>
#include <fwk_module.h>

enum cpu_idx {
    CORE0_IDX,
    CORE1_IDX,
    CORE2_IDX,
    CORE3_IDX,
    CORE4_IDX,
    CORE5_IDX,
    CORE6_IDX,
    CORE7_IDX,
    CORE_COUNT_IDX,
};

uint32_t tc2_core_amu_layout[] = {
    [AMEVCNTR0_CORE] = 0,
    [AMEVCNTR0_CONST] = sizeof(uint64_t) * 1,
    [AMEVCNTR0_INST_RET] = sizeof(uint64_t) * 2,
    [AMEVCNTR0_MEM_STALL] = sizeof(uint64_t) * 3,
    [AMEVCNTR1_AUX0] = SCP_AMU_AMEVCNTR1_OFFSET,
    [AMEVCNTR1_AUX1] = SCP_AMU_AMEVCNTR1_OFFSET + sizeof(uint64_t) * 1,
    [AMEVCNTR1_AUX2] = SCP_AMU_AMEVCNTR1_OFFSET + sizeof(uint64_t) * 2,
};

static struct mod_core_element_config core_config [CORE_COUNT_IDX] = {
    [CORE0_IDX] = {
        .counters_base_addr = (uint64_t *)SCP_AMU_CORE_BASE(CORE0_IDX),
        .counters_offsets = tc2_core_amu_layout,
    },
    [CORE1_IDX] = {
        .counters_base_addr = (uint64_t *)SCP_AMU_CORE_BASE(CORE1_IDX),
        .counters_offsets = tc2_core_amu_layout,
    },
    [CORE2_IDX] = {
        .counters_base_addr = (uint64_t *)SCP_AMU_CORE_BASE(CORE2_IDX),
        .counters_offsets = tc2_core_amu_layout,
    },
    [CORE3_IDX] = {
        .counters_base_addr = (uint64_t *)SCP_AMU_CORE_BASE(CORE3_IDX),
        .counters_offsets = tc2_core_amu_layout,
    },
    [CORE4_IDX] = {
        .counters_base_addr = (uint64_t *)SCP_AMU_CORE_BASE(CORE4_IDX),
        .counters_offsets = tc2_core_amu_layout,
    },
    [CORE5_IDX] = {
        .counters_base_addr = (uint64_t *)SCP_AMU_CORE_BASE(CORE5_IDX),
        .counters_offsets = tc2_core_amu_layout,
    },
    [CORE6_IDX] = {
        .counters_base_addr = (uint64_t *)SCP_AMU_CORE_BASE(CORE6_IDX),
        .counters_offsets = tc2_core_amu_layout,
    },
    [CORE7_IDX] = {
        .counters_base_addr = (uint64_t *)SCP_AMU_CORE_BASE(CORE7_IDX),
        .counters_offsets = tc2_core_amu_layout,
    }
};

static const struct fwk_element element_table[] = {
    [CORE0_IDX] = {
        .name = "Core0 AMU base",
        .data = &core_config[CORE0_IDX],
        .sub_element_count = AMEVCNTR_COUNT,
    },
    [CORE1_IDX] = {
        .name = "Core1 AMU base",
        .data = &core_config[CORE1_IDX],
        .sub_element_count = AMEVCNTR_COUNT,
    },
    [CORE2_IDX] = {
        .name = "Core2 AMU base",
        .data = &core_config[CORE2_IDX],
        .sub_element_count = AMEVCNTR_COUNT,
    },
    [CORE3_IDX] = {
        .name = "Core3 AMU base",
        .data = &core_config[CORE3_IDX],
        .sub_element_count = AMEVCNTR_COUNT,
    },
    [CORE4_IDX] = {
        .name = "Core4 AMU base",
        .data = &core_config[CORE4_IDX],
        .sub_element_count = AMEVCNTR_COUNT,
    },
    [CORE5_IDX] = {
        .name = "Core5 AMU base",
        .data = &core_config[CORE5_IDX],
        .sub_element_count = AMEVCNTR_COUNT,
    },
    [CORE6_IDX] = {
        .name = "Core6 AMU base",
        .data = &core_config[CORE6_IDX],
        .sub_element_count = AMEVCNTR_COUNT,
    },
    [CORE7_IDX] = {
        .name = "Core7 AMU base",
        .data = &core_config[CORE7_IDX],
        .sub_element_count = AMEVCNTR_COUNT,
    },
    [CORE_COUNT_IDX] = {0},
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_amu_mmap = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};

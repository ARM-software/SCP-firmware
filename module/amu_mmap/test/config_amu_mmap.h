/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEST_CONFIG_AMU_MMAP_H
#define TEST_CONFIG_AMU_MMAP_H

#include <interface_amu.h>

#include <mod_amu_mmap.h>

enum core_idx {
    CORE0_IDX,
    CORE1_IDX,
    CORE_COUNT,
};

/*!
 * \brief AMU Counter names per core type A
 */
enum core_a_amu_counter {
    COREA_CORE,
    COREA_CONST,
    COREA_INST_RET,
    COREA_MEM_STALL,
    COREA_AUX0,
    COREA_AUX1,
    COREA_AUX2,
    COREA_AUX3,
    COREA_AUX4,
    COREA_AUX5,
    NUM_OF_COREA_COUNTERS,
};

/*!
 * \brief AMU Counter names per core type B
 */
enum core_b_amu_counter {
    COREB_CORE,
    COREB_CONST,
    COREB_INST_RET,
    COREB_MEM_STALL,
    COREB_AUX0,
    COREB_AUX1,
    COREB_AUX2,
    COREB_AUX3,
    NUM_OF_COREB_COUNTERS,
};

/* Testing counters are consecutive */
uint64_t amu_counters[CORE_COUNT][NUM_OF_COREA_COUNTERS];

/* Core A layout (counters offsets) */
uint32_t core_a_layout[] = {
    [COREA_CORE] = 0,
    [COREA_CONST] = sizeof(uint64_t) * COREA_CONST,
    [COREA_INST_RET] = sizeof(uint64_t) * COREA_INST_RET,
    [COREA_MEM_STALL] = sizeof(uint64_t) * COREA_MEM_STALL,
    [COREA_AUX0] = sizeof(uint64_t) * COREA_AUX0,
    [COREA_AUX1] = sizeof(uint64_t) * COREA_AUX1,
    [COREA_AUX2] = sizeof(uint64_t) * COREA_AUX2,
    [COREA_AUX3] = sizeof(uint64_t) * COREA_AUX3,
    [COREA_AUX4] = sizeof(uint64_t) * COREA_AUX4,
    [COREA_AUX5] = sizeof(uint64_t) * COREA_AUX5,
};

/* Core B layout (counters offsets) */
uint32_t core_b_layout[] = {
    [COREB_CORE] = 0,
    [COREB_CONST] = sizeof(uint64_t) * COREB_CONST,
    [COREB_INST_RET] = sizeof(uint64_t) * COREB_INST_RET,
    [COREB_MEM_STALL] = sizeof(uint64_t) * COREB_MEM_STALL,
    [COREB_AUX0] = sizeof(uint64_t) * COREB_AUX0,
    [COREB_AUX1] = sizeof(uint64_t) * COREB_AUX1,
    [COREB_AUX2] = sizeof(uint64_t) * COREB_AUX2,
    [COREB_AUX3] = sizeof(uint64_t) * COREB_AUX3,
};

static struct mod_core_element_config core_config [CORE_COUNT] = {
    [CORE0_IDX] = {
        .counters_base_addr = amu_counters[CORE0_IDX],
        .counters_offsets = core_a_layout,
    },
    [CORE1_IDX] = {
        .counters_base_addr = amu_counters[CORE1_IDX],
        .counters_offsets = core_b_layout,
    },
};

static const struct fwk_element element_table[CORE_COUNT] = {
    [CORE0_IDX] = {
        .name = "Core0 AMU counters",
        .data = &core_config[CORE0_IDX],
        .sub_element_count = NUM_OF_COREA_COUNTERS,
    },
    [CORE1_IDX] = {
        .name = "Core1 AMU counters",
        .data = &core_config[CORE1_IDX],
        .sub_element_count = NUM_OF_COREB_COUNTERS,
    },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_amu_mmap = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};

#endif /* TEST_CONFIG_AMU_MMAP_H */

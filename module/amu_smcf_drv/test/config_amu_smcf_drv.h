/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEST_CONFIG_AMU_SMCF_DRV_H
#define TEST_CONFIG_AMU_SMCF_DRV_H

#include <mod_amu_smcf_drv.h>

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#define COREA_TYPE_AUX_MEMORY_GAP UINT32_C(0x0)
#define COREB_TYPE_AUX_MEMORY_GAP UINT32_C(0xE0)

#define AMU_DATA_MAX_SZ 72

enum amu_core_set {
    AMU_COUNTERS_CORE0,
    AMU_COUNTERS_CORE1,
    AMU_COUNTERS_CORE2,
    NUM_OF_CORES,
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

/* Core A layout (counters offsets) */
uint32_t core_a_layout[] = {
    [COREA_CORE] = 0,
    [COREA_CONST] = sizeof(uint64_t) * COREA_CONST,
    [COREA_INST_RET] = sizeof(uint64_t) * COREA_INST_RET,
    [COREA_MEM_STALL] = sizeof(uint64_t) * COREA_MEM_STALL,
    [COREA_AUX0] = COREA_TYPE_AUX_MEMORY_GAP + sizeof(uint64_t) * COREA_AUX0,
    [COREA_AUX1] = COREA_TYPE_AUX_MEMORY_GAP + sizeof(uint64_t) * COREA_AUX1,
    [COREA_AUX2] = COREA_TYPE_AUX_MEMORY_GAP + sizeof(uint64_t) * COREA_AUX2,
    [COREA_AUX3] = COREA_TYPE_AUX_MEMORY_GAP + sizeof(uint64_t) * COREA_AUX3,
    [COREA_AUX4] = COREA_TYPE_AUX_MEMORY_GAP + sizeof(uint64_t) * COREA_AUX4,
    [COREA_AUX5] = COREA_TYPE_AUX_MEMORY_GAP + sizeof(uint64_t) * COREA_AUX5,
};

/* Core B layout (counters offsets) */
uint32_t core_b_layout[] = {
    [COREB_CORE] = 0,
    [COREB_CONST] = sizeof(uint64_t) * COREB_CONST,
    [COREB_INST_RET] = sizeof(uint64_t) * COREB_INST_RET,
    [COREB_MEM_STALL] = sizeof(uint64_t) * COREB_MEM_STALL,
    [COREB_AUX0] = COREB_TYPE_AUX_MEMORY_GAP + sizeof(uint64_t) * COREB_AUX0,
    [COREB_AUX1] = COREB_TYPE_AUX_MEMORY_GAP + sizeof(uint64_t) * COREB_AUX1,
    [COREB_AUX2] = COREB_TYPE_AUX_MEMORY_GAP + sizeof(uint64_t) * COREB_AUX2,
    [COREB_AUX3] = COREB_TYPE_AUX_MEMORY_GAP + sizeof(uint64_t) * COREB_AUX3,
};

/* CPU layout CPU0(TYPE A) CPU1(TYPE B)  CPU2(TYPE A) */
static const struct amu_smcf_drv_element_config core_config_table[NUM_OF_CORES] = {
    [AMU_COUNTERS_CORE0] = {
        .smcf_mli_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_SMCF, AMU_COUNTERS_CORE0, 0),
        .counter_offsets = core_a_layout,
    },
    [AMU_COUNTERS_CORE1] = {
        .smcf_mli_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_SMCF, AMU_COUNTERS_CORE1, 0),
        .counter_offsets = core_b_layout,
    },
    [AMU_COUNTERS_CORE2] = {
        .smcf_mli_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_SMCF, AMU_COUNTERS_CORE2, 0),
        .counter_offsets = core_a_layout,
    },
};

static const struct fwk_element test_element_table[NUM_OF_CORES] = {
    [AMU_COUNTERS_CORE0] = {
        .name = "Core0 AMU element config",
        .data = &core_config_table[AMU_COUNTERS_CORE0],
        .sub_element_count = NUM_OF_COREA_COUNTERS,
    },
    [AMU_COUNTERS_CORE1] = {
        .name = "Core1 AMU element config",
        .data = &core_config_table[AMU_COUNTERS_CORE1],
        .sub_element_count = NUM_OF_COREB_COUNTERS,
    },
    [AMU_COUNTERS_CORE2] = {
        .name = "Core2 AMU element config",
        .data = &core_config_table[AMU_COUNTERS_CORE2],
        .sub_element_count = NUM_OF_COREA_COUNTERS,
    },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return test_element_table;
}

struct fwk_module_config config_amu_smcf_drv = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};

#endif /* TEST_CONFIG_AMU_SMCF_DRV_H */

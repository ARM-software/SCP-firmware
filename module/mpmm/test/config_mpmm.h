/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <mpmm.h>

#include <mod_mpmm.h>

#include <fwk_macros.h>
#include <fwk_module_idx.h>

#define NUM_OF_MPMM_GEARS  1
#define DEFAULT_PERF_LIMIT 0xFFFFFFFF

enum amu_counter {
    AMU_CORE,
    AMU_CONST,
    AMU_INST_RET,
    AMU_MEM_STALL,
    AMU_AUX0,
    AMU_AUX1,
    AMU_AUX2,
    AMU_COUNT,
};

enum cpu_idx {
    CORE0_IDX,
    CORE1_IDX,
    CORE_IDX_COUNT,
};

enum fake_pct_table_idx {
    FAKE_PCT_TABLE_DEFAULT,
    FAKE_PCT_TABLE_NO_THRESHOLD_BITMAP,
    FAKE_PCT_TABLE_COUNT,
};

enum fake_mpmm_domain_idx {
    MPMM_DOM_DEFAULT,
    MPMM_DOM_MAX_THRESHOLD_COUNT,
    MPMM_DOM_NUM_THRESHOLD_NOT_EQUAL,
    MPMM_DOM_TWO_THRESHOLD_COUNTER,
    MPMM_DOM_COUNT,
};

static struct mpmm_reg fake_mpmm_reg[CORE_IDX_COUNT] = {
    [CORE0_IDX] = {
        .PPMCR = (NUM_OF_MPMM_GEARS << MPMM_PPMCR_NUM_GEARS_POS),
        .MPMMCR = 0,
    },
    [CORE1_IDX] = {
        .PPMCR = (NUM_OF_MPMM_GEARS << MPMM_PPMCR_NUM_GEARS_POS),
        .MPMMCR = 0,
    },
};

static const struct mod_mpmm_core_config fake_core_config[CORE_IDX_COUNT] = {
    [CORE0_IDX] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, 0),
        .mpmm_reg_base = (uintptr_t)&fake_mpmm_reg[CORE0_IDX],
        .core_starts_online = true,
        .base_aux_counter_id = FWK_ID_SUB_ELEMENT_INIT(
            FWK_MODULE_IDX_AMU_MMAP, CORE0_IDX, AMU_AUX0),
    },
    [CORE1_IDX] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, 0),
        .mpmm_reg_base = (uintptr_t)&fake_mpmm_reg[CORE1_IDX],
        .core_starts_online = true,
        .base_aux_counter_id = FWK_ID_SUB_ELEMENT_INIT(
            FWK_MODULE_IDX_AMU_MMAP, CORE1_IDX, AMU_AUX0),
    },
};

static struct mod_mpmm_pct_table fake_pct_table[FAKE_PCT_TABLE_COUNT] = {
    [FAKE_PCT_TABLE_DEFAULT] = {
        .cores_online = 1,
        .default_perf_limit = DEFAULT_PERF_LIMIT,
        .num_perf_limits = 2,
        .threshold_perf = {
            {
                .threshold_bitmap = 0,
                .perf_limit = 0,
            },
            {
                .threshold_bitmap = 0,
                .perf_limit = 0,
            },
        },
    },
    [FAKE_PCT_TABLE_NO_THRESHOLD_BITMAP] = {
        .cores_online = 1,
        .default_perf_limit = 1,
        .num_perf_limits = 1,
        .threshold_perf = {
            {
                .threshold_bitmap = 0,
                .perf_limit = 0,
            },
        },
    },
};

static struct mod_mpmm_domain_config fake_dom_conf[MPMM_DOM_COUNT] = {
    [MPMM_DOM_DEFAULT] = {
        .core_config = fake_core_config,
        .pct = fake_pct_table,
        .pct_size = FWK_ARRAY_SIZE(fake_pct_table),
        .btc = 1,
        .num_threshold_counters = 1,
        .perf_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DVFS, 0),
    },
    [MPMM_DOM_MAX_THRESHOLD_COUNT] = {
        .core_config = fake_core_config,
        .pct = fake_pct_table,
        .pct_size = FWK_ARRAY_SIZE(fake_pct_table),
        .btc = 0,
        .num_threshold_counters = (MPMM_MAX_THRESHOLD_COUNT + 1),
        .perf_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DVFS, 0),
    },
    [MPMM_DOM_NUM_THRESHOLD_NOT_EQUAL] = {
        .core_config = fake_core_config,
        .pct = fake_pct_table,
        .pct_size = FWK_ARRAY_SIZE(fake_pct_table),
        .btc = 0,
        .num_threshold_counters = (NUM_OF_MPMM_GEARS + 1),
        .perf_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DVFS, 0),
    },
    [MPMM_DOM_TWO_THRESHOLD_COUNTER] = {
        .core_config = fake_core_config,
        .pct = fake_pct_table,
        .pct_size = FWK_ARRAY_SIZE(fake_pct_table),
        .btc = 0,
        .num_threshold_counters = 2,
        .perf_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DVFS, 0),
    },
};

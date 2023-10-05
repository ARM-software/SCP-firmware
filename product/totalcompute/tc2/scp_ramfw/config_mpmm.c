/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_mmap.h"
#include "tc2_amu.h"
#include "tc2_dvfs.h"
#include "tc2_timer.h"
#include "tc_core.h"

#include <mod_amu_mmap.h>
#include <mod_mpmm.h>

#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

enum cpu_idx {
    CORE0_IDX,
    CORE1_IDX,
    CORE2_IDX,
    CORE3_IDX,
    CORE4_IDX,
    CORE5_IDX,
    CORE6_IDX,
    CORE7_IDX
};

static struct mod_mpmm_pct_table cortex_a720_pct[4] = {
    {
        .cores_online = 4,
        .default_perf_limit = 1419 * 1000000UL,
        .num_perf_limits = 3,
        .threshold_perf = {
            {
                .threshold_bitmap = 0x2222,
                .perf_limit = 1419 * 1000000UL,
            },
            {
                .threshold_bitmap = 0x2211,
                .perf_limit = 1893 * 1000000UL,
            },
            {
                .threshold_bitmap = 0x1111,
                .perf_limit = 2271 * 1000000UL,
            },
        },
    },
    {
        .cores_online = 3,
        .default_perf_limit = 1893 * 1000000UL,
        .num_perf_limits = 2,
        .threshold_perf = {
            {
                .threshold_bitmap = 0x222,
                .perf_limit = 1893 * 1000000UL,
            },
            {
                .threshold_bitmap = 0x111,
                .perf_limit = 2271 * 1000000UL,
            },
        },
    },
    {
        .cores_online = 2,
        .default_perf_limit = 2271 * 1000000UL,
        .num_perf_limits = 1,
        .threshold_perf = {
            {
                .threshold_bitmap = 0x22,
                .perf_limit = 2271 * 1000000UL,
            },
        },
    },
    {
        .cores_online = 1,
        .default_perf_limit = 2650 * 1000000UL,
        .num_perf_limits = 1,
        .threshold_perf = {
            {
                .threshold_bitmap = 0x2,
                .perf_limit = 2650 * 1000000UL,
            },
        },
    },
};

static const struct mod_mpmm_core_config cortex_a720_core_config[4] = {
    [0] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE4_IDX),
        .mpmm_reg_base = SCP_MPMM_CORE_BASE(CORE4_IDX),
        .core_starts_online = false,
        .base_aux_counter_id = FWK_ID_SUB_ELEMENT_INIT(
            FWK_MODULE_IDX_AMU_MMAP, CORE4_IDX, AMEVCNTR1_AUX0),
    },
    [1] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE5_IDX),
        .mpmm_reg_base = SCP_MPMM_CORE_BASE(CORE5_IDX),
        .core_starts_online = false,
        .base_aux_counter_id = FWK_ID_SUB_ELEMENT_INIT(
            FWK_MODULE_IDX_AMU_MMAP, CORE5_IDX, AMEVCNTR1_AUX0),
    },
    [2] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE6_IDX),
        .mpmm_reg_base = SCP_MPMM_CORE_BASE(CORE6_IDX),
        .core_starts_online = false,
        .base_aux_counter_id = FWK_ID_SUB_ELEMENT_INIT(
            FWK_MODULE_IDX_AMU_MMAP, CORE6_IDX, AMEVCNTR1_AUX0),
    },
    [3] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE7_IDX),
        .mpmm_reg_base = SCP_MPMM_CORE_BASE(CORE7_IDX),
        .core_starts_online = false,
        .base_aux_counter_id = FWK_ID_SUB_ELEMENT_INIT(
            FWK_MODULE_IDX_AMU_MMAP, CORE7_IDX, AMEVCNTR1_AUX0),
    },
};

static const struct mod_mpmm_domain_config cortex_a720_domain_conf[2] = {
    [0] = {
        .perf_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_DVFS, DVFS_ELEMENT_IDX_CORTEX_A720),
        .pct = cortex_a720_pct,
        .pct_size = FWK_ARRAY_SIZE(cortex_a720_pct),
        .btc = 10,
        .num_threshold_counters = 3,
        .core_config = cortex_a720_core_config,
    },
    [1] = {0},
};

static const struct fwk_element element_table[2] = {
    [0] = {
        .name = "MPMM_CORTEX_A720_ELEM",
        .sub_element_count = 1,
        .data = cortex_a720_domain_conf,
    },
    [1] = { 0 },
};

static const struct fwk_element *mpmm_get_element_table(fwk_id_t module_id)
{
    return element_table;
}
const struct fwk_module_config config_mpmm = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(mpmm_get_element_table),
    .data = (const void *)(&(fwk_id_t)FWK_ID_API_INIT(
        FWK_MODULE_IDX_AMU_MMAP,
        MOD_AMU_MMAP_API_IDX_AMU)),
};

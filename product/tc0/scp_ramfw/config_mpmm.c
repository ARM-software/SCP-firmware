/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_css_mmap.h"
#include "tc0_core.h"
#include "tc0_dvfs.h"
#include "tc0_timer.h"

#include <mod_mpmm.h>

#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

enum core_pd_idx {
    CORE0_IDX,
    CORE1_IDX,
    CORE2_IDX,
    CORE3_IDX,
    CORE4_IDX,
    CORE5_IDX,
    CORE6_IDX,
    CORE7_IDX
};

#if defined(PLATFORM_VARIANT) && (PLATFORM_VARIANT == TC0_VARIANT_STD)
static struct mod_mpmm_pct_table k_pct[] = {
    { .cores_online = 4,
      .default_perf_limit = 1153 * 1000000UL,
      .num_perf_limits = 3,
      .threshold_perf = { {
                              .threshold_bitmap = 0x2222,
                              .perf_limit = 1153 * 1000000UL,
                          },
                          {
                              .threshold_bitmap = 0x2211,
                              .perf_limit = 1537 * 1000000UL,
                          },
                          {
                              .threshold_bitmap = 0x1111,
                              .perf_limit = 1844 * 1000000UL,
                          },
                        } },
    { .cores_online = 3,
      .default_perf_limit = 1844 * 1000000UL,
      .num_perf_limits = 2,
      .threshold_perf = { {
                              .threshold_bitmap = 0x222,
                              .perf_limit = 1844 * 1000000UL,
                          },
                          {
                              .threshold_bitmap = 0x111,
                              .perf_limit = 2152 * 1000000UL,
                          },
                        } },
    { .cores_online = 2,
      .default_perf_limit = 2152 * 1000000UL,
      .num_perf_limits = 1,
      .threshold_perf = { {
                              .threshold_bitmap = 0x22,
                              .perf_limit = 2152 * 1000000UL,
                          },
                        } },

   { .cores_online = 1,
     .default_perf_limit = 2152 * 1000000UL,
     .num_perf_limits = 1,
     .threshold_perf = { {
                              .threshold_bitmap = 0x2,
                              .perf_limit = 2152 * 1000000UL,
                          },
                        } },

};

static struct mod_mpmm_pct_table m_pct[] = {
    { .cores_online = 3,
      .default_perf_limit = 1419 * 1000000UL,
      .num_perf_limits = 3,
      .threshold_perf = { {
                              .threshold_bitmap = 0x222,
                              .perf_limit = 1419 * 1000000UL,
                          },
                          {
                              .threshold_bitmap = 0x211,
                              .perf_limit = 1893 * 1000000UL,
                          },
                          {
                              .threshold_bitmap = 0x110,
                              .perf_limit = 2271 * 1000000UL,
                          },
                        } },
    { .cores_online = 2,
      .default_perf_limit = 2271 * 1000000UL,
      .num_perf_limits = 2,
      .threshold_perf = { {
                              .threshold_bitmap = 0x22,
                              .perf_limit = 2271 * 1000000UL,
                          },
                          {
                              .threshold_bitmap = 0x11,
                              .perf_limit = 2650 * 1000000UL,
                          },
                        } },
    { .cores_online = 1,
      .default_perf_limit = 2650 * 1000000UL,
      .num_perf_limits = 1,
      .threshold_perf = { {
                              .threshold_bitmap = 0x2,
                              .perf_limit = 2650 * 1000000UL,
                          },
                        } },
};
#endif

static struct mod_mpmm_pct_table m_elp_pct[] = {
    { .cores_online = 1,
      .default_perf_limit = 2612 * 1000000UL,
      .num_perf_limits = 2,
      .threshold_perf = { {
                              .threshold_bitmap = 0x2,
                              .perf_limit = 2612 * 1000000UL,
                          },
                          {
                              .threshold_bitmap = 0x1,
                              .perf_limit = 3047 * 1000000UL,
                          },
                        } },
};

#if defined(PLATFORM_VARIANT) && (PLATFORM_VARIANT == TC0_VARIANT_STD)
static const struct mod_mpmm_core_config k_core_config[] = {
    [0] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE0_IDX),
        .mpmm_reg_base = SCP_MPMM_CORE_BASE(CORE0_IDX),
        .amu_aux_reg_base = SCP_AMU_AMEVCNTR1X(CORE0_IDX),
        .core_starts_online = true,
        },
    [1] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE1_IDX),
        .mpmm_reg_base = SCP_MPMM_CORE_BASE(CORE1_IDX),
        .amu_aux_reg_base = SCP_AMU_AMEVCNTR1X(CORE1_IDX),
        .core_starts_online = false,
        },
    [2] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE2_IDX),
        .mpmm_reg_base = SCP_MPMM_CORE_BASE(CORE2_IDX),
        .amu_aux_reg_base = SCP_AMU_AMEVCNTR1X(CORE2_IDX),
        .core_starts_online = false,
        },
    [3] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE3_IDX),
        .mpmm_reg_base = SCP_MPMM_CORE_BASE(CORE3_IDX),
        .amu_aux_reg_base = SCP_AMU_AMEVCNTR1X(CORE3_IDX),
        .core_starts_online = false,
        },
};

static const struct mod_mpmm_core_config m_core_config[] = {
    [0] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE4_IDX),
        .mpmm_reg_base = SCP_MPMM_CORE_BASE(CORE4_IDX),
        .amu_aux_reg_base = SCP_AMU_AMEVCNTR1X(CORE4_IDX),
        .core_starts_online = false,
        },
    [1] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE5_IDX),
        .mpmm_reg_base = SCP_MPMM_CORE_BASE(CORE5_IDX),
        .amu_aux_reg_base = SCP_AMU_AMEVCNTR1X(CORE5_IDX),
        .core_starts_online = false,
        },
    [2] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE6_IDX),
        .mpmm_reg_base = SCP_MPMM_CORE_BASE(CORE6_IDX),
        .amu_aux_reg_base = SCP_AMU_AMEVCNTR1X(CORE6_IDX),
        .core_starts_online = false,
        },
};
#endif

static const struct mod_mpmm_core_config m_elp_core_config[] = {
    [0] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE7_IDX),
        .mpmm_reg_base = SCP_MPMM_CORE_BASE(CORE7_IDX),
        .amu_aux_reg_base = SCP_AMU_AMEVCNTR1X(CORE7_IDX),
        .core_starts_online = false,
        },
};

#if defined(PLATFORM_VARIANT) && (PLATFORM_VARIANT == TC0_VARIANT_STD)
static const struct mod_mpmm_domain_config k_domain_conf[] = {
    [0] = {
        .perf_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_DVFS,
            DVFS_ELEMENT_IDX_KLEIN),
        .pct = k_pct,
        .pct_size = FWK_ARRAY_SIZE(k_pct),
        .btc = 10,
        .num_threshold_counters = 3,
        .core_config = k_core_config,
    },
    [1] = {0},
};

static const struct mod_mpmm_domain_config m_domain_conf[] = {
    [0] = {
        .perf_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_DVFS,
            DVFS_ELEMENT_IDX_MATTERHORN),
        .pct = m_pct,
        .pct_size = FWK_ARRAY_SIZE(m_pct),
        .btc = 10,
        .num_threshold_counters = 3,
        .core_config = m_core_config,
    },
    [1] = {0},
};
#endif

static const struct mod_mpmm_domain_config m_elp_domain_conf[] = {
    [0] = {
        .perf_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_DVFS,
            DVFS_ELEMENT_IDX_MATTERHORN_ELP_ARM),
        .pct = m_elp_pct,
        .pct_size = FWK_ARRAY_SIZE(m_elp_pct),
        .btc = 10,
        .num_threshold_counters = 3,
        .core_config = m_elp_core_config,
    },
    [1] = {0},
};

static const struct fwk_element element_table[] = {
#if defined(PLATFORM_VARIANT) && (PLATFORM_VARIANT == TC0_VAR_EXPERIMENT_POWER)
    [0] = {
        .name = "MPMM_MATTERHORN_ELP_ARM_ELEM",
        .sub_element_count = 1,
        .data = m_elp_domain_conf,
    },
    [1] = { 0 },
#else
    [0] = {
        .name = "MPMM_KLEIN_ELEM",
        .sub_element_count = 4,
        .data = k_domain_conf,
    },
    [1] = {
        .name = "MPMM_MATTERHORN_ELEM",
        .sub_element_count = 3,
        .data = m_domain_conf,
    },
    [2] = {
        .name = "MPMM_MATTERHORN_ELP_ARM_ELEM",
        .sub_element_count = 1,
        .data = m_elp_domain_conf,
    },
    [3] = { 0 },
#endif
};

static const struct fwk_element *mpmm_get_element_table(fwk_id_t module_id)
{
    return element_table;
}
const struct fwk_module_config config_mpmm = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(mpmm_get_element_table),
};

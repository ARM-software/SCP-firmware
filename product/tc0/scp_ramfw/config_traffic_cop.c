/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_css_mmap.h"
#include "scp_software_mmap.h"
#include "tc0_core.h"
#include "tc0_dvfs.h"
#include "tc0_timer.h"

#include <mod_traffic_cop.h>

#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

enum core_pd_idx {
    CORE0_PD_IDX,
    CORE1_PD_IDX,
    CORE2_PD_IDX,
    CORE3_PD_IDX,
    CORE4_PD_IDX,
    CORE5_PD_IDX,
    CORE6_PD_IDX,
    CORE7_PD_IDX
};

static struct mod_tcop_pct_table k_pct[] = {
    {
        /*
         * Perf limit for 3 or 4 cores online.
         * The first entry must be the maximum number of cores in this domain.
         */
        .cores_online = 4,
        .perf_limit = 1153 * 1000000UL,
    },
    {
        /* Perf limit for 1 or 2 cores online. */
        .cores_online = 2,
        .perf_limit = 1844 * 1000000UL,
    },
};

static struct mod_tcop_pct_table m_pct[] = {
    {
        /*
         * Perf limit for 3 cores online.
         * The first entry must be the maximum number of cores in this domain.
         */
        .cores_online = 3,
        .perf_limit = 1893 * 1000000UL,
    },
    {
        /* Perf limit for 2 cores online. */
        .cores_online = 2,
        .perf_limit = 2271 * 1000000UL,
    },
    {
        /* Perf limit for 1 core online. */
        .cores_online = 1,
        .perf_limit = 2650 * 1000000UL,
    },
};

static const struct mod_tcop_core_config k_core_config[] = {
    [0] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE0_PD_IDX),
        .core_starts_online = true,
        },
    [1] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE1_PD_IDX),
        .core_starts_online = false,
        },
    [2] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE2_PD_IDX),
        .core_starts_online = false,
        },
    [3] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE3_PD_IDX),
        .core_starts_online = false,
        },
};

static const struct mod_tcop_core_config m_core_config[] = {
    [0] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE4_PD_IDX),
        .core_starts_online = false,
        },
    [1] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE5_PD_IDX),
        .core_starts_online = false,
        },
    [2] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, CORE6_PD_IDX),
        .core_starts_online = false,
        },
};

static const struct mod_tcop_domain_config k_domain_conf[] = {
    [0] = {
        .perf_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_DVFS,
            DVFS_ELEMENT_IDX_KLEIN),
        .pct = k_pct,
        .pct_size = FWK_ARRAY_SIZE(k_pct),
        .core_config = k_core_config,
    },
    [1] = { { 0 } },
};

static const struct mod_tcop_domain_config m_domain_conf[] = {
    [0] = {
        .perf_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_DVFS,
            DVFS_ELEMENT_IDX_MATTERHORN),
        .pct = m_pct,
        .pct_size = FWK_ARRAY_SIZE(m_pct),
        .core_config = m_core_config,
    },
    [1] = { { 0 } },
};

static const struct fwk_element element_table[] = {
    [0] = {
        .name = "TCOP_KLEIN_ELEM",
        .sub_element_count = 4,
        .data = k_domain_conf,
    },
    [1] = {
        .name = "TCOP_MATTERHORN_ELEM",
        .sub_element_count = 3,
        .data = m_domain_conf,
    },
    [2] = { 0 },
};

static const struct fwk_element *tcop_get_element_table(fwk_id_t module_id)
{
    return element_table;
}
const struct fwk_module_config config_traffic_cop = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(tcop_get_element_table),
};

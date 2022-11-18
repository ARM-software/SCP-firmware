/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_traffic_cop.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

enum fake_tcop_pct_idx {
    FAKE_MOD_TCOP_PCT_0,
    FAKE_MOD_TCOP_PCT_1,
    FAKE_MOD_TCOP_PCT_COUNT,
};

enum fake_tcop_core_config_idx {
    FAKE_MOD_TCOP_CORE_CFG_0,
    FAKE_MOD_TCOP_CORE_CFG_1,
    FAKE_MOD_TCOP_CORE_CFG_COUNT,
};

enum fake_tcop_domain_idx {
    FAKE_MOD_TCOP_DOM_0,
    FAKE_MOD_TCOP_DOM_1,
    FAKE_MOD_TCOP_DOM_COUNT,
};

enum fake_tcop_element_idx {
    FAKE_MOD_TCOP_ELEM_0,
    FAKE_MOD_TCOP_ELEM_1,
    FAKE_MOD_TCOP_ELEM_COUNT,
};

static struct mod_tcop_pct_table fake_pct_table[FAKE_MOD_TCOP_PCT_COUNT] = {
    [FAKE_MOD_TCOP_PCT_0] = {
        .cores_online = 2,
        .perf_limit = 1001 * 1000000UL,
    },
    [FAKE_MOD_TCOP_PCT_1] = {
        .cores_online = 1,
        .perf_limit = 1002 * 1000000UL,
    },
};

static const struct mod_tcop_core_config
    fake_core_config[FAKE_MOD_TCOP_CORE_CFG_COUNT] = {
    [FAKE_MOD_TCOP_CORE_CFG_0] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, 0),
        .core_starts_online = true,
        },
    [FAKE_MOD_TCOP_CORE_CFG_1] = {
        .pd_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_POWER_DOMAIN, 1),
        .core_starts_online = false,
        },
};

static const struct mod_tcop_domain_config
    fake_domain_conf[FAKE_MOD_TCOP_DOM_COUNT] = {
    [FAKE_MOD_TCOP_DOM_0] = {
        .perf_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_DVFS, 0),
        .pct = fake_pct_table,
        .pct_size = FWK_ARRAY_SIZE(fake_pct_table),
        .core_config = fake_core_config,
    },
    [FAKE_MOD_TCOP_DOM_1] = { { 0 } },
};

static const struct fwk_element element_table[FAKE_MOD_TCOP_ELEM_COUNT] = {
    [FAKE_MOD_TCOP_ELEM_0] = {
        .name = "ELEM-0",
        .sub_element_count = 2,
        .data = fake_domain_conf,
    },
    [FAKE_MOD_TCOP_ELEM_1] = { 0 },
};

static const struct fwk_element *tcop_get_element_table(fwk_id_t module_id)
{
    return element_table;
}
const struct fwk_module_config config_traffic_cop = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(tcop_get_element_table),
};

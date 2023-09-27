/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEST_FWK_MODULE_IDX_H
#define TEST_FWK_MODULE_IDX_H

#include <fwk_id.h>

enum fwk_module_idx {
    FWK_MODULE_IDX_MPMM,
    FWK_MODULE_IDX_DVFS,
    FWK_MODULE_IDX_SCMI_PERF,
    FWK_MODULE_IDX_POWER_DOMAIN,
    FWK_MODULE_IDX_AMU_MMAP,
    FWK_MODULE_IDX_COUNT,
};

enum mod_amu_mmap_api_idx {
    MOD_AMU_MMAP_API_IDX_AMU,
    MOD_AMU_MMAP_API_IDX_COUNT,
};

static const fwk_id_t fwk_module_id_mpmm =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MPMM);

static const fwk_id_t fwk_module_id_scmi_perf =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_PERF);

#endif /* TEST_FWK_MODULE_IDX_H */

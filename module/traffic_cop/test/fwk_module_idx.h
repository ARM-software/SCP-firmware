/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEST_FWK_MODULE_MODULE_IDX_H
#define TEST_FWK_MODULE_MODULE_IDX_H

#include <fwk_id.h>

enum fwk_module_idx {
    FWK_MODULE_IDX_TRAFFIC_COP,
    FWK_MODULE_IDX_DVFS,
    FWK_MODULE_IDX_SCMI_PERF,
    FWK_MODULE_IDX_POWER_DOMAIN,
    FWK_MODULE_IDX_COUNT,
};

static const fwk_id_t fwk_module_id_traffic_cop =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_TRAFFIC_COP);

static const fwk_id_t fwk_module_id_power_domain =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_POWER_DOMAIN);

static const fwk_id_t fwk_module_id_dvfs =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_DVFS);

static const fwk_id_t fwk_module_id_scmi_perf =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_PERF);

#endif /* TEST_FWK_MODULE_MODULE_IDX_H */

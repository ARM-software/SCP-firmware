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
    FWK_MODULE_IDX_SCMI_SYSTEM_POWER_REQ,
    FWK_MODULE_IDX_SCMI,
    FWK_MODULE_IDX_SCMI_SYSTEM_POWER,
    FWK_MODULE_IDX_COUNT,
};

static const fwk_id_t fwk_module_id_scmi_system_power_req =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_SYSTEM_POWER_REQ);

static const fwk_id_t fwk_module_id_scmi =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI);

static const fwk_id_t fwk_module_id_scmi_system_power =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI_SYSTEM_POWER);

#endif /* TEST_FWK_MODULE_MODULE_IDX_H */

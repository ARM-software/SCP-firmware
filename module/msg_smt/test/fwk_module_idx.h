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
    FWK_MODULE_IDX_OPTEE_MBX,
    FWK_MODULE_IDX_MSG_SMT,
    FWK_MODULE_IDX_SCMI,
    FWK_MODULE_IDX_COUNT,
};

#define FWK_MODULE_BIND_ROUND_MAX 2

static const fwk_id_t fwk_module_id_scmi =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCMI);

#endif /* TEST_FWK_MODULE_IDX_H */

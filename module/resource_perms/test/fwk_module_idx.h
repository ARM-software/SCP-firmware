/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEST_FWK_MODULE_MODULE_IDX_H
#define TEST_FWK_MODULE_MODULE_IDX_H

#include <fwk_id.h>

enum fwk_module_idx {
    FWK_MODULE_IDX_RESOURCE_PERMS,
    FWK_MODULE_IDX_COUNT,
};

static const fwk_id_t fwk_module_id_resource_perms =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_RESOURCE_PERMS);

#endif /* TEST_FWK_MODULE_MODULE_IDX_H */

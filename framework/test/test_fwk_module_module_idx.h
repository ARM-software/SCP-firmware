/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEST_FWK_MODULE_MODULE_IDX_H
#define TEST_FWK_MODULE_MODULE_IDX_H

#include <fwk_id.h>

enum fwk_module_idx {
    FWK_MODULE_IDX_FAKE0,
    FWK_MODULE_IDX_FAKE1,
    FWK_MODULE_IDX_COUNT,
};

static const fwk_id_t fwk_module_id_fake0 =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_FAKE0);
static const fwk_id_t fwk_module_id_fake1 =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_FAKE1);

#endif /* TEST_FWK_MODULE_MODULE_IDX_H */

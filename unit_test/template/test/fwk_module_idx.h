/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * fwk_module_idx Template file
 */

#ifndef TEST_FWK_MODULE_IDX_H
#define TEST_FWK_MODULE_IDX_H

#include <fwk_id.h>

enum fwk_module_idx {
    FWK_MODULE_IDX_MODULE1,
    FWK_MODULE_IDX_MODULE2,
    FWK_MODULE_IDX_MODULE3,
    FWK_MODULE_IDX_COUNT,
};

static const fwk_id_t fwk_module_id_module1 =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MODULE1);

static const fwk_id_t fwk_module_id_module2 =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MODULE2);

static const fwk_id_t fwk_module_id_module3 =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MODULE3);

#endif /* TEST_FWK_MODULE_IDX_H */

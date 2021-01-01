/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef FWK_MODULE_IDX_H
#define FWK_MODULE_IDX_H

#include <fwk_id.h>

#ifdef FWK_TEST_MODULE_IDX_H
#    include FWK_TEST_MODULE_IDX_H
#else

enum fwk_module_idx {
    FWK_MODULE_IDX_TEST0,
    FWK_MODULE_IDX_TEST1,
    FWK_MODULE_IDX_TEST2,
    FWK_MODULE_IDX_COUNT,
};

static const fwk_id_t fwk_module_id_test0 =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_TEST0);
static const fwk_id_t fwk_module_id_test1 =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_TEST1);
static const fwk_id_t fwk_module_id_test2 =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_TEST2);

#endif

#endif /* FWK_MODULE_IDX_H */

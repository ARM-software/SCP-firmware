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
    FWK_MODULE_IDX_CDNS_I2C,
    FWK_MODULE_IDX_CLOCK,
    FWK_MODULE_IDX_PSU,
    FWK_MODULE_IDX_TIMER,
    FWK_MODULE_IDX_XR77128,
    FWK_MODULE_IDX_COUNT,
};

enum fwk_element_idx {
    FWK_ELEMENT_IDX_DUMMY,
    FWK_ELEMENT_IDX_COUNT,
};

enum fwk_sub_element_idx {
    FWK_CHANNEL_IDX_DUMMY,
    FWK_CHANNEL_IDX_COUNT,
};

static const fwk_id_t fwk_module_id_psu =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_PSU);

static const fwk_id_t fwk_module_id_xr77128 =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_XR77128);

static const fwk_id_t fwk_element_id_dummy =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_XR77128, FWK_ELEMENT_IDX_DUMMY);

static const fwk_id_t fwk_sub_element_id_dummy = FWK_ID_SUB_ELEMENT_INIT(
    FWK_MODULE_IDX_XR77128,
    FWK_ELEMENT_IDX_DUMMY,
    FWK_CHANNEL_IDX_DUMMY);

#endif /* TEST_FWK_MODULE_MODULE_IDX_H */

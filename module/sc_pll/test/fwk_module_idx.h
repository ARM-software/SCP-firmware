/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEST_FWK_MODULE_MODULE_IDX_H
#define TEST_FWK_MODULE_MODULE_IDX_H

#include <fwk_id.h>

enum fwk_module_idx {
    FWK_MODULE_IDX_CLOCK,
    FWK_MODULE_IDX_POWER_DOMAIN,
    FWK_MODULE_IDX_SC_PLL,
    FWK_MODULE_IDX_FAKE,
    FWK_MODULE_IDX_COUNT,
};

enum fwk_element_idx {
    FWK_ELEMENT_IDX_DUMMY,
    FWK_ELEMENT_IDX_COUNT,
};

static const fwk_id_t fwk_module_id_sc_pll =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SC_PLL);

static const fwk_id_t fwk_module_id_fake_module =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_FAKE);

static const fwk_id_t fwk_element_id_dummy =
    FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_SC_PLL, FWK_ELEMENT_IDX_DUMMY);

#endif /* TEST_FWK_MODULE_MODULE_IDX_H */

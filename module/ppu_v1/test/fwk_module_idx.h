/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TEST_FWK_MODULE_MODULE_IDX_H
#define TEST_FWK_MODULE_MODULE_IDX_H

#include <fwk_id.h>

enum fwk_module_idx {
    FWK_MODULE_IDX_PPU_V1,
    FWK_MODULE_IDX_POWER_DOMAIN,
    FWK_MODULE_IDX_TIMER,
    FWK_MODULE_IDX_COUNT,
};

static const fwk_id_t fwk_module_id_ppu_v1 =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_PPU_V1);

static const fwk_id_t fwk_module_id_power_domain =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_POWER_DOMAIN);

static const fwk_id_t fwk_module_id_timer =
    FWK_ID_MODULE_INIT(FWK_MODULE_IDX_TIMER);

#endif /* TEST_FWK_MODULE_MODULE_IDX_H */

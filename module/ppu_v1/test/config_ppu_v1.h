/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Power unit test configuration.
 */

#include <mod_ppu_v1.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

enum pd_ppu_idx {
    PD_PPU_IDX_0,
    PD_PPU_IDX_1,
    PD_PPU_IDX_COUNT,
    PD_PPU_IDX_NONE = UINT32_MAX
};

static const struct mod_ppu_v1_pd_config pd_ppu_ctx_config[PD_PPU_IDX_COUNT] = {
    [PD_PPU_IDX_0] = {
        .pd_type = MOD_PD_TYPE_CORE,
        .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, PD_PPU_IDX_0, 0),
        .alarm_delay = 10,
    },
    [PD_PPU_IDX_1] = {
        .pd_type = MOD_PD_TYPE_CORE,
        .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, PD_PPU_IDX_1, 0),
        .alarm_delay = 10,
    },
};

static struct mod_ppu_v1_config ppu_v1_config_data_ut = {
    .num_of_cores_in_cluster = 2,
};

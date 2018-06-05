/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <sgm775_irq.h>
#include <config_ppu_v0.h>
#include <mod_system_power.h>
#include <mod_sgm775_system.h>

/*
 * The DPU/GPU/VPU PPUs in this list are there as a temporary workaround, until
 * Linux supports handling these domains on its own.
 */
static const struct mod_system_power_ext_ppu_config ext_ppus[] = {
    {
        .ppu_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_PPU_V0,
            PPU_V0_ELEMENT_IDX_DPU0TOP),
        .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0, 0),
    },
    {
        .ppu_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_PPU_V0,
            PPU_V0_ELEMENT_IDX_DPU1TOP),
        .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0, 0),
    },
    {
        .ppu_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_PPU_V0,
            PPU_V0_ELEMENT_IDX_GPUTOP),
        .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0, 0),
    },
    {
        .ppu_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_PPU_V0,
            PPU_V0_ELEMENT_IDX_VPUTOP),
        .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0, 0),
    },
};

const struct fwk_module_config config_system_power = {
    .data = &((struct mod_system_power_config) {
        .soc_wakeup_irq = SOC_WAKEUP0_IRQ,
        .ppu_sys0_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V0,
                                           PPU_V0_ELEMENT_IDX_SYS0),
        .ppu_sys1_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V0,
                                           PPU_V0_ELEMENT_IDX_SYS1),
        .ppu_sys_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0, 0),
        .ext_ppus = ext_ppus,
        .ext_ppus_count = FWK_ARRAY_SIZE(ext_ppus),
        .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SGM775_SYSTEM),
        .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SGM775_SYSTEM,
             MOD_SGM775_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER)
    })
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_ppu_v0.h"
#include "sgm775_irq.h"

#include <mod_power_domain.h>
#include <mod_sgm775_system.h>
#include <mod_system_power.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

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

static const uint8_t system_power_to_sys_ppu0_state[] = {
    [MOD_PD_STATE_ON]                     = (uint8_t)MOD_PD_STATE_ON,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = (uint8_t)MOD_PD_STATE_OFF,
    [MOD_PD_STATE_OFF]                    = (uint8_t)MOD_PD_STATE_OFF,
};

static const uint8_t system_power_to_sys_ppu1_state[] = {
    [MOD_PD_STATE_ON]                     = (uint8_t)MOD_PD_STATE_ON,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = (uint8_t)MOD_PD_STATE_ON,
    [MOD_PD_STATE_OFF]                    = (uint8_t)MOD_PD_STATE_OFF,
};

static const struct fwk_element system_power_element_table[] = {
    [0] = {
        .name = "SYS-PPU-0",
        .data = &((struct mod_system_power_dev_config) {
            .sys_ppu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V0,
                PPU_V0_ELEMENT_IDX_SYS0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0, 0),
            .sys_state_table = system_power_to_sys_ppu0_state,
        }),
    },

    [1] = {
        .name = "SYS-PPU-1",
        .data = &((struct mod_system_power_dev_config) {
            .sys_ppu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V0,
                PPU_V0_ELEMENT_IDX_SYS1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0, 0),
            .sys_state_table = system_power_to_sys_ppu1_state,
        }),
    },

    [2] = { 0 }, /* Termination description */
};

static const struct fwk_element *system_power_get_element_table(
    fwk_id_t module_id)
{
    return system_power_element_table;
}

const struct fwk_module_config config_system_power = {
    .data = &((struct mod_system_power_config) {
        .soc_wakeup_irq = SOC_WAKEUP0_IRQ,
        .ext_ppus = ext_ppus,
        .ext_ppus_count = FWK_ARRAY_SIZE(ext_ppus),
        .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SGM775_SYSTEM),
        .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SGM775_SYSTEM,
             MOD_SGM775_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER),
        .initial_system_power_state = MOD_PD_STATE_ON,
    }),
    .get_element_table = system_power_get_element_table,
};

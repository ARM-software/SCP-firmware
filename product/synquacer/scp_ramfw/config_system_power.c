/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_ppu_v0.h"

#include <mod_power_domain.h>
#include <mod_synquacer_system.h>
#include <mod_system_power.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static const struct mod_system_power_ext_ppu_config ext_ppus[] = {
    {
        .ppu_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_PPU_V0_SYNQUACER,
            PPU_V0_ELEMENT_IDX_SYS1),
        .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0_SYNQUACER, 0),
    },
    {
        .ppu_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_PPU_V0_SYNQUACER,
            PPU_V0_ELEMENT_IDX_SYS2),
        .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0_SYNQUACER, 0),
    },
    {
        .ppu_id = FWK_ID_ELEMENT_INIT(
            FWK_MODULE_IDX_PPU_V0_SYNQUACER,
            PPU_V0_ELEMENT_IDX_SYSTOP),
        .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0_SYNQUACER, 0),
    },
};

static const uint8_t system_power_to_sys_debug_state[] = {
    [MOD_PD_STATE_ON]                     = (uint8_t)MOD_PD_STATE_ON,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = (uint8_t)MOD_PD_STATE_OFF,
    [MOD_PD_STATE_OFF]                    = (uint8_t)MOD_PD_STATE_OFF,
};

static const uint8_t system_power_to_sys3_state[] = {
    [MOD_PD_STATE_ON]                     = (uint8_t)MOD_PD_STATE_ON,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = (uint8_t)MOD_PD_STATE_ON,
    [MOD_PD_STATE_OFF]                    = (uint8_t)MOD_PD_STATE_OFF,
};

static const struct fwk_element system_power_element_table[] = {
    [0] = {
        .name = "DEBUG",
        .data = &((struct mod_system_power_dev_config) {
            .sys_ppu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V0_SYNQUACER,
                PPU_V0_ELEMENT_IDX_DEBUG),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0_SYNQUACER, 0),
            .sys_state_table = system_power_to_sys_debug_state,
        }),
    },

    [1] = {
        .name = "SYS3",
        .data = &((struct mod_system_power_dev_config) {
            .sys_ppu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V0_SYNQUACER,
                PPU_V0_ELEMENT_IDX_SYS3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0_SYNQUACER, 0),
            .sys_state_table = system_power_to_sys3_state,
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
        .soc_wakeup_irq = FWK_INTERRUPT_NONE,
        .ext_ppus = ext_ppus,
        .ext_ppus_count = FWK_ARRAY_SIZE(ext_ppus),
        .initial_system_power_state = MOD_PD_STATE_ON,
        .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SYNQUACER_SYSTEM),
        .driver_api_id = FWK_ID_API_INIT(
            FWK_MODULE_IDX_SYNQUACER_SYSTEM,
            MOD_SYNQUACER_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER)
    }),
    .get_element_table = system_power_get_element_table,
};

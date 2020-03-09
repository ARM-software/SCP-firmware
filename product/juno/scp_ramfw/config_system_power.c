/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "juno_irq.h"
#include "juno_ppu_idx.h"

#include <mod_juno_ppu.h>
#include <mod_power_domain.h>
#include <mod_system_power.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static const uint8_t system_power_to_sys_ppu_state[] = {
    [MOD_PD_STATE_ON]                     = (uint8_t)MOD_PD_STATE_ON,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] =
        (uint8_t)MOD_SYSTEM_POWER_POWER_STATE_SLEEP0,
    [MOD_PD_STATE_OFF]                    = (uint8_t)MOD_PD_STATE_OFF,
};

static const struct fwk_element system_power_element_table[] = {
    [0] = {
        .name = "",
        .data = &((struct mod_system_power_dev_config) {
            .sys_ppu_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_PPU,
                JUNO_PPU_DEV_IDX_SYSTOP),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_PPU,
                MOD_JUNO_PPU_API_IDX_PD),
            .sys_state_table = system_power_to_sys_ppu_state,
        }),
    },

    [1] = { 0 }, /* Termination description */
};

static const struct fwk_element *system_power_get_element_table(
    fwk_id_t module_id)
{
    return system_power_element_table;
}

const struct fwk_module_config config_system_power = {
    .data = &((struct mod_system_power_config) {
        .soc_wakeup_irq = EXT_WAKEUP_IRQ,
        .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_JUNO_SYSTEM),
        .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_SYSTEM, 0),
        .initial_system_power_state = MOD_PD_STATE_ON,
    }),
    .get_element_table = system_power_get_element_table,
};

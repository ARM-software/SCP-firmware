/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <morello_core.h>

#include <mod_morello_system.h>
#include <mod_ppu_v1.h>
#include <mod_system_power.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <fmw_cmsis.h>

static const uint8_t system_power_to_sys_ppu0_state[] = {
    [MOD_PD_STATE_ON] = (uint8_t)MOD_PD_STATE_ON,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = (uint8_t)MOD_PD_STATE_OFF,
    [MOD_PD_STATE_OFF] = (uint8_t)MOD_PD_STATE_OFF,
};

static const uint8_t system_power_to_sys_ppu1_state[] = {
    [MOD_PD_STATE_ON] = (uint8_t)MOD_PD_STATE_ON,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = (uint8_t)MOD_PD_STATE_ON,
    [MOD_PD_STATE_OFF] = (uint8_t)MOD_PD_STATE_OFF,
};

static struct fwk_element system_power_element_table[3] = {
    [0] = {
        .name = "SYS-PPU-0",
        .data = &((struct mod_system_power_dev_config) {
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .sys_state_table = system_power_to_sys_ppu0_state,
        }),
    },

    [1] = {
        .name = "SYS-PPU-1",
        .data = &((struct mod_system_power_dev_config) {
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .sys_state_table = system_power_to_sys_ppu1_state,
        }),
    },

    [2] = { 0 }, /* Termination description */
};

static struct mod_system_power_config system_power_config = {
    .soc_wakeup_irq = SCP_EXT_IRQ,

    /* System driver */
    .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MORELLO_SYSTEM),
    .driver_api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_MORELLO_SYSTEM,
        MOD_MORELLO_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER),

    /* Initial system state */
    .initial_system_power_state = MOD_PD_STATE_OFF,
};

static const struct fwk_element *morello_system_get_element_table(
    fwk_id_t unused)
{
    struct mod_system_power_dev_config *dev_config_table;
    unsigned int ppu_idx_base;
    unsigned int i;

    /* The system PPUs are placed after the cores, clusters & GPU, DPU PPUs */
    ppu_idx_base =
        morello_core_get_core_count() + morello_core_get_cluster_count();

    for (i = 0; i < (FWK_ARRAY_SIZE(system_power_element_table) - 1); i++) {
        dev_config_table =
            (struct mod_system_power_dev_config *)system_power_element_table[i]
                .data;
        dev_config_table->sys_ppu_id =
            fwk_id_build_element_id(fwk_module_id_ppu_v1, ppu_idx_base + i);
    }

    return system_power_element_table;
}

const struct fwk_module_config config_system_power = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(morello_system_get_element_table),
    .data = &system_power_config,
};

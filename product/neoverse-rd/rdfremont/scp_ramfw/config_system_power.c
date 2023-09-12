/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'system_power'.
 */

#include "platform_core.h"

#include <mod_power_domain.h>
#include <mod_ppu_v1.h>
#include <mod_scp_platform.h>
#include <mod_system_power.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/* Indices for system power module elements */
enum cfgd_mod_atu_element_idx {
    CFGD_MOD_SYSTEM_POWER_EIDX_SYS_PPU,
    CFGD_MOD_SYSTEM_POWER_EIDX_COUNT
};

/* Module 'system_power' element count */
#define MOD_SYSTEM_POWER_ELEMENT_COUNT (CFGD_MOD_SYSTEM_POWER_EIDX_COUNT + 1)

#define SYS_PWR_STATE_TABLE_COUNT (MOD_SYSTEM_POWER_POWER_STATE_SLEEP0 + 1)

static const uint8_t sys_pwr_state_table[SYS_PWR_STATE_TABLE_COUNT] = {
    [MOD_PD_STATE_OFF] = (uint8_t)MOD_PD_STATE_OFF,
    [MOD_PD_STATE_ON] = (uint8_t)MOD_PD_STATE_ON,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = (uint8_t)MOD_PD_STATE_OFF,
};

static struct fwk_element element_table[MOD_SYSTEM_POWER_ELEMENT_COUNT] = {
    [CFGD_MOD_SYSTEM_POWER_EIDX_SYS_PPU] = {
        .name = "SYS-PPU-0",
        .data = &((struct mod_system_power_dev_config) {
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .sys_state_table = sys_pwr_state_table,
        }),
    },
    [CFGD_MOD_SYSTEM_POWER_EIDX_COUNT] = { 0 }, /* Termination description */
};

static const struct fwk_element *system_power_get_element_table(fwk_id_t unused)
{
    unsigned int ppu_idx_base;
    unsigned int core_count;
    unsigned int cluster_count;
    unsigned int i;
    struct mod_system_power_dev_config *dev_config;

    core_count = platform_get_core_count();
    cluster_count = platform_get_cluster_count();

    /* The system PPUs are placed after the core and cluster PPUs */
    ppu_idx_base = core_count + cluster_count;

    /* Configure System PPU id */
    for (i = 0; i < (FWK_ARRAY_SIZE(element_table) - 1); i++) {
        dev_config =
            (struct mod_system_power_dev_config *)element_table[i].data;

        dev_config->sys_ppu_id =
            fwk_id_build_element_id(fwk_module_id_ppu_v1, ppu_idx_base + i);
    }

    return element_table;
}

static struct mod_system_power_config system_power_config = {
    .soc_wakeup_irq = FWK_INTERRUPT_NONE,
    .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SCP_PLATFORM),
    .driver_api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_SCP_PLATFORM,
        MOD_SCP_PLATFORM_API_IDX_SYSTEM_POWER_DRIVER),
    .initial_system_power_state = MOD_PD_STATE_ON,
};

const struct fwk_module_config config_system_power = {
    .data = &system_power_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(system_power_get_element_table),
};

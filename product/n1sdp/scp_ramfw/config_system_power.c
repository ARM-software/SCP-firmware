/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_n1sdp_system.h>
#include <mod_ppu_v1.h>
#include <mod_system_power.h>
#include <n1sdp_core.h>
#include <n1sdp_scp_irq.h>

/* Table with only a terminator */
static const struct fwk_element n1sdp_system_element_table = { 0 };

static struct mod_system_power_config system_power_config = {
    .soc_wakeup_irq = SCP_EXT_IRQ,

    /* PPU settings */
    .ppu_sys_api_id = FWK_ID_API_INIT(
        FWK_MODULE_IDX_PPU_V1,
        MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),

    /* System driver */
    .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_N1SDP_SYSTEM),
    .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_N1SDP_SYSTEM,
         MOD_N1SDP_SYSTEM_API_IDX_SYSTEM_POWER_DRIVER),
};

static const struct fwk_element *n1sdp_system_get_element_table(
    fwk_id_t unused)
{
    /* The system PPUs are placed after the core and cluster PPUs */
    unsigned int ppu_idx_base = n1sdp_core_get_core_count() +
                                n1sdp_core_get_cluster_count();

    /* Set the system PPU elements */
    system_power_config.ppu_sys0_id = fwk_id_build_element_id(
        fwk_module_id_ppu_v1, ppu_idx_base);
    system_power_config.ppu_sys1_id = fwk_id_build_element_id(
        fwk_module_id_ppu_v1, ppu_idx_base + 1);

    /*
     * Return table with only a terminator as this function is only used to
     * setup the dynamic module data.
     */
    return &n1sdp_system_element_table;
}

const struct fwk_module_config config_system_power = {
    .get_element_table = n1sdp_system_get_element_table,
    .data = &system_power_config,
};

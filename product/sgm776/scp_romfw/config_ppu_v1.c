/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sgm776_mmap.h"

#include <mod_msys_rom.h>
#include <mod_power_domain.h>
#include <mod_ppu_v1.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <fmw_cmsis.h>

#include <stdbool.h>

static struct fwk_element sgm776_ppu_v1_element_table[] = {
    {
        .name = "SYS0",
        .data = &((struct mod_ppu_v1_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = PPU_SYS0_BASE,
            .ppu.irq = PPU_SYS0_IRQ,
            .default_power_on = true,
            .observer_id = FWK_ID_NONE_INIT,
        }),
    },
    {
        .name = "SYS1",
        .data = &((struct mod_ppu_v1_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = PPU_SYS1_BASE,
            .ppu.irq = PPU_SYS1_IRQ,
            .default_power_on = true,
            .observer_id = FWK_ID_NONE_INIT,
        }),
    },
    {
        .name = "CLUS0",
        .data = &((struct mod_ppu_v1_pd_config) {
            .pd_type = MOD_PD_TYPE_CLUSTER,
            .ppu.reg_base = PPU_CLUS0_BASE,
            .ppu.irq = PPU_CLUS0_IRQ,
            .observer_id = FWK_ID_NONE_INIT,
        }),
    },
    {
        .name = "CORE0",
        .data = &((struct mod_ppu_v1_pd_config) {
            .pd_type = MOD_PD_TYPE_CORE,
            .ppu.reg_base = PPU_CLUS0CORE0_BASE,
            .ppu.irq = PPU_CLUS0CORE0_IRQ,
            .cluster_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 2),
            .observer_id = FWK_ID_NONE_INIT,
        }),
    },
    { 0 }, /* Termination entry */
};

static const struct fwk_element *sgm776_ppu_v1_get_element_table(
    fwk_id_t module_id)
{
    return sgm776_ppu_v1_element_table;
}

/*
 * Power module configuration data
 */
struct fwk_module_config config_ppu_v1 = {
    .data =
        &(struct mod_ppu_v1_config){
            .pd_notification_id = FWK_ID_NOTIFICATION_INIT(
                FWK_MODULE_IDX_MSYS_ROM,
                MOD_MSYS_ROM_NOTIFICATION_IDX_POWER_SYSTOP),
            .pd_source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_MSYS_ROM),
        },

    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(sgm776_ppu_v1_get_element_table),
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_ppu_v0.h>
#include <morello_scp_mmap.h>

#include <mod_ppu_v0.h>

#include <fwk_element.h>
#include <fwk_module.h>

#include <stdbool.h>

static struct fwk_element ppu_v0_element_table[PPU_V0_ELEMENT_IDX_COUNT + 1] = {
    [PPU_V0_ELEMENT_IDX_DBGTOP] = {
        .name = "DBGTOP",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE_DEBUG,
            .ppu.reg_base = SCP_PPU_DEBUG_BASE,
            .default_power_on = true,
        }),
    },
    [PPU_V0_ELEMENT_IDX_COUNT] = { 0 }, /* Termination entry */
};

static const struct fwk_element *ppu_v0_get_element_table(fwk_id_t module_id)
{
    return ppu_v0_element_table;
}

/*
 * Power module configuration data
 */
const struct fwk_module_config config_ppu_v0 = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(ppu_v0_get_element_table),
};

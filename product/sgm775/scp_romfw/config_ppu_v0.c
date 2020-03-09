/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sgm775_irq.h"
#include "sgm775_mmap.h"

#include <mod_power_domain.h>
#include <mod_ppu_v0.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <stdbool.h>

static struct fwk_element sgm775_ppu_v0_element_table[] = {
    {
        .name = "SYS0",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = PPU_SYS0_BASE,
            .ppu.irq = PPU_SYS0_IRQ,
            .default_power_on = true,
        }),
    },
    {
        .name = "SYS1",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = PPU_SYS1_BASE,
            .ppu.irq = PPU_SYS1_IRQ,
            .default_power_on = true,
        }),
    },
    { 0 }, /* Termination entry */
};


static const struct fwk_element *sgm775_ppu_v0_get_element_table(
    fwk_id_t module_id)
{
    return sgm775_ppu_v0_element_table;
}

/*
 * Power module configuration data
 */
struct fwk_module_config config_ppu_v0 = {
    .get_element_table = sgm775_ppu_v0_get_element_table,
};

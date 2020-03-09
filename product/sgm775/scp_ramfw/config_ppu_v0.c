/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_ppu_v0.h"
#include "sgm775_irq.h"
#include "sgm775_mmap.h"

#include <mod_power_domain.h>
#include <mod_ppu_v0.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>

#include <stdbool.h>
#include <stddef.h>

static struct fwk_element sgm775_ppu_v0_element_table[] = {
    [PPU_V0_ELEMENT_IDX_DBGTOP] = {
        .name = "DBGTOP",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE_DEBUG,
            .ppu.reg_base = PPU_DEBUG_BASE,
            .ppu.irq = PPU_DEBUG_IRQ
        }),
    },
    [PPU_V0_ELEMENT_IDX_DPU0TOP] = {
        .name = "DPU0TOP",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = PPU_DPU0_BASE,
            .ppu.irq = PPU_DPU0_IRQ,
            .default_power_on = true,
        }),
    },
    [PPU_V0_ELEMENT_IDX_DPU1TOP] = {
        .name = "DPU1TOP",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = PPU_DPU1_BASE,
            .ppu.irq = PPU_DPU1_IRQ,
            .default_power_on = true,
        }),
    },
    [PPU_V0_ELEMENT_IDX_GPUTOP] = {
        .name = "GPUTOP",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = PPU_GPU_BASE,
            .ppu.irq = PPU_GPU_IRQ,
            .default_power_on = true,
        }),
    },
    [PPU_V0_ELEMENT_IDX_VPUTOP] = {
        .name = "VPUTOP",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = PPU_VPU_BASE,
            .ppu.irq = PPU_VPU_IRQ,
            .default_power_on = true,
        }),
    },
    [PPU_V0_ELEMENT_IDX_SYS0] = {
        .name = "SYS0",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = PPU_SYS0_BASE,
            .ppu.irq = PPU_SYS0_IRQ
        }),
    },
    [PPU_V0_ELEMENT_IDX_SYS1] = {
        .name = "SYS1",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = PPU_SYS1_BASE,
            .ppu.irq = PPU_SYS1_IRQ
        }),
    },
    [PPU_V0_ELEMENT_IDX_COUNT] = { 0 }, /* Termination entry */
};


static const struct fwk_element *sgm775_ppu_v0_get_element_table
                                                           (fwk_id_t module_id)
{
    return sgm775_ppu_v0_element_table;
}

/*
 * Power module configuration data
 */
struct fwk_module_config config_ppu_v0 = {
    .get_element_table = sgm775_ppu_v0_get_element_table,
    .data = NULL,
};

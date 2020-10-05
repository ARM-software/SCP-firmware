/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_module.h>
#include <mod_ppu_v0.h>
#include <fake_irq.h>
#include "config_ppu_v0.h"

uint32_t vppu_regdbg[16];
uint32_t vppu_regdpu0[16];
uint32_t vppu_regdpu1[16];
uint32_t vppu_reggpu[16];
uint32_t vppu_regvpu[16];
uint32_t vppu_regsys0[16];
uint32_t vppu_regsys1[16];
uint32_t vppu_regcluster[16];
uint32_t vppu_regsys[16];

static struct fwk_element host_ppu_v0_element_table[] = {
   [PPU_V0_ELEMENT_IDX_CPU0] = {
        .name = "CPU0",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regcluster,
            .ppu.irq = PPU_SYS1_IRQ
        }),
    },
   [PPU_V0_ELEMENT_IDX_CPU1] = {
        .name = "CPU1",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regcluster,
            .ppu.irq = PPU_SYS1_IRQ
        }),
    },
   [PPU_V0_ELEMENT_IDX_CPU2] = {
        .name = "CPU2",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regcluster,
            .ppu.irq = PPU_SYS1_IRQ
        }),
    },
   [PPU_V0_ELEMENT_IDX_CPU3] = {
        .name = "CPU3",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regcluster,
            .ppu.irq = PPU_SYS1_IRQ
        }),
    },
   [PPU_V0_ELEMENT_IDX_CLUSTER0] = {
        .name = "CLUSTER0",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regcluster,
            .ppu.irq = PPU_SYS1_IRQ
        }),
    },
   [PPU_V0_ELEMENT_IDX_CLUSTER1] = {
        .name = "CLUSTER1",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regcluster,
            .ppu.irq = PPU_SYS1_IRQ
        }),
    },
    [PPU_V0_ELEMENT_IDX_DBGTOP] = {
        .name = "DBGTOP",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE_DEBUG,
            .ppu.reg_base = (uintptr_t)vppu_regdbg,
            .ppu.irq = PPU_DEBUG_IRQ
        }),
    },
    [PPU_V0_ELEMENT_IDX_DPU0TOP] = {
        .name = "DPU0TOP",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regdpu0,
            .ppu.irq = PPU_DPU0_IRQ,
            .default_power_on = true,
        }),
    },
    [PPU_V0_ELEMENT_IDX_DPU1TOP] = {
        .name = "DPU1TOP",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regdpu1,
            .ppu.irq = PPU_DPU1_IRQ,
            .default_power_on = true,
        }),
    },
    [PPU_V0_ELEMENT_IDX_GPUTOP] = {
        .name = "GPUTOP",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_reggpu,
            .ppu.irq = PPU_GPU_IRQ,
            .default_power_on = true,
        }),
    },
    [PPU_V0_ELEMENT_IDX_VPUTOP] = {
        .name = "VPUTOP",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regvpu,
            .ppu.irq = PPU_VPU_IRQ,
            .default_power_on = true,
        }),
    },
    [PPU_V0_ELEMENT_IDX_SYS0] = {
        .name = "SYS0",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = (uintptr_t)vppu_regsys0,
            .ppu.irq = PPU_SYS0_IRQ
        }),
    },
    [PPU_V0_ELEMENT_IDX_SYS1] = {
        .name = "SYS1",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = (uintptr_t)vppu_regsys1,
            .ppu.irq = PPU_SYS1_IRQ
        }),
    },
   [PPU_V0_ELEMENT_IDX_SYSTEM] = {
        .name = "SYSTEM",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regsys,
            .ppu.irq = PPU_SYS1_IRQ,
            .default_power_on = true,
        }),
    },
    [PPU_V0_ELEMENT_IDX_COUNT] = { 0 }, /* Termination entry */
};


static const struct fwk_element *host_ppu_v0_get_element_table
                                                           (fwk_id_t module_id)
{
    return host_ppu_v0_element_table;
}

/*
 * Power module configuration data
 */
struct fwk_module_config config_vppu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(host_ppu_v0_get_element_table),
};

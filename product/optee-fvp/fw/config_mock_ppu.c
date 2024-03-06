/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_mock_ppu.h"

#include <mod_mock_ppu.h>

#include <fwk_module.h>

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
   [MOCK_PPU_ELEMENT_IDX_CPU0] = {
        .name = "CPU0",
        .data = &((struct mod_mock_ppu_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regcluster,
        }),
    },
   [MOCK_PPU_ELEMENT_IDX_CPU1] = {
        .name = "CPU1",
        .data = &((struct mod_mock_ppu_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regcluster,
        }),
    },
   [MOCK_PPU_ELEMENT_IDX_CPU2] = {
        .name = "CPU2",
        .data = &((struct mod_mock_ppu_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regcluster,
        }),
    },
   [MOCK_PPU_ELEMENT_IDX_CPU3] = {
        .name = "CPU3",
        .data = &((struct mod_mock_ppu_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regcluster,
        }),
    },
   [MOCK_PPU_ELEMENT_IDX_CLUSTER0] = {
        .name = "CLUSTER0",
        .data = &((struct mod_mock_ppu_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regcluster,
        }),
    },
   [MOCK_PPU_ELEMENT_IDX_CLUSTER1] = {
        .name = "CLUSTER1",
        .data = &((struct mod_mock_ppu_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regcluster,
        }),
    },
    [MOCK_PPU_ELEMENT_IDX_DBGTOP] = {
        .name = "DBGTOP",
        .data = &((struct mod_mock_ppu_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE_DEBUG,
            .ppu.reg_base = (uintptr_t)vppu_regdbg,
        }),
    },
    [MOCK_PPU_ELEMENT_IDX_DPU0TOP] = {
        .name = "DPU0TOP",
        .data = &((struct mod_mock_ppu_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regdpu0,
            .default_power_on = true,
        }),
    },
    [MOCK_PPU_ELEMENT_IDX_DPU1TOP] = {
        .name = "DPU1TOP",
        .data = &((struct mod_mock_ppu_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regdpu1,
            .default_power_on = true,
        }),
    },
    [MOCK_PPU_ELEMENT_IDX_GPUTOP] = {
        .name = "GPUTOP",
        .data = &((struct mod_mock_ppu_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_reggpu,
            .default_power_on = true,
        }),
    },
    [MOCK_PPU_ELEMENT_IDX_VPUTOP] = {
        .name = "VPUTOP",
        .data = &((struct mod_mock_ppu_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regvpu,
            .default_power_on = true,
        }),
    },
    [MOCK_PPU_ELEMENT_IDX_SYS0] = {
        .name = "SYS0",
        .data = &((struct mod_mock_ppu_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = (uintptr_t)vppu_regsys0,
        }),
    },
    [MOCK_PPU_ELEMENT_IDX_SYS1] = {
        .name = "SYS1",
        .data = &((struct mod_mock_ppu_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = (uintptr_t)vppu_regsys1,
        }),
    },
   [MOCK_PPU_ELEMENT_IDX_SYSTEM] = {
        .name = "SYSTEM",
        .data = &((struct mod_mock_ppu_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)vppu_regsys,
            .default_power_on = true,
        }),
    },
    [MOCK_PPU_ELEMENT_IDX_COUNT] = { 0 }, /* Termination entry */
};

static const struct fwk_element *mock_ppu_get_element_table(fwk_id_t module_id)
{
    return host_ppu_v0_element_table;
}

/*
 * Power module configuration data
 */
struct fwk_module_config config_mock_ppu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(mock_ppu_get_element_table),
};

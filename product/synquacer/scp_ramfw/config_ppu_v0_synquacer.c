/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "synquacer_core.h"
#include "synquacer_mmap.h"

#include <mod_power_domain.h>
#include <mod_ppu_v0.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define PPU_V0_NAME_SIZE (12)

static struct fwk_element ppu_v0_static_element_table[] = {
    [0] = {
        .name = "SYS3",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)PPU_SYS3,
            .default_power_on = false,
        }),
    },
    [1] = {
        .name = "SYS1",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)PPU_SYS1,
            .default_power_on = false,
        }),
    },
    [2] = {
        .name = "SYS2",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = (uintptr_t)PPU_SYS2,
            .default_power_on = false,
        }),
    },
    [3] = {
        .name = "DEBUG",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE_DEBUG,
            .ppu.reg_base = (uintptr_t)PPU_DEBUG,
            .default_power_on = false,
        }),
    },
    [4] = {
        .name = "SYSTOP",
        .data = &((struct mod_ppu_v0_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = (uintptr_t)PPU_SYS0,
            .default_power_on = false,
        }),
    },
    [5] = { 0 }, /* Termination entry */

    /* PPU_SYS4 is always ON and managed by romfw */

};

static const struct fwk_element *ppu_v0_get_element_table(fwk_id_t module_id)
{
    struct fwk_element *element_table, *element;
    struct mod_ppu_v0_pd_config *ppu_v0_config_table, *ppu_v0_config;
    unsigned int core_idx;
    unsigned int cluster_idx;
    unsigned int core_count;
    unsigned int cluster_count;
    unsigned int core_per_cluster_count;
    unsigned int element_count = 0;

    core_count = synquacer_core_get_core_count();
    cluster_count = synquacer_core_get_cluster_count();
    core_per_cluster_count = synquacer_core_get_core_per_cluster_count();

    element_table = fwk_mm_calloc(
        cluster_count + core_count +
            FWK_ARRAY_SIZE(ppu_v0_static_element_table) + 1, /* Terminator */
        sizeof(struct fwk_element));
    if (element_table == NULL)
        return NULL;

    ppu_v0_config_table = fwk_mm_calloc(
        (cluster_count + core_count), sizeof(struct mod_ppu_v0_pd_config));

    element_count = 0;

    for (cluster_idx = 0; cluster_idx < cluster_count; cluster_idx++) {
        /* prepare core config table */
        for (core_idx = 0; core_idx < core_per_cluster_count; core_idx++) {
            element = &element_table[element_count];
            ppu_v0_config = &ppu_v0_config_table[element_count];

            element->name = fwk_mm_alloc(PPU_V0_NAME_SIZE, 1);

            snprintf(
                (char *)element->name,
                PPU_V0_NAME_SIZE,
                "CLUS%uCORE%u",
                cluster_idx,
                core_idx);

            element->data = ppu_v0_config;
            ppu_v0_config->pd_type = MOD_PD_TYPE_CORE;
            ppu_v0_config->ppu.reg_base =
                (uintptr_t)PPU_CPU(cluster_idx, core_idx);
            ppu_v0_config->ppu.irq = FWK_INTERRUPT_NONE;
            ppu_v0_config->default_power_on = false;
            element_count++;
        }
    }

    for (cluster_idx = 0; cluster_idx < cluster_count; cluster_idx++) {
        element = &element_table[element_count];
        ppu_v0_config = &ppu_v0_config_table[element_count];

        /* prepare cluster config table */
        element->name = fwk_mm_alloc(PPU_V0_NAME_SIZE, 1);

        snprintf(
            (char *)element->name, PPU_V0_NAME_SIZE, "CLUS%u", cluster_idx);

        element->data = ppu_v0_config;
        ppu_v0_config->pd_type = MOD_PD_TYPE_CLUSTER;
        ppu_v0_config->ppu.reg_base = (uintptr_t)PPU_CLUSTER(cluster_idx);
        ppu_v0_config->ppu.irq = FWK_INTERRUPT_NONE;
        ppu_v0_config->default_power_on = false;
        element_count++;
    }

    memcpy(
        element_table + element_count,
        ppu_v0_static_element_table,
        sizeof(ppu_v0_static_element_table));
    return element_table;
}

/*
 * Power module configuration data
 */
const struct fwk_module_config config_ppu_v0_synquacer = {
    .get_element_table = ppu_v0_get_element_table,
};

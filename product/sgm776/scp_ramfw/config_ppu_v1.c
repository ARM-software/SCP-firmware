/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "config_ppu_v1.h"
#include "sgm776_core.h"
#include "sgm776_irq.h"
#include "sgm776_mmap.h"

#include <mod_power_domain.h>
#include <mod_ppu_v1.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const char *core_pd_name_table[SGM776_CORE_PER_CLUSTER_MAX] = {
    "CLUS0CORE0", "CLUS0CORE1", "CLUS0CORE2", "CLUS0CORE3",
    "CLUS0CORE4", "CLUS0CORE5", "CLUS0CORE6", "CLUS0CORE7",
};

static uintptr_t core_pd_ppu_base_table[] = {
    PPU_CLUS0CORE0_BASE, PPU_CLUS0CORE1_BASE, PPU_CLUS0CORE2_BASE,
    PPU_CLUS0CORE3_BASE, PPU_CLUS0CORE4_BASE, PPU_CLUS0CORE5_BASE,
    PPU_CLUS0CORE6_BASE, PPU_CLUS0CORE7_BASE
};

static unsigned int core_pd_ppu_irq_table[] = {
    PPU_CLUS0CORE0_IRQ, PPU_CLUS0CORE1_IRQ, PPU_CLUS0CORE2_IRQ,
    PPU_CLUS0CORE3_IRQ, PPU_CLUS0CORE4_IRQ, PPU_CLUS0CORE5_IRQ,
    PPU_CLUS0CORE6_IRQ, PPU_CLUS0CORE7_IRQ
};

struct mod_ppu_v1_config sgm776_ppu_v1_notification_config = {
    .pd_notification_id = FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_POWER_DOMAIN,
        MOD_PD_NOTIFICATION_IDX_POWER_STATE_TRANSITION),
};

static const struct fwk_element static_ppu_table[] = {
    [PPU_V1_ELEMENT_IDX_DBGTOP] = {
        .name = "DBGTOP",
        .data = &((struct mod_ppu_v1_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE_DEBUG,
            .ppu.reg_base = PPU_DEBUG_BASE,
            .ppu.irq = PPU_DEBUG_IRQ,
            .observer_id = FWK_ID_NONE_INIT,
        }),
    },
    [PPU_V1_ELEMENT_IDX_DPUTOP] = {
        .name = "DPU0TOP",
        .data = &((struct mod_ppu_v1_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = PPU_DPU_BASE,
            .ppu.irq = PPU_DPU_IRQ,
            .observer_id = FWK_ID_NONE_INIT,
            .default_power_on = true,
        }),
    },
    [PPU_V1_ELEMENT_IDX_GPUTOP] = {
        .name = "GPUTOP",
        .data = &((struct mod_ppu_v1_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = PPU_GPU_BASE,
            .ppu.irq = PPU_GPU_IRQ,
            .observer_id = FWK_ID_NONE_INIT,
            .default_power_on = true,
        }),
    },
    [PPU_V1_ELEMENT_IDX_VPUTOP] = {
        .name = "VPUTOP",
        .data = &((struct mod_ppu_v1_pd_config) {
            .pd_type = MOD_PD_TYPE_DEVICE,
            .ppu.reg_base = PPU_VPU_BASE,
            .ppu.irq = PPU_VPU_IRQ,
            .observer_id = FWK_ID_NONE_INIT,
            .default_power_on = true,
        }),
    },
    [PPU_V1_ELEMENT_IDX_SYS0] = {
        .name = "SYS0",
        .data = &((struct mod_ppu_v1_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = PPU_SYS0_BASE,
            .ppu.irq = PPU_SYS0_IRQ,
            .observer_id = FWK_ID_NONE_INIT,
        }),
    },
    [PPU_V1_ELEMENT_IDX_SYS1] = {
        .name = "SYS1",
        .data = &((struct mod_ppu_v1_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = PPU_SYS1_BASE,
            .ppu.irq = PPU_SYS1_IRQ,
            .observer_id = FWK_ID_NONE_INIT,
        }),
    },
};

static const struct fwk_element *sgm776_ppu_v1_get_element_table(
    fwk_id_t module_id)
{
    struct fwk_element *element_table, *element;
    struct mod_ppu_v1_pd_config *pd_config_table, *pd_config;
    unsigned int core_idx, table_size;

    /*
     * Allocate element descriptors based on:
     *   Size of static ppu table
     *   +  Number of cores
     *   +1 cluster descriptor
     *   +1 terminator descriptor
     */
    table_size = FWK_ARRAY_SIZE(static_ppu_table) + sgm776_core_get_count() + 2;
    element_table = fwk_mm_calloc(table_size, sizeof(struct fwk_element));

    /* Table to hold configs for all cores and the cluster */
    pd_config_table = fwk_mm_calloc(sgm776_core_get_count() + 1,
                                    sizeof(struct mod_ppu_v1_pd_config));

    /* Cores */
    for (core_idx = 0; core_idx < sgm776_core_get_count(); core_idx++) {
        element = &element_table[PPU_V1_ELEMENT_IDX_COUNT + core_idx];
        pd_config = &pd_config_table[core_idx];

        element->name = core_pd_name_table[core_idx];
        element->data = pd_config;

        pd_config->pd_type = MOD_PD_TYPE_CORE;
        pd_config->ppu.reg_base = core_pd_ppu_base_table[core_idx];
        pd_config->ppu.irq = core_pd_ppu_irq_table[core_idx];
        pd_config->cluster_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V1,
                                               table_size - 2);
        pd_config->observer_id = FWK_ID_NONE;
    }

    element = &element_table[table_size - 2];
    pd_config = &pd_config_table[sgm776_core_get_count()];

    element->name = "CLUS0";
    element->data = pd_config;

    pd_config->pd_type = MOD_PD_TYPE_CLUSTER;
    pd_config->ppu.reg_base = PPU_CLUS0_BASE;
    pd_config->ppu.irq = PPU_CLUS0_IRQ;
    pd_config->observer_id = FWK_ID_NONE;

    /* Copy static table to the beginning of the dynamic table */
    memcpy(element_table, static_ppu_table, sizeof(static_ppu_table));

    sgm776_ppu_v1_notification_config.pd_source_id = FWK_ID_ELEMENT(
        FWK_MODULE_IDX_POWER_DOMAIN,
        CONFIG_POWER_DOMAIN_SYSTOP_CHILD_COUNT + sgm776_core_get_count());

    return element_table;
}

/*
 * Power module configuration data
 */
struct fwk_module_config config_ppu_v1 = {
    .get_element_table = sgm776_ppu_v1_get_element_table,
    .data = &sgm776_ppu_v1_notification_config,
};

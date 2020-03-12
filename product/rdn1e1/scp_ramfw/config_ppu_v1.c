/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdio.h>
#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_cmn600.h>
#include <mod_power_domain.h>
#include <mod_ppu_v1.h>
#include <rdn1e1_core.h>
#include <scp_rdn1e1_irq.h>
#include <scp_rdn1e1_mmap.h>
#include <config_power_domain.h>

/* Maximum PPU core name size including the null terminator */
#define PPU_CORE_NAME_SIZE 12

/* Maximum PPU cluster name size including the null terminator */
#define PPU_CLUS_NAME_SIZE 6

/* Lookup table for translating cluster indicies into CMN600 node IDs */
static const unsigned int cluster_idx_to_node_id[] = {76, 32};

/* Module configuration data */
static struct mod_ppu_v1_config ppu_v1_config_data = {
    .pd_notification_id = FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_POWER_DOMAIN,
        MOD_PD_NOTIFICATION_IDX_POWER_STATE_TRANSITION),
};

static struct fwk_element ppu_v1_system_element_table[] = {
    [0] = {
        .name = "SYS0",
        .data = &((struct mod_ppu_v1_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = SCP_PPU_SYS0_BASE,
            .observer_id = FWK_ID_NONE_INIT,
        }),
    },
    [1] = {
        .name = "SYS1",
        .data = &((struct mod_ppu_v1_pd_config) {
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = SCP_PPU_SYS1_BASE,
            .observer_id = FWK_ID_NONE_INIT,
        }),
    },
};

static const struct fwk_element *ppu_v1_get_element_table(fwk_id_t module_id)
{
    struct fwk_element *element_table, *element;
    struct mod_ppu_v1_pd_config *pd_config_table, *pd_config;
    unsigned int core_idx;
    unsigned int cluster_idx;
    unsigned int core_count;
    unsigned int cluster_count;
    unsigned int core_element_count = 0;

    core_count = rdn1e1_core_get_core_count();
    cluster_count = rdn1e1_core_get_cluster_count();

    assert(cluster_count == FWK_ARRAY_SIZE(cluster_idx_to_node_id));

    /*
     * Allocate element descriptors based on:
     *   Number of cores
     *   + Number of cluster descriptors
     *   + Number of system power domain descriptors
     *   + 1 terminator descriptor
     */
    element_table = fwk_mm_calloc(core_count + cluster_count +
        FWK_ARRAY_SIZE(ppu_v1_system_element_table) + 1,
        sizeof(struct fwk_element));

    pd_config_table = fwk_mm_calloc(core_count + cluster_count,
                                    sizeof(struct mod_ppu_v1_pd_config));

    for (cluster_idx = 0; cluster_idx < cluster_count; cluster_idx++) {
        for (core_idx = 0;
             core_idx < rdn1e1_core_get_core_per_cluster_count(cluster_idx);
             core_idx++) {
            element = &element_table[core_element_count];
            pd_config = &pd_config_table[core_element_count];

            element->name = fwk_mm_alloc(PPU_CORE_NAME_SIZE, 1);

            snprintf((char *)element->name, PPU_CORE_NAME_SIZE, "CLUS%uCORE%u",
                cluster_idx, core_idx);

            element->data = pd_config;

            pd_config->pd_type = MOD_PD_TYPE_CORE;
            pd_config->ppu.reg_base = SCP_PPU_CORE_BASE(cluster_idx, core_idx);
            pd_config->ppu.irq = FWK_INTERRUPT_NONE;
            pd_config->cluster_id =
                FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V1,
                               (core_count + cluster_idx));
            pd_config->observer_id = FWK_ID_NONE;
            core_element_count++;
        }

        element = &element_table[core_count + cluster_idx];
        pd_config = &pd_config_table[core_count + cluster_idx];

        element->name = fwk_mm_alloc(PPU_CLUS_NAME_SIZE, 1);

        snprintf((char *)element->name, PPU_CLUS_NAME_SIZE, "CLUS%u",
            cluster_idx);

        element->data = pd_config;

        pd_config->pd_type = MOD_PD_TYPE_CLUSTER;
        pd_config->ppu.reg_base = SCP_PPU_CLUSTER_BASE(cluster_idx);
        pd_config->ppu.irq = FWK_INTERRUPT_NONE;
        pd_config->observer_id = fwk_module_id_cmn600;
        pd_config->observer_api = FWK_ID_API(FWK_MODULE_IDX_CMN600,
                                             MOD_CMN600_API_IDX_PPU_OBSERVER);
        pd_config->post_ppu_on_param =
            (void *)&cluster_idx_to_node_id[cluster_idx];
    }

    memcpy(&element_table[core_count + cluster_count],
           ppu_v1_system_element_table,
           sizeof(ppu_v1_system_element_table));

    /*
     * Configure pd_source_id with the SYSTOP identifier from the power domain
     * module which is dynamically defined based on the number of cores.
     */
    ppu_v1_config_data.pd_source_id = fwk_id_build_element_id(
        fwk_module_id_power_domain,
        core_count + PD_STATIC_DEV_IDX_SYSTOP);

    return element_table;
}

/*
 * Power module configuration data
 */
const struct fwk_module_config config_ppu_v1 = {
    .get_element_table = ppu_v1_get_element_table,
    .data = &ppu_v1_config_data,
};

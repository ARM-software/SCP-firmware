/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "scp_mmap.h"
#include "tc_core.h"

#include <mod_cmn_booker.h>
#include <mod_power_domain.h>
#include <mod_ppu_v1.h>
#include <mod_tc2_bl1.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_string.h>

#include <stdio.h>
#include <string.h>

/* Maximum PPU core name size including the null terminator */
#define PPU_CORE_NAME_SIZE 12

/* Maximum PPU cluster name size including the null terminator */
#define PPU_CLUS_NAME_SIZE 6

/* Lookup table for translating cluster indicies into CMN_BOOKER node IDs */
static const unsigned int cluster_idx_to_node_id[1] = { 68 };

static const struct fwk_element ppu_v1_system_element_table[1] = {
    {
        .name = "SYS0",
        .data = &((struct mod_ppu_v1_pd_config){
            .pd_type = MOD_PD_TYPE_SYSTEM,
            .ppu.reg_base = SCP_PPU_SYS0_BASE,
            .observer_id = FWK_ID_NONE_INIT,
            .default_power_on = true,
        }),
    },
};

static const struct fwk_element *tc2_ppu_v1_get_element_table(
    fwk_id_t module_id)
{
    struct fwk_element *element_table, *element;
    struct mod_ppu_v1_pd_config *pd_config_table, *pd_config;

    /*
     * Allocate element descriptors based on:
     *   Core0
     *   + Cluster0
     *   + Number of system power domain descriptors
     *   + 1 terminator descriptor
     */
    element_table = fwk_mm_calloc(
        2 + FWK_ARRAY_SIZE(ppu_v1_system_element_table) + 1,
        sizeof(struct fwk_element));
    if (element_table == NULL) {
        return NULL;
    }

    pd_config_table = fwk_mm_calloc(2, sizeof(struct mod_ppu_v1_pd_config));
    if (pd_config_table == NULL) {
        return NULL;
    }

    // pd_config for core0
    element = &element_table[0];
    pd_config = &pd_config_table[0];

    element->name = fwk_mm_alloc(PPU_CORE_NAME_SIZE, 1);
    if (element->name == NULL) {
        return NULL;
    }

    (void)snprintf((char *)element->name, PPU_CORE_NAME_SIZE, "CLUS0CORE0");

    element->data = pd_config;

    pd_config->pd_type = MOD_PD_TYPE_CORE;
    pd_config->ppu.reg_base = SCP_PPU_CORE_BASE(0);
    pd_config->ppu.irq = FWK_INTERRUPT_NONE;
    pd_config->cluster_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V1, 1);
    pd_config->observer_id = FWK_ID_NONE;

    element = &element_table[1];
    pd_config = &pd_config_table[1];

    element->name = fwk_mm_alloc(PPU_CLUS_NAME_SIZE, 1);
    if (element->name == NULL) {
        return NULL;
    }

    (void)snprintf((char *)element->name, PPU_CLUS_NAME_SIZE, "CLUS0");

    element->data = pd_config;

    pd_config->pd_type = MOD_PD_TYPE_CLUSTER;
    pd_config->ppu.reg_base = SCP_PPU_CLUSTER_BASE;
    pd_config->ppu.irq = FWK_INTERRUPT_NONE;

    pd_config->observer_id = fwk_module_id_cmn_booker;
    pd_config->observer_api = FWK_ID_API(
        FWK_MODULE_IDX_CMN_BOOKER, MOD_CMN_BOOKER_API_IDX_PPU_OBSERVER);
    pd_config->post_ppu_on_param = (void *)&cluster_idx_to_node_id[0];

    fwk_str_memcpy(
        &element_table[2],
        ppu_v1_system_element_table,
        sizeof(ppu_v1_system_element_table));

    return element_table;
}

/*
 * Power module configuration data
 */
struct fwk_module_config config_ppu_v1 = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(tc2_ppu_v1_get_element_table),
    .data =
        &(struct mod_ppu_v1_config){
            .pd_notification_id = FWK_ID_NOTIFICATION_INIT(
                FWK_MODULE_IDX_TC2_BL1,
                MOD_TC2_BL1_NOTIFICATION_IDX_POWER_SYSTOP),
            .pd_source_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_TC2_BL1),
        },
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "config_ppu_v0.h"
#include "synquacer_core.h"

#include <mod_power_domain.h>
#include <mod_system_power.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Maximum power domain name size including the null terminator */
#define PD_NAME_SIZE 12

/*
 * Mask of the allowed states for the cluster power domain depending on the
 * system states.
 */
static const uint32_t systop_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK
};

/*
 * Mask of the allowed states for the cluster power domain depending on the
 * system states.
 */
static const uint32_t cluster_pd_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK
};

/* Mask of the allowed states for a core depending on the cluster states. */
static const uint32_t core_pd_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK
};

/* Power module specific configuration data (none) */
static const struct mod_power_domain_config synquacer_power_domain_config = {
    0
};

/* Tree position should follow the ascending order */
static struct fwk_element synquacer_power_domain_static_element_table[] = {
    [PD_STATIC_DEV_IDX_CHILD_SYS3] = {
        .name = "SYS3",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .tree_pos = MOD_PD_TREE_POS(MOD_PD_LEVEL_1, 0, 0, 12, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_PPU_V0_SYNQUACER,
                PPU_V0_ELEMENT_IDX_SYS3),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0_SYNQUACER, 0),
            .allowed_state_mask_table = systop_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_allowed_state_mask_table) }),
    },
    [PD_STATIC_DEV_IDX_CHILD_SYS1] = {
        .name = "SYS1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .tree_pos = MOD_PD_TREE_POS(MOD_PD_LEVEL_1, 0, 0, 13, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_PPU_V0_SYNQUACER,
                PPU_V0_ELEMENT_IDX_SYS1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0_SYNQUACER, 0),
            .allowed_state_mask_table = systop_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_allowed_state_mask_table) }),
    },
    [PD_STATIC_DEV_IDX_CHILD_SYS2] = {
        .name = "SYS2",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .tree_pos = MOD_PD_TREE_POS(MOD_PD_LEVEL_1, 0, 0, 14, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_PPU_V0_SYNQUACER,
                PPU_V0_ELEMENT_IDX_SYS2),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0_SYNQUACER, 0),
            .allowed_state_mask_table = systop_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_allowed_state_mask_table) }),
    },

    /* PPU_SYS4 is managed by romfw */

    [PD_STATIC_DEV_IDX_CHILD_DEBUG] = {
        .name = "DEBUG",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE_DEBUG,
            .tree_pos = MOD_PD_TREE_POS(MOD_PD_LEVEL_1, 0, 0, 16, 0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_PPU_V0_SYNQUACER,
                PPU_V0_ELEMENT_IDX_DEBUG),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0_SYNQUACER, 0),
            .allowed_state_mask_table = systop_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_allowed_state_mask_table) }),
    },
    [PD_STATIC_DEV_IDX_SYSTOP] = {
        .name = "SYSTOP",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_SYSTEM,
            .tree_pos = MOD_PD_TREE_POS(MOD_PD_LEVEL_2, 0, 0, 0, 0),
            .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SYSTEM_POWER),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_SYSTEM_POWER,
                MOD_SYSTEM_POWER_API_IDX_PD_DRIVER),
            .allowed_state_mask_table = systop_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_allowed_state_mask_table) }),
    },
    [PD_STATIC_DEV_IDX_COUNT] = { 0 }, /* Termination entry */
};

static const struct fwk_element *synquacer_power_domain_get_element_table(
    fwk_id_t module_id)
{
    struct fwk_element *element_table, *element;
    struct mod_power_domain_element_config *pd_config_table, *pd_config;
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
            FWK_ARRAY_SIZE(synquacer_power_domain_static_element_table) +
            1, /* Terminator */
        sizeof(struct fwk_element));

    pd_config_table = fwk_mm_calloc(
        (cluster_count + core_count),
        sizeof(struct mod_power_domain_element_config));

    /*
     * power domain element table should follow the ascending order
     * of tree position.
     * It means first element must be cluster0_core0,
     * last element should be system power element(SYSTOP)
     */
    /* prepare core config table */
    for (cluster_idx = 0; cluster_idx < cluster_count; cluster_idx++) {
        for (core_idx = 0; core_idx < core_per_cluster_count; core_idx++) {
            element = &element_table[element_count];
            pd_config = &pd_config_table[element_count];

            element->name = fwk_mm_alloc(PD_NAME_SIZE, 1);

            snprintf(
                (char *)element->name,
                PD_NAME_SIZE,
                "CLUS%uCORE%u",
                cluster_idx,
                core_idx);

            element->data = pd_config;

            pd_config->attributes.pd_type = MOD_PD_TYPE_CORE;
            pd_config->tree_pos =
                MOD_PD_TREE_POS(MOD_PD_LEVEL_0, 0, 0, cluster_idx, core_idx);
            pd_config->driver_id =
                FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V0_SYNQUACER, element_count);
            pd_config->api_id = FWK_ID_API(FWK_MODULE_IDX_PPU_V0_SYNQUACER, 0);
            pd_config->allowed_state_mask_table =
                core_pd_allowed_state_mask_table;
            pd_config->allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table);
            element_count++;
        }
    }

    /* prepare cluster config table */
    for (cluster_idx = 0; cluster_idx < cluster_count; cluster_idx++) {
        element = &element_table[element_count];
        pd_config = &pd_config_table[element_count];

        element->name = fwk_mm_alloc(PD_NAME_SIZE, 1);

        snprintf((char *)element->name, PD_NAME_SIZE, "CLUS%u", cluster_idx);

        element->data = pd_config;

        pd_config->attributes.pd_type = MOD_PD_TYPE_CLUSTER;
        pd_config->tree_pos =
            MOD_PD_TREE_POS(MOD_PD_LEVEL_1, 0, 0, cluster_idx, 0);
        pd_config->driver_id =
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PPU_V0_SYNQUACER, element_count);
        pd_config->api_id = FWK_ID_API(FWK_MODULE_IDX_PPU_V0_SYNQUACER, 0);
        pd_config->allowed_state_mask_table =
            cluster_pd_allowed_state_mask_table;
        pd_config->allowed_state_mask_table_size =
            FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table);
        element_count++;
    }

    memcpy(
        element_table + element_count,
        synquacer_power_domain_static_element_table,
        sizeof(synquacer_power_domain_static_element_table));

    return element_table;
}

/*
 * Power module configuration data
 */
const struct fwk_module_config config_power_domain = {
    .get_element_table = synquacer_power_domain_get_element_table,
    .data = &synquacer_power_domain_config,
};

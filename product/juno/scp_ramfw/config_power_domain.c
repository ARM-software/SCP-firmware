/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "juno_power_domain.h"
#include "juno_ppu_idx.h"

#include <mod_juno_ppu.h>
#include <mod_power_domain.h>
#include <mod_system_power.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/* Masks of the allowed states for the systop power domain */
static const uint32_t systop_pd_allowed_state_mask_table[] = {
    [0] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK |
        (1 << MOD_SYSTEM_POWER_POWER_STATE_SLEEP0)
};

/*
 * Masks of the allowed states for the top level power domains (gpu and debug
 * only) depending on the systop states.
 */
static const uint32_t toplevel_pd_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = MOD_PD_STATE_OFF_MASK,
};

/*
 * Masks of the allowed states for the cluster power domain depending on the
 * systop states.
 */
static const uint32_t cluster_pd_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_ON] = JUNO_CLUSTER_VALID_STATE_MASK,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = MOD_PD_STATE_OFF_MASK,
};

/* Masks of the allowed states for a core depending on the cluster states. */
static const uint32_t core_pd_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK,
    [MOD_PD_STATE_ON] = JUNO_CORE_VALID_STATE_MASK,
};

/* Tree indices for children of a cluster power domain */
enum tree_idx_cluster_child {
    TREE_IDX_CLUSTER_CHILD_CPU0,
    TREE_IDX_CLUSTER_CHILD_CPU1,
    TREE_IDX_CLUSTER_CHILD_CPU2,
    TREE_IDX_CLUSTER_CHILD_CPU3,
};

/* Tree indices for children of systop power domain */
enum tree_idx_systop_child {
    TREE_IDX_SYSTOP_CHILD_BIG,
    TREE_IDX_SYSTOP_CHILD_LITTLE,
    TREE_IDX_SYSTOP_CHILD_DBG,
    TREE_IDX_SYSTOP_CHILD_GPU,
};

static struct fwk_element element_table[] = {
    [POWER_DOMAIN_IDX_BIG_CPU0] = {
        .name = "BIG_CPU0",
        .data = &(struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0,
                0,
                0,
                TREE_IDX_SYSTOP_CHILD_BIG,
                TREE_IDX_CLUSTER_CHILD_CPU0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                JUNO_PPU_DEV_IDX_BIG_CPU0),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                MOD_JUNO_PPU_API_IDX_PD),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        },
    },
    [POWER_DOMAIN_IDX_BIG_CPU1] = {
        .name = "BIG_CPU1",
        .data = &(struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0,
                0,
                0,
                TREE_IDX_SYSTOP_CHILD_BIG,
                TREE_IDX_CLUSTER_CHILD_CPU1),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                JUNO_PPU_DEV_IDX_BIG_CPU1),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                MOD_JUNO_PPU_API_IDX_PD),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        },
    },
    [POWER_DOMAIN_IDX_LITTLE_CPU0] = {
        .name = "LITTLE_CPU0",
        .data = &(struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0,
                0,
                0,
                TREE_IDX_SYSTOP_CHILD_LITTLE,
                TREE_IDX_CLUSTER_CHILD_CPU0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                JUNO_PPU_DEV_IDX_LITTLE_CPU0),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                MOD_JUNO_PPU_API_IDX_PD),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        },
    },
    [POWER_DOMAIN_IDX_LITTLE_CPU1] = {
        .name = "LITTLE_CPU1",
        .data = &(struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0,
                0,
                0,
                TREE_IDX_SYSTOP_CHILD_LITTLE,
                TREE_IDX_CLUSTER_CHILD_CPU1),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                JUNO_PPU_DEV_IDX_LITTLE_CPU1),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                MOD_JUNO_PPU_API_IDX_PD),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        },
    },
    [POWER_DOMAIN_IDX_LITTLE_CPU2] = {
        .name = "LITTLE_CPU2",
        .data = &(struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0,
                0,
                0,
                TREE_IDX_SYSTOP_CHILD_LITTLE,
                TREE_IDX_CLUSTER_CHILD_CPU2),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                JUNO_PPU_DEV_IDX_LITTLE_CPU2),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                MOD_JUNO_PPU_API_IDX_PD),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        },
    },
    [POWER_DOMAIN_IDX_LITTLE_CPU3] = {
        .name = "LITTLE_CPU3",
        .data = &(struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_0,
                0,
                0,
                TREE_IDX_SYSTOP_CHILD_LITTLE,
                TREE_IDX_CLUSTER_CHILD_CPU3),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                JUNO_PPU_DEV_IDX_LITTLE_CPU3),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                MOD_JUNO_PPU_API_IDX_PD),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        },
    },
    [POWER_DOMAIN_IDX_BIG_SSTOP] = {
        .name = "BIG_SSTOP",
        .data = &(struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CLUSTER,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_1,
                0,
                0,
                TREE_IDX_SYSTOP_CHILD_BIG,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                JUNO_PPU_DEV_IDX_BIG_SSTOP),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                MOD_JUNO_PPU_API_IDX_PD),
            .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table),
        },
    },
    [POWER_DOMAIN_IDX_LITTLE_SSTOP] = {
        .name = "LITTLE_SSTOP",
        .data = &(struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CLUSTER,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_1,
                0,
                0,
                TREE_IDX_SYSTOP_CHILD_LITTLE,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                JUNO_PPU_DEV_IDX_LITTLE_SSTOP),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                MOD_JUNO_PPU_API_IDX_PD),
            .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table),
        },
    },
    [POWER_DOMAIN_IDX_GPUTOP] = {
        .name = "GPUTOP",
        .data = &(struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_1,
                0,
                0,
                TREE_IDX_SYSTOP_CHILD_GPU,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                JUNO_PPU_DEV_IDX_GPUTOP),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                MOD_JUNO_PPU_API_IDX_PD),
            .allowed_state_mask_table = toplevel_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_pd_allowed_state_mask_table),
        },
    },
    [POWER_DOMAIN_IDX_DBGSYS] = {
        .name = "DBGSYS",
        .data = &(struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE_DEBUG,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_1,
                0,
                0,
                TREE_IDX_SYSTOP_CHILD_DBG,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                JUNO_PPU_DEV_IDX_DBGSYS),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_JUNO_PPU,
                MOD_JUNO_PPU_API_IDX_PD),
            .allowed_state_mask_table = toplevel_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_pd_allowed_state_mask_table),
        },
    },
    [POWER_DOMAIN_IDX_SYSTOP] = {
        .name = "SYSTOP",
        .data = &(struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_SYSTEM,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2,
                0,
                0,
                0,
                0),
            .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SYSTEM_POWER),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_POWER,
                MOD_SYSTEM_POWER_API_IDX_PD_DRIVER),
            .allowed_state_mask_table = systop_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_pd_allowed_state_mask_table),
        },
    },
    [POWER_DOMAIN_IDX_COUNT] = { 0 },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_power_domain = {
    .get_element_table = get_element_table,
    .data = &(struct mod_power_domain_config){ 0 }
};

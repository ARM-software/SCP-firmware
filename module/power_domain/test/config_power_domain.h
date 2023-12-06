/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_POWER_DOMAIN_H
#define CONFIG_POWER_DOMAIN_H

#include <mod_power_domain.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/*
 * Power domain indices
 */
enum pd_idx {
    PD_IDX_CLUS0CORE0,
    PD_IDX_CLUS0CORE1,
    PD_IDX_CLUS1CORE0,
    PD_IDX_CLUS1CORE1,
    PD_IDX_CLUSTER0,
    PD_IDX_CLUSTER1,
    PD_IDX_SYSTOP,
    PD_IDX_COUNT,
    PD_IDX_NONE = UINT32_MAX
};

/* Mask of the allowed states for a core depending on the cluster states. */
static const uint32_t core_pd_allowed_state_mask_table[3] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK,
    [MOD_PD_STATE_ON] =
        MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK | MOD_PD_STATE_ON_MASK,
    [MOD_PD_STATE_SLEEP] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK,
};

/*
 * Mask of the allowed states for the cluster power domain depending on the
 * system states.
 */
static const uint32_t cluster_pd_allowed_state_mask_table[2] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK,
};

/* Mask of the allowed states for the systop power domain */
static const uint32_t systop_allowed_state_mask_table[1] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_ON_MASK,
};

static const struct fwk_element pd_element_table[PD_IDX_COUNT] = {
    [PD_IDX_CLUS0CORE0] = {
        .name = "CLUS0CORE0",
        .data = &((const struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .parent_idx = PD_IDX_CLUSTER0,
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        }),
    },
    [PD_IDX_CLUS0CORE1] = {
        .name = "CLUS0CORE1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .parent_idx = PD_IDX_CLUSTER0,
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        }),
    },
    [PD_IDX_CLUSTER0] = {
        .name = "CLUS0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CLUSTER,
            .parent_idx = PD_IDX_SYSTOP,
            .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table)
        }),
    },
    [PD_IDX_CLUS1CORE0] = {
        .name = "CLUS1CORE0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .parent_idx = PD_IDX_CLUSTER1,
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        }),
    },
    [PD_IDX_CLUS1CORE1] = {
        .name = "CLUS1CORE1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .parent_idx = PD_IDX_CLUSTER1,
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        }),
    },
    [PD_IDX_CLUSTER1] = {
        .name = "CLUS1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CLUSTER,
            .parent_idx = PD_IDX_SYSTOP,
            .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table)
        }),
    },
    [PD_IDX_SYSTOP] = {
        .name = "SYSTOP0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_SYSTEM,
            .parent_idx = PD_IDX_NONE,
            .allowed_state_mask_table = systop_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_allowed_state_mask_table),
        }),
    },
};

#endif /* CONFIG_POWER_DOMAIN_H */

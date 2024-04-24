/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
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

static const unsigned int pd_core_state_mapping[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_ON,
    [MOD_PD_STATE_SLEEP] = MOD_PD_STATE_SLEEP,
    [MOD_PD_STATE_OFF_0] = MOD_PD_STATE_SLEEP,
    [MOD_PD_STATE_OFF_1] = MOD_PD_STATE_SLEEP,
    [MOD_PD_STATE_OFF_2] = MOD_PD_STATE_SLEEP,
};

/* Mask of the core composite states */
const uint32_t core_composite_state_mask_table_UT[] = {
    MOD_PD_CS_STATE_MASK << MOD_PD_CS_LEVEL_0_STATE_SHIFT,
    MOD_PD_CS_STATE_MASK << MOD_PD_CS_LEVEL_1_STATE_SHIFT,
    MOD_PD_CS_STATE_MASK << MOD_PD_CS_LEVEL_2_STATE_SHIFT,
    MOD_PD_CS_STATE_MASK << MOD_PD_CS_LEVEL_3_STATE_SHIFT,
};

/* Mask of the allowed states for a core depending on the cluster states. */
static const uint32_t core_pd_allowed_state_mask_table[6] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK |
        MOD_PD_STATE_ON_MASK | MOD_PD_STATE_OFF_0_MASK |
        MOD_PD_STATE_OFF_1_MASK | MOD_PD_STATE_OFF_2_MASK,
    [MOD_PD_STATE_SLEEP] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK |
        MOD_PD_STATE_OFF_0_MASK | MOD_PD_STATE_OFF_1_MASK |
        MOD_PD_STATE_OFF_2_MASK,
    [MOD_PD_STATE_OFF_0] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK |
        MOD_PD_STATE_ON_MASK | MOD_PD_STATE_OFF_0_MASK |
        MOD_PD_STATE_OFF_1_MASK | MOD_PD_STATE_OFF_2_MASK,
    [MOD_PD_STATE_OFF_1] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK |
        MOD_PD_STATE_ON_MASK | MOD_PD_STATE_OFF_0_MASK |
        MOD_PD_STATE_OFF_1_MASK | MOD_PD_STATE_OFF_2_MASK,
    [MOD_PD_STATE_OFF_2] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK |
        MOD_PD_STATE_ON_MASK | MOD_PD_STATE_OFF_0_MASK |
        MOD_PD_STATE_OFF_1_MASK | MOD_PD_STATE_OFF_2_MASK,
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
            .pd_state_mapping_table = pd_core_state_mapping,
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
            .pd_state_mapping_table = pd_core_state_mapping,
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
            .pd_state_mapping_table = pd_core_state_mapping,
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
            .pd_state_mapping_table = pd_core_state_mapping,
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

static struct mod_power_domain_config mod_pd_config = {
    .enable_system_suspend_notification = true,
};

static const struct pd_ctx pd_ctx_config[PD_IDX_COUNT] = {
    [PD_IDX_CLUS0CORE0] = {
        .id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, PD_IDX_CLUS0CORE0),
        .config = pd_element_table[PD_IDX_CLUS0CORE0].data,
        .requested_state = MOD_PD_STATE_OFF,
        .state_requested_to_driver = MOD_PD_STATE_OFF,
        .current_state = MOD_PD_STATE_OFF,
        .parent = NULL,
        .allowed_state_mask_table = core_pd_allowed_state_mask_table,
        .allowed_state_mask_table_size =
            FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        .cs_support = true,
        .composite_state_mask_table = core_composite_state_mask_table_UT,
        .composite_state_mask_table_size =
            FWK_ARRAY_SIZE(core_composite_state_mask_table_UT),
    },
    [PD_IDX_CLUS0CORE1] = {
        .id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, PD_IDX_CLUS0CORE1),
        .config = pd_element_table[PD_IDX_CLUS0CORE1].data,
        .requested_state = MOD_PD_STATE_OFF,
        .state_requested_to_driver = MOD_PD_STATE_OFF,
        .current_state = MOD_PD_STATE_OFF,
        .parent = NULL,
        .allowed_state_mask_table = core_pd_allowed_state_mask_table,
        .allowed_state_mask_table_size =
            FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        .cs_support = true,
        .composite_state_mask_table = core_composite_state_mask_table_UT,
        .composite_state_mask_table_size =
            FWK_ARRAY_SIZE(core_composite_state_mask_table_UT),
    },
    [PD_IDX_CLUS1CORE0] = {
        .id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, PD_IDX_CLUS1CORE0),
        .config = pd_element_table[PD_IDX_CLUS1CORE0].data,
        .requested_state = MOD_PD_STATE_OFF,
        .state_requested_to_driver = MOD_PD_STATE_OFF,
        .current_state = MOD_PD_STATE_OFF,
        .parent = NULL,
        .allowed_state_mask_table = core_pd_allowed_state_mask_table,
        .allowed_state_mask_table_size =
            FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        .cs_support = true,
        .composite_state_mask_table = core_composite_state_mask_table_UT,
        .composite_state_mask_table_size =
            FWK_ARRAY_SIZE(core_composite_state_mask_table_UT),
    },
    [PD_IDX_CLUS1CORE1] = {
        .id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, PD_IDX_CLUS1CORE1),
        .config = pd_element_table[PD_IDX_CLUS1CORE1].data,
        .requested_state = MOD_PD_STATE_OFF,
        .state_requested_to_driver = MOD_PD_STATE_OFF,
        .current_state = MOD_PD_STATE_OFF,
        .parent = NULL,
        .allowed_state_mask_table = core_pd_allowed_state_mask_table,
        .allowed_state_mask_table_size =
            FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        .cs_support = true,
        .composite_state_mask_table = core_composite_state_mask_table_UT,
        .composite_state_mask_table_size =
            FWK_ARRAY_SIZE(core_composite_state_mask_table_UT),
    },
    [PD_IDX_CLUSTER0] = {
        .id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, PD_IDX_CLUSTER0),
        .config = pd_element_table[PD_IDX_CLUSTER0].data,
        .requested_state = MOD_PD_STATE_OFF,
        .state_requested_to_driver = MOD_PD_STATE_OFF,
        .current_state = MOD_PD_STATE_OFF,
        .parent = NULL,
        .cs_support = false,
        .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
        .allowed_state_mask_table_size =
            FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table),
    },
    [PD_IDX_CLUSTER1] = {
        .id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, PD_IDX_CLUSTER1),
        .config = pd_element_table[PD_IDX_CLUSTER1].data,
        .requested_state = MOD_PD_STATE_OFF,
        .state_requested_to_driver = MOD_PD_STATE_OFF,
        .current_state = MOD_PD_STATE_OFF,
        .parent = NULL,
        .cs_support = false,
        .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
        .allowed_state_mask_table_size =
            FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table),
    },
    [PD_IDX_SYSTOP] = {
        .id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, PD_IDX_SYSTOP),
        .config = pd_element_table[PD_IDX_SYSTOP].data,
        .parent = NULL,
        .cs_support = false,
        .allowed_state_mask_table = systop_allowed_state_mask_table,
        .allowed_state_mask_table_size =
            FWK_ARRAY_SIZE(systop_allowed_state_mask_table),
        .requested_state = MOD_PD_STATE_ON,
        .state_requested_to_driver = MOD_PD_STATE_ON,
        .current_state = MOD_PD_STATE_ON,
    },
};

#endif /* CONFIG_POWER_DOMAIN_H */

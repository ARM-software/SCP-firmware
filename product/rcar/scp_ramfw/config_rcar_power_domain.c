/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <string.h>
#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <config_rcar_power_domain.h>
#include <config_rcar_pd_sysc.h>
#include <config_rcar_pd_core.h>
#include <mod_system_power.h>
#include <mod_rcar_power_domain.h>
#include <mod_rcar_pd_core.h>
#include <rcar_core.h>

static const char *core_pd_name_table[RCAR_CORE_PER_CLUSTER_MAX] = {
    "CLUS0CORE0", "CLUS0CORE1", "CLUS0CORE2", "CLUS0CORE3",
    "CLUS1CORE0", "CLUS1CORE1", "CLUS1CORE2", "CLUS1CORE3",
};

/* Mask of the allowed states for the systop power domain */
static const uint32_t systop_allowed_state_mask_table[] = {
    [0] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK |
          (1 << MOD_SYSTEM_POWER_POWER_STATE_SLEEP0) |
          (1 << MOD_SYSTEM_POWER_POWER_STATE_SLEEP1)
};
/*
 * Mask of the allowed states for the top level power domains
 * (but the cluster power domains) depending on the system states.
 */
static const uint32_t toplevel_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = MOD_PD_STATE_OFF_MASK,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP1] = MOD_PD_STATE_OFF_MASK
};
/*
 * Mask of the allowed states for the cluster power domain depending on the
 * system states.
 */
static const uint32_t cluster_pd_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK |
        MOD_PD_STATE_SLEEP_MASK,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = MOD_PD_STATE_OFF_MASK,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP1] = MOD_PD_STATE_OFF_MASK
};

/* Mask of the allowed states for a core depending on the cluster states. */
static const uint32_t core_pd_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK |
        MOD_PD_STATE_SLEEP_MASK,
    [MOD_PD_STATE_SLEEP] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK,
};

/* Power module specific configuration data (none) */
static const struct mod_power_domain_config rcar_power_domain_config = { };

static struct fwk_element rcar_power_domain_static_element_table[] = {
    [CONFIG_POWER_DOMAIN_CHILD_CLUSTER0] = {
        .name = "CLUS0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CLUSTER,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2,
                0,
                0,
                CONFIG_POWER_DOMAIN_CHILD_CLUSTER0,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_PD_CORE,
                                  RCAR_PD_CORE_ELEMENT_IDX_CLU0),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_CORE,
                                  MOD_RCAR_PD_SYSC_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_CLUSTER1] = {
        .name = "CLUS1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CLUSTER,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2,
                0,
                0,
                CONFIG_POWER_DOMAIN_CHILD_CLUSTER1,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_PD_CORE,
                                  RCAR_PD_CORE_ELEMENT_IDX_CLU1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_CORE,
                                  MOD_RCAR_PD_SYSC_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_A3IR] = {
        .name = "a3ir",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2,
                0,
                0,
                CONFIG_POWER_DOMAIN_CHILD_A3IR,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_A3IR),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_3DGE] = {
        .name = "3dg-e",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2,
                0,
                0,
                CONFIG_POWER_DOMAIN_CHILD_3DGE,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_3DGE),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_3DGD] = {
        .name = "3dg-d",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2,
                0,
                0,
                CONFIG_POWER_DOMAIN_CHILD_3DGD,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_3DGD),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_3DGC] = {
        .name = "3dg-c",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2,
                0,
                0,
                CONFIG_POWER_DOMAIN_CHILD_3DGC,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_3DGC),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_3DGB] = {
        .name = "3dg-b",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2,
                0,
                0,
                CONFIG_POWER_DOMAIN_CHILD_3DGB,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_3DGB),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_3DGA] = {
        .name = "3dg-a",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2,
                0,
                0,
                CONFIG_POWER_DOMAIN_CHILD_3DGA,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_3DGA),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_A2VC1] = {
        .name = "a2vc1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2,
                0,
                0,
                CONFIG_POWER_DOMAIN_CHILD_A2VC1,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_A2VC1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_A3VC] = {
        .name = "a3vc",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2,
                0,
                0,
                CONFIG_POWER_DOMAIN_CHILD_A3VC,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_A3VC),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_CR7] = {
        .name = "cr7",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2,
                0,
                0,
                CONFIG_POWER_DOMAIN_CHILD_CR7,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_CR7),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_A3VP] = {
        .name = "a3vp",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2,
                0,
                0,
                CONFIG_POWER_DOMAIN_CHILD_A3VP,
                0),
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_A3VP),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_COUNT] = {
        .name = "SYSTOP",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_SYSTEM,
            .tree_pos = MOD_PD_TREE_POS(
                MOD_PD_LEVEL_2, 0, 0, 0, 0),
            .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SYSTEM_POWER),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_POWER,
                MOD_SYSTEM_POWER_API_IDX_PD_DRIVER),
            .allowed_state_mask_table = systop_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_allowed_state_mask_table)
        }),
    },
};

/*
 * Function definitions with internal linkage
 */
static const struct fwk_element *rcar_power_domain_get_element_table
    (fwk_id_t module_id)
{
    struct fwk_element *element_table, *element;
    struct mod_power_domain_element_config *pd_config_table, *pd_config;
    unsigned int core_idx;

    element_table = fwk_mm_calloc(
        rcar_core_get_count()
        + FWK_ARRAY_SIZE(rcar_power_domain_static_element_table)
        + 1, /* Terminator */
        sizeof(struct fwk_element));
    if (element_table == NULL)
        return NULL;

    pd_config_table = fwk_mm_calloc(rcar_core_get_count(),
        sizeof(struct mod_power_domain_element_config));
    if (pd_config_table == NULL)
        return NULL;

    for (core_idx = 0; core_idx < rcar_core_get_count(); core_idx++) {
        element = &element_table[core_idx];
        pd_config = &pd_config_table[core_idx];

        element->name = core_pd_name_table[core_idx];
        element->data = pd_config;

        pd_config->attributes.pd_type = MOD_PD_TYPE_CORE,
        pd_config->tree_pos = MOD_PD_TREE_POS(
            MOD_PD_LEVEL_2,
            0,
            0,
            CONFIG_POWER_DOMAIN_CHILD_CLUSTER0,
            core_idx),
        pd_config->driver_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_RCAR_PD_CORE,
            core_idx),
        pd_config->api_id = FWK_ID_API(
            FWK_MODULE_IDX_RCAR_PD_CORE,
            MOD_RCAR_PD_SYSC_API_IDX_POWER_DOMAIN_DRIVER),
        pd_config->allowed_state_mask_table = core_pd_allowed_state_mask_table,
        pd_config->allowed_state_mask_table_size =
            FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table);
    }

    memcpy(element_table + rcar_core_get_count(),
           rcar_power_domain_static_element_table,
           sizeof(rcar_power_domain_static_element_table));

    return element_table;
}

/*
 * Power module configuration data
 */
struct fwk_module_config config_rcar_power_domain = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(rcar_power_domain_get_element_table),
    .data = &rcar_power_domain_config,
};

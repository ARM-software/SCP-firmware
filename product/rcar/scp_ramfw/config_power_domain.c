/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_power_domain.h>
#include <config_rcar_pd_sysc.h>
#include <config_rcar_pd_core.h>
#include <config_rcar_pd_pmic_bd9571.h>
#include <power_domain_utils.h>
#include <rcar_core.h>

#include <mod_system_power.h>
#include <mod_power_domain.h>
#include <mod_rcar_pd_core.h>
#include <mod_rcar_pd_sysc.h>
#include <mod_rcar_pd_pmic_bd9571.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>
#include <string.h>

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
static const struct mod_power_domain_config rcar_power_domain_config = { 0 };

static struct fwk_element rcar_power_domain_static_element_table[] = {
    [CONFIG_POWER_DOMAIN_CHILD_A3IR] = {
        .name = "a3ir",
        .data = &((struct mod_power_domain_element_config){
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_A3IR),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC,
                MOD_RCAR_PD_SYSC_API_TYPE_PUBLIC),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_3DGE] = {
        .name = "3dg-e",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_3DGE),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC,
                MOD_RCAR_PD_SYSC_API_TYPE_PUBLIC),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_3DGD] = {
        .name = "3dg-d",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_3DGD),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC,
                MOD_RCAR_PD_SYSC_API_TYPE_PUBLIC),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_3DGC] = {
        .name = "3dg-c",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_3DGC),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC,
                MOD_RCAR_PD_SYSC_API_TYPE_PUBLIC),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_3DGB] = {
        .name = "3dg-b",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_3DGB),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC,
                MOD_RCAR_PD_SYSC_API_TYPE_PUBLIC),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_3DGA] = {
        .name = "3dg-a",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_3DGA),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC,
                MOD_RCAR_PD_SYSC_API_TYPE_PUBLIC),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_A2VC1] = {
        .name = "a2vc1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_A2VC1),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC,
                MOD_RCAR_PD_SYSC_API_TYPE_PUBLIC),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_A3VC] = {
        .name = "a3vc",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_A3VC),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC,
                MOD_RCAR_PD_SYSC_API_TYPE_PUBLIC),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_CR7] = {
        .name = "cr7",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_CR7),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC,
                MOD_RCAR_PD_SYSC_API_TYPE_PUBLIC),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_A3VP] = {
        .name = "a3vp",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC, RCAR_PD_SYSC_ELEMENT_IDX_A3VP),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC,
                MOD_RCAR_PD_SYSC_API_TYPE_PUBLIC),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_DDR_BKUP] = {
        .name = "pmic_ddr_bkup",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_PMIC_BD9571,
                RCAR_PD_PMIC_ELEMENT_IDX_PMIC_DDR_BKUP),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_PMIC_BD9571,
                MOD_RCAR_PD_PMIC_API_TYPE_PUBLIC),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_ALWAYS_ON] = {
        .name = "always-on",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_RCAR_PD_SYSC,
                RCAR_PD_SYSC_ELEMENT_IDX_ALWAYS_ON),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PD_SYSC,
                MOD_RCAR_PD_SYSC_API_TYPE_PUBLIC),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_CHILD_SYSTOP] = {
        .name = "SYSTOP",
        .data = &(struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_SYSTEM,
            .parent_idx = CONFIG_POWER_DOMAIN_CHILD_NONE,
            .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SYSTEM_POWER),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_SYSTEM_POWER,
                MOD_SYSTEM_POWER_API_IDX_PD_DRIVER),
            .allowed_state_mask_table = systop_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_allowed_state_mask_table),
        },
    },
    [CONFIG_POWER_DOMAIN_CHILD_COUNT] = { 0 },
};

/*
 * Function definitions with internal linkage
 */
static const struct fwk_element *rcar_power_domain_get_element_table
    (fwk_id_t module_id)
{
    return create_power_domain_element_table(
        rcar_core_get_count(),
        rcar_cluster_get_count(),
        FWK_MODULE_IDX_RCAR_PD_CORE,
        0,
        core_pd_allowed_state_mask_table,
        FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        cluster_pd_allowed_state_mask_table,
        FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table),
        rcar_power_domain_static_element_table,
        FWK_ARRAY_SIZE(rcar_power_domain_static_element_table));
}

/*
 * Power module configuration data
 */
struct fwk_module_config config_power_domain = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(rcar_power_domain_get_element_table),
    .data = &rcar_power_domain_config,
};

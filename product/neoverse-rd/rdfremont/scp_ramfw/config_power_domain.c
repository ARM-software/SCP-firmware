/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'power_domain'.
 */

#include "platform_core.h"
#include "scp_cfgd_power_domain.h"

#include <power_domain_utils.h>

#include <mod_power_domain.h>
#include <mod_ppu_v1.h>
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

/* Module 'power_domain' static element count */
#define PD_STATIC_ELEMENT_COUNT (PD_STATIC_DEV_IDX_SYSTOP + 1)

/* Allowed PD state count */
#define SYSTOP_PD_STATE_COUNT 1
#define CLUS_PD_STATE_COUNT   2
#define CORE_PD_STATE_COUNT   2

/* Maximum power domain name size including the null terminator */
#define PD_NAME_SIZE 12

/* Mask for the cluster valid power states */
#define CLUSTER_VALID_STATE_MASK (MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK)

/* Mask for the core valid power states */
#define CORE_VALID_STATE_MASK (MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK)

/* Mask of the allowed states for the systop power domain */
static const uint32_t systop_allowed_state_mask_table[SYSTOP_PD_STATE_COUNT] = {
    [0] = MOD_PD_STATE_ON_MASK
};

/*
 * Mask of the allowed states for the cluster power domain depending on the
 * system states.
 */
static const uint32_t
    cluster_pd_allowed_state_mask_table[CLUS_PD_STATE_COUNT] = {
        [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
        [MOD_PD_STATE_ON] = CLUSTER_VALID_STATE_MASK,
    };

/* Mask of the allowed states for a core depending on the cluster states. */
static const uint32_t core_pd_allowed_state_mask_table[CORE_PD_STATE_COUNT] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK,
    [MOD_PD_STATE_ON] = CORE_VALID_STATE_MASK,
};

/* Power module specific configuration data (none) */
static const struct mod_power_domain_config platform_power_domain_config = {
    0
};

static struct fwk_element pd_static_element_table[PD_STATIC_ELEMENT_COUNT] = {
    [PD_STATIC_DEV_IDX_SYSTOP] = {
        .name = "SYSTOP",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_SYSTEM,
            .parent_idx = PD_STATIC_DEV_IDX_NONE,
            .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SYSTEM_POWER),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_SYSTEM_POWER,
                MOD_SYSTEM_POWER_API_IDX_PD_DRIVER),
            .allowed_state_mask_table = systop_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_allowed_state_mask_table)
        }),
    },
};

static const struct fwk_element *platform_power_domain_get_element_table(
    fwk_id_t module_id)
{
    return create_power_domain_element_table(
        platform_get_core_count(),
        platform_get_cluster_count(),
        FWK_MODULE_IDX_PPU_V1,
        MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER,
        core_pd_allowed_state_mask_table,
        FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        cluster_pd_allowed_state_mask_table,
        FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table),
        pd_static_element_table,
        FWK_ARRAY_SIZE(pd_static_element_table));
}

const struct fwk_module_config config_power_domain = {
    .data = &platform_power_domain_config,
    .elements =
        FWK_MODULE_DYNAMIC_ELEMENTS(platform_power_domain_get_element_table),
};

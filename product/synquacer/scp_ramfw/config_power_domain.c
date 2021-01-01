/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "config_ppu_v0.h"
#include "synquacer_core.h"

#include <power_domain_utils.h>

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
            .parent_idx = PD_STATIC_DEV_IDX_NONE,
            .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SYSTEM_POWER),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_SYSTEM_POWER,
                MOD_SYSTEM_POWER_API_IDX_PD_DRIVER),
            .allowed_state_mask_table = systop_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_allowed_state_mask_table) }),
    },
};

static const struct fwk_element *synquacer_power_domain_get_element_table(
    fwk_id_t module_id)
{
    return create_power_domain_element_table(
        synquacer_core_get_core_count(),
        synquacer_core_get_cluster_count(),
        FWK_MODULE_IDX_PPU_V0_SYNQUACER,
        0,
        core_pd_allowed_state_mask_table,
        FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        cluster_pd_allowed_state_mask_table,
        FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table),
        synquacer_power_domain_static_element_table,
        FWK_ARRAY_SIZE(synquacer_power_domain_static_element_table));
}

/*
 * Power module configuration data
 */
const struct fwk_module_config config_power_domain = {
    .data = &synquacer_power_domain_config,
    .elements =
        FWK_MODULE_DYNAMIC_ELEMENTS(synquacer_power_domain_get_element_table),
};

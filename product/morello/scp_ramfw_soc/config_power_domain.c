/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <config_power_domain.h>
#include <config_ppu_v0.h>
#include <morello_core.h>
#include <morello_power_domain.h>

#include <mod_power_domain.h>
#include <mod_ppu_v1.h>
#include <mod_system_power.h>

#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Maximum power domain name size including the null terminator */
#define PD_NAME_SIZE 16

/* Mask of the allowed states for the systop power domain */
static const uint32_t systop_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK |
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
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK,
    [MOD_PD_STATE_ON] = MORELLO_CLUSTER_VALID_STATE_MASK,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP0] = MOD_PD_STATE_OFF_MASK,
    [MOD_SYSTEM_POWER_POWER_STATE_SLEEP1] = MOD_PD_STATE_OFF_MASK
};

/* Mask of the allowed states for a core depending on the cluster states. */
static const uint32_t core_pd_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK,
    [MOD_PD_STATE_ON] = MORELLO_CORE_VALID_STATE_MASK,
    [MOD_PD_STATE_SLEEP] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK,
    [MORELLO_POWER_DOMAIN_STATE_FUNC_RET] = MORELLO_CORE_VALID_STATE_MASK,
    [MORELLO_POWER_DOMAIN_STATE_MEM_RET] = MOD_PD_STATE_OFF_MASK
};

/* Power module specific configuration data (none) */
static const struct mod_power_domain_config morello_power_domain_config = { 0 };

/* Power domain element table pointer */
struct fwk_element *element_table = NULL;

/* Power domain element configuration table pointer */
struct mod_power_domain_element_config *pd_config_table = NULL;

/*
 * The SCP's view of PD tree in a single chip mode looks like below:
 *
 *            -------------SYSTOP0----------------
 *           /        /       |        \          \
 *          /        /        |         \          \
 *         /        /         |          \          \
 *    CLUS0       CLUS1    DBGTOP0    GPUTOP0    DPUTOP0
 *   /    \      /    \
 * CPU0--CPU1--CPU2--CPU3
 *
 */

static struct fwk_element
        morello_pd_single_chip_element_table[PD_SINGLE_CHIP_IDX_COUNT + 1] = {
    [PD_SINGLE_CHIP_IDX_CLUS0CORE0] = {
        .name = "CLUS0CORE0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .parent_idx = PD_SINGLE_CHIP_IDX_CLUSTER0,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 0),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_CLUS0CORE1] = {
        .name = "CLUS0CORE1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .parent_idx = PD_SINGLE_CHIP_IDX_CLUSTER0,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 1),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_CLUS1CORE0] = {
        .name = "CLUS1CORE0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .parent_idx = PD_SINGLE_CHIP_IDX_CLUSTER1,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 2),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_CLUS1CORE1] = {
        .name = "CLUS1CORE1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CORE,
            .parent_idx = PD_SINGLE_CHIP_IDX_CLUSTER1,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 3),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = core_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_CLUSTER0] = {
        .name = "CLUS0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CLUSTER,
            .parent_idx = PD_SINGLE_CHIP_IDX_SYSTOP0,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 4),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_CLUSTER1] = {
        .name = "CLUS1",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_CLUSTER,
            .parent_idx = PD_SINGLE_CHIP_IDX_SYSTOP0,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 5),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = cluster_pd_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_DBGTOP0] = {
        .name = "DBGTOP0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE_DEBUG,
            .parent_idx = PD_SINGLE_CHIP_IDX_SYSTOP0,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_PPU_V0, PPU_V0_ELEMENT_IDX_DBGTOP),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PPU_V0, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_GPUTOP0] = {
        .name = "GPUTOP0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .parent_idx = PD_SINGLE_CHIP_IDX_SYSTOP0,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 8),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_DPUTOP0] = {
        .name = "DPUTOP0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .parent_idx = PD_SINGLE_CHIP_IDX_SYSTOP0,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PPU_V1, 9),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_PPU_V1,
                MOD_PPU_V1_API_IDX_POWER_DOMAIN_DRIVER),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_SYSTOP0] = {
        .name = "SYSTOP0",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_SYSTEM,
            .parent_idx = PD_SINGLE_CHIP_IDX_NONE,
            .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SYSTEM_POWER),
            .api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_SYSTEM_POWER,
                MOD_SYSTEM_POWER_API_IDX_PD_DRIVER),
            .allowed_state_mask_table = systop_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(systop_allowed_state_mask_table)
        }),
    },
    [PD_SINGLE_CHIP_IDX_COUNT] = { 0 },
};

/*
 * Function definitions with internal linkage
 */
static const struct fwk_element *morello_power_domain_get_element_table(
    fwk_id_t module_id)
{
    return morello_pd_single_chip_element_table;
}

/*
 * Power module configuration data
 */
const struct fwk_module_config config_power_domain = {
    .elements =
        FWK_MODULE_DYNAMIC_ELEMENTS(morello_power_domain_get_element_table),
    .data = &morello_power_domain_config,
};

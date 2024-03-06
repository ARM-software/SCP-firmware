/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_mock_ppu.h"
#include "config_power_domain.h"

#include <power_domain_utils.h>

#include <mod_power_domain.h>

#include <fwk_module.h>
#include <fwk_module_idx.h>

/*
 * Mask of the allowed states for the top level power domains
 * (but the cluster power domains) depending on the system states.
 */
static const uint32_t toplevel_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK,
};

/*
 * Mask of the allowed states for the cluster power domain depending on the
 * system states.
 *
 * While the clusters may reach a SLEEP state, SLEEP does not appear in this
 * table. This is because the PPU driver backing the clusters will not accept a
 * manual SLEEP request, but will transition to it automatically when possible.
 */
static const uint32_t cluster_pd_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_ON] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK,
};

/* Mask of the allowed states for a core depending on the cluster states. */
static const uint32_t core_pd_allowed_state_mask_table[] = {
    [MOD_PD_STATE_OFF] = MOD_PD_STATE_OFF_MASK,
    [MOD_PD_STATE_SLEEP] = MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_SLEEP_MASK,
    [MOD_PD_STATE_ON] =
        MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK | MOD_PD_STATE_SLEEP_MASK,
};

/* Power module specific configuration data (none) */
static const struct mod_power_domain_config fvp_power_domain_config = { 0 };

static struct fwk_element fvp_power_domain_static_element_table[] = {
     [CONFIG_POWER_DOMAIN_SYSTOP_CHILD_DBGTOP] = {
        .name = "DBGTOP",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PPU, MOCK_PPU_ELEMENT_IDX_DBGTOP),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MOCK_PPU, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_SYSTOP_CHILD_DPU0TOP] = {
        .name = "DPU0TOP",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PPU, MOCK_PPU_ELEMENT_IDX_DPU0TOP),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MOCK_PPU, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_SYSTOP_CHILD_DPU1TOP] = {
        .name = "DPU1TOP",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PPU, MOCK_PPU_ELEMENT_IDX_DPU1TOP),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MOCK_PPU, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_SYSTOP_CHILD_GPUTOP] = {
        .name = "GPUTOP",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PPU, MOCK_PPU_ELEMENT_IDX_GPUTOP),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MOCK_PPU, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_SYSTOP_CHILD_VPUTOP] = {
        .name = "VPUTOP",
        .data = &((struct mod_power_domain_element_config) {
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PPU, MOCK_PPU_ELEMENT_IDX_VPUTOP),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MOCK_PPU, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
    [CONFIG_POWER_DOMAIN_SYSTOP_CHILD_COUNT] = {
        .name = "SYSTOP",
        .data = &((struct mod_power_domain_element_config) {
            .parent_idx = UINT32_MAX,
            .attributes.pd_type = MOD_PD_TYPE_DEVICE,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PPU, MOCK_PPU_ELEMENT_IDX_SYSTEM),
            .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MOCK_PPU, 0),
            .allowed_state_mask_table = toplevel_allowed_state_mask_table,
            .allowed_state_mask_table_size =
                FWK_ARRAY_SIZE(toplevel_allowed_state_mask_table)
        }),
    },
};

/*
 * Function definitions with internal linkage
 */
static const struct fwk_element *fvp_power_domain_get_element_table(
    fwk_id_t module_id)
{
    const struct fwk_element *element_table;

    element_table = create_power_domain_element_table(
        4,
        2,
        FWK_MODULE_IDX_MOCK_PPU,
        0,
        core_pd_allowed_state_mask_table,
        FWK_ARRAY_SIZE(core_pd_allowed_state_mask_table),
        cluster_pd_allowed_state_mask_table,
        FWK_ARRAY_SIZE(cluster_pd_allowed_state_mask_table),
        fvp_power_domain_static_element_table,
        FWK_ARRAY_SIZE(fvp_power_domain_static_element_table));

    return element_table;
}

/*
 * Power module configuration data
 */
struct fwk_module_config config_power_domain = {
    .data = &fvp_power_domain_config,
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(fvp_power_domain_get_element_table),
};

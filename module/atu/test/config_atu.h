/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_atu.h>

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

struct atu_reg dummy_atu_reg = {
    /* Set default values for ATU build configuration register */
    .ATUBC = 0x5d5
};

const struct atu_region_map atu_regions[1] = {
    [0] = {
        .region_owner_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_ATU),
        .log_addr_base = 0x60000000,
        .phy_addr_base = 0x200000000,
        .region_size = (256 * FWK_MIB),
        .attributes = ATU_ENCODE_ATTRIBUTES_SECURE_PAS,
    },
};

static const struct fwk_element element_table[] = {
    [0] = {
        .name = "SCP_ATU",
        .data = &(struct mod_atu_device_config) {
            .atu_base = (uintptr_t)&dummy_atu_reg,
            .atu_region_config_table = atu_regions,
            .atu_region_count = FWK_ARRAY_SIZE(atu_regions),
        },
    },
    [1] = { 0 },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

struct fwk_module_config config_atu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
};

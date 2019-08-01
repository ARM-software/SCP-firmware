/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_element.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_juno_xrp7724.h>
#include <mod_psu.h>
#include <config_juno_xrp7724.h>
#include <config_psu.h>

static struct fwk_element psu_dev_desc_table[] = {
    [MOD_PSU_ELEMENT_IDX_VSYS] = {
        .name = "VSYS",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_XRP7724,
                MOD_JUNO_XRP7724_ELEMENT_IDX_PSU_VSYS),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_XRP7724,
                MOD_JUNO_XRP7724_API_IDX_PSU),
        },
    },
    [MOD_PSU_ELEMENT_IDX_VBIG] = {
        .name = "VBIG",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_XRP7724,
                MOD_JUNO_XRP7724_ELEMENT_IDX_PSU_VBIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_XRP7724,
                MOD_JUNO_XRP7724_API_IDX_PSU),
        },
    },
    [MOD_PSU_ELEMENT_IDX_VLITTLE] = {
        .name = "VLITLLE",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_XRP7724,
                MOD_JUNO_XRP7724_ELEMENT_IDX_PSU_VLITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_XRP7724,
                MOD_JUNO_XRP7724_API_IDX_PSU),
        },
    },
    [MOD_PSU_ELEMENT_IDX_VGPU] = {
        .name = "VGPU",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_JUNO_XRP7724,
                MOD_JUNO_XRP7724_ELEMENT_IDX_PSU_VGPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_JUNO_XRP7724,
                MOD_JUNO_XRP7724_API_IDX_PSU),
        },
    },
    [MOD_PSU_ELEMENT_IDX_COUNT] = { 0 },
};

static const struct fwk_element *psu_get_dev_desc_table(fwk_id_t module_id)
{
    return psu_dev_desc_table;
}

struct fwk_module_config config_psu = {
    .get_element_table = psu_get_dev_desc_table,
};

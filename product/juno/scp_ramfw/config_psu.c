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
#include <mod_mock_psu.h>
#include <mod_psu.h>
#include <config_juno_xrp7724.h>

/* When running on a model, use the mock_psu */
static struct fwk_element psu_element_table_fvp[] = {
    [0] = {
        .name = "Fake-VSYS",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PSU, 0),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_API_IDX_DRIVER),
        },
    },
    [1] = {
        .name = "Fake-VBIG",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PSU, 0),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_API_IDX_DRIVER),
        },
    },
    [2] = {
        .name = "Fake-VLITTLE",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PSU, 0),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_API_IDX_DRIVER),
        },
    },
    [3] = {
        .name = "Fake-VGPU",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PSU, 0),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_API_IDX_DRIVER),
        },
    },
    [4] = { 0 } /* Termination description */
};

static const struct fwk_element *psu_get_dev_desc_table(fwk_id_t module_id)
{
    return psu_element_table_fvp;
}

struct fwk_module_config config_psu = {
    .get_element_table = psu_get_dev_desc_table,
    .data = NULL,
};

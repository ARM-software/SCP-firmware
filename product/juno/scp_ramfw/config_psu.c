/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_juno_xrp7724.h"
#include "config_psu.h"
#include "juno_id.h"

#include <mod_juno_xrp7724.h>
#include <mod_mock_psu.h>
#include <mod_psu.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>

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
    int status;
    enum juno_idx_platform platform_id = JUNO_IDX_PLATFORM_COUNT;

    status = juno_id_get_platform(&platform_id);
    if (!fwk_expect(status == FWK_SUCCESS))
        return NULL;

    if (platform_id == JUNO_IDX_PLATFORM_FVP)
        return psu_element_table_fvp;
    else
        return psu_dev_desc_table;
}

struct fwk_module_config config_psu = {
    .get_element_table = psu_get_dev_desc_table,
};

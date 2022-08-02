/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if (PLATFORM_VARIANT == JUNO_VARIANT_BOARD)
#    include "config_juno_xrp7724.h"

#    include <mod_juno_xrp7724.h>
#endif

#include "config_mock_psu.h"
#include "config_psu.h"
#include "juno_id.h"

#include <mod_mock_psu.h>
#include <mod_psu.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>

#if (PLATFORM_VARIANT == JUNO_VARIANT_BOARD)
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

#elif (PLATFORM_VARIANT == JUNO_VARIANT_FVP)

/* When running on a model, use the mock_psu */
static struct fwk_element psu_dev_desc_table[] = {
    [MOD_PSU_ELEMENT_IDX_VSYS] = {
        .name = "Fake-VSYS",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_ELEMENT_IDX_VSYS),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_API_IDX_DRIVER),
        },
    },
    [MOD_PSU_ELEMENT_IDX_VBIG] = {
        .name = "Fake-VBIG",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_ELEMENT_IDX_VBIG),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_API_IDX_DRIVER),
        },
    },
    [MOD_PSU_ELEMENT_IDX_VLITTLE] = {
        .name = "Fake-VLITTLE",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_ELEMENT_IDX_VLITTLE),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_API_IDX_DRIVER),
        },
    },
    [MOD_PSU_ELEMENT_IDX_VGPU] = {
        .name = "Fake-VGPU",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_ELEMENT_IDX_VGPU),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_API_IDX_DRIVER),
        },
    },
    [MOD_PSU_ELEMENT_IDX_COUNT] = { 0 } /* Termination description */
};

#endif

static const struct fwk_element *psu_get_dev_desc_table(fwk_id_t module_id)
{
    return psu_dev_desc_table;
}

struct fwk_module_config config_psu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(psu_get_dev_desc_table),
};

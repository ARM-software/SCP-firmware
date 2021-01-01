/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_rcar_pmic.h>
#include <mod_psu.h>
#include <config_rcar_pmic.h>
#include <config_psu.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>

static struct fwk_element psu_dev_desc_table[] = {
    [MOD_PSU_ELEMENT_IDX_VLITTLE] = {
        .name = "VLITTLE",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_PMIC,
                MOD_RCAR_PMIC_ELEMENT_IDX_LITTLE),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PMIC,
                MOD_RCAR_PMIC_API_IDX_DEVICE),
        },
    },
    [MOD_PSU_ELEMENT_IDX_VBIG] = {
        .name = "VBIG",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_PMIC,
                MOD_RCAR_PMIC_ELEMENT_IDX_BIG),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PMIC,
                MOD_RCAR_PMIC_API_IDX_DEVICE),
        },
    },
    [MOD_PSU_ELEMENT_IDX_VGPU] = {
        .name = "VGPU",
        .data = &(const struct mod_psu_element_cfg) {
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_RCAR_PMIC,
                MOD_RCAR_PMIC_ELEMENT_IDX_GPU),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_RCAR_PMIC,
                MOD_RCAR_PMIC_API_IDX_DEVICE),
        },
    },
    [MOD_PSU_ELEMENT_IDX_COUNT] = { 0 },
};

static const struct fwk_element *psu_get_dev_desc_table(fwk_id_t module_id)
{
    return psu_dev_desc_table;
}

struct fwk_module_config config_psu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(psu_get_dev_desc_table),
};

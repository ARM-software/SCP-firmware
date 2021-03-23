/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_mock_psu.h>
#include <mod_psu.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#define PSU_ELEMENT_IDX(n) \
    [n] = { .name = "PSU_GROUP" #n, \
            .data = &(const struct mod_psu_element_cfg){ \
                .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MOCK_PSU, n), \
                .driver_api_id = FWK_ID_API_INIT( \
                    FWK_MODULE_IDX_MOCK_PSU, MOD_MOCK_PSU_API_IDX_DRIVER) } }

static const struct fwk_element element_table[] = {
    PSU_ELEMENT_IDX(0),
    PSU_ELEMENT_IDX(1),
    PSU_ELEMENT_IDX(2),
    PSU_ELEMENT_IDX(3),
    { 0 },
};

static const struct fwk_element *psu_get_element_table(fwk_id_t module_id)
{
    return element_table;
}

const struct fwk_module_config config_psu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(psu_get_element_table),
};

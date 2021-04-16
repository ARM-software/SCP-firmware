/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_dwt_pmi.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

struct fwk_module_config config_pmi = {
    .elements = { 0 },
    .data = &((const struct mod_pmi_driver_config){
        .driver_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_DWT_PMI),
        .driver_api_id = FWK_ID_API_INIT(
            FWK_MODULE_IDX_DWT_PMI,
            MOD_DWT_PMI_API_IDX_DRIVER) }),
};

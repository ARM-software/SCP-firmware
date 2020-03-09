/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_ssc.h>
#include <mod_system_info.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stddef.h>

const struct fwk_module_config config_system_info = {
    .get_element_table = NULL,
    .data = &((struct mod_system_info_config) {
            .system_info_driver_module_id =
                FWK_ID_MODULE_INIT(FWK_MODULE_IDX_SSC),
            .system_info_driver_data_api_id =
                FWK_ID_API_INIT(FWK_MODULE_IDX_SSC,
                                MOD_SSC_SYSTEM_INFO_DRIVER_DATA_API_IDX),
    }),
};

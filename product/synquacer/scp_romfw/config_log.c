/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_log.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct mod_log_config log_data = {
    .device_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_F_UART3, 0),
    .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_F_UART3, 0),
};

struct fwk_module_config config_log = {
    .data = &log_data,
};

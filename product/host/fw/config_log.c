/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_log.h>

#include <fwk_banner.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

/*
 * Log module
 */
static const struct mod_log_config log_data = {
    .device_id = FWK_ID_MODULE_INIT(FWK_MODULE_IDX_HOST_CONSOLE),
    .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_HOST_CONSOLE, 0),
    .banner = FWK_BANNER_SCP "Host Firmware" BUILD_VERSION_DESCRIBE_STRING,
};

const struct fwk_module_config config_log = {
    .data = &log_data,
};

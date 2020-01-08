/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_banner.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_log.h>

/*
 * Log module
 */
static const struct mod_log_config log_data = {
    .device_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_PL011, 0),
    .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_PL011, 0),
    .log_groups = MOD_LOG_GROUP_ERROR |
                  MOD_LOG_GROUP_INFO |
                  MOD_LOG_GROUP_WARNING |
                  MOD_LOG_GROUP_DEBUG,
    .banner = FWK_BANNER_SCP
              FWK_BANNER_RAM_FIRMWARE
              BUILD_VERSION_DESCRIBE_STRING "\n",
};

struct fwk_module_config config_log = {
    .data = &log_data,
};

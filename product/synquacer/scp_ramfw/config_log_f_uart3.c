/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_banner.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_f_uart3.h>
#include <mod_log.h>
#include <synquacer_mmap.h>

/*
 * F_UART3 module
 */
static const struct fwk_element f_uart3_element_desc_table[] = {
    [0] = {
        .name = "F_UART3",
        .data = &((struct mod_f_uart3_device_config) {
            .reg_base = F_UART3_BASE_ADDR,
            .dla_reg_base = F_UART3_BASE_ADDR,
            .baud_rate_bps = 115200,
            .clock_rate_hz = 62500 * FWK_KHZ,
            .clock_id = FWK_ID_NONE_INIT,
        }),
    },
    [1] = { 0 },
};

static const struct fwk_element *get_f_uart3_table(fwk_id_t module_id)
{
    return f_uart3_element_desc_table;
}

struct fwk_module_config config_f_uart3 = {
    .get_element_table = get_f_uart3_table,
};

/*
 * Log module
 */
static const struct mod_log_config log_data = {
    .device_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_F_UART3, 0),
    .api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_F_UART3, 0),
    .log_groups = MOD_LOG_GROUP_ERROR | MOD_LOG_GROUP_INFO |
        MOD_LOG_GROUP_WARNING | MOD_LOG_GROUP_DEBUG,
    .banner =
        FWK_BANNER_SCP FWK_BANNER_RAM_FIRMWARE BUILD_VERSION_DESCRIBE_STRING
    "\n",
};

struct fwk_module_config config_log = {
    .data = &log_data,
};

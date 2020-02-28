/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_banner.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <mod_log.h>
#include <mod_pl011.h>
#include <scp_rdn1e1_mmap.h>

/*
 * PL011 module
 */
static const struct fwk_element pl011_element_desc_table[] = {
    [0] = {
        .name = "scp-uart1",
        .data = &((struct mod_pl011_device_config) {
            .reg_base = SCP_UART_BASE,
            .baud_rate_bps = 115200,
            .clock_rate_hz = 24 * FWK_MHZ,
            .clock_id = FWK_ID_NONE_INIT,
            .pd_id = FWK_ID_NONE_INIT,
            }),
    },
    [1] = { 0 },
};

static const struct fwk_element *get_pl011_table(fwk_id_t module_id)
{
    return pl011_element_desc_table;
}

struct fwk_module_config config_pl011 = {
    .get_element_table = get_pl011_table,
};

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

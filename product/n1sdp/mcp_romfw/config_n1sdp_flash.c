/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_module.h>
#include <mod_n1sdp_flash.h>
#include <n1sdp_mcp_system_mmap.h>

const struct fwk_module_config config_n1sdp_flash = {
    .data = &((struct mod_n1sdp_flash_config) {
        .flash_base_address = MCP_QSPI_FLASH_BASE_ADDR,
        .flash_base_address_alt = MCP_QSPI_FLASH_BASE_ADDR_ALT,
        .toc_offset = 0x0,
    })
};

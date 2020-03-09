/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "n1sdp_scp_system_mmap.h"

#include <mod_n1sdp_flash.h>

#include <fwk_module.h>

const struct fwk_module_config config_n1sdp_flash = {
    .data = &((struct mod_n1sdp_flash_config) {
        .flash_base_address = SCP_QSPI_FLASH_BASE_ADDR,
        .flash_base_address_alt = SCP_QSPI_FLASH_BASE_ADDR_ALT,
        .toc_offset = 0x0,
    })
};

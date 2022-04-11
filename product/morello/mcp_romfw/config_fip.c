/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <morello_fip.h>

#include <mod_fip.h>

#include <fwk_module.h>

#define FIP_UUID_MCP_BL2 \
    (struct fip_uuid_desc) \
    { \
        .image_type = \
            (enum mod_fip_toc_entry_type)MOD_MORELLO_FIP_TOC_ENTRY_MCP_BL2, \
        .uuid = { \
            0x54, \
            0x46, \
            0x42, \
            0x22, \
            0xa4, \
            0xcf, \
            0x4b, \
            0xf8, \
            0xb1, \
            0xb6, \
            0xce, \
            0xe7, \
            0xda, \
            0xde, \
            0x53, \
            0x9e \
        } \
    }

struct fip_uuid_desc custom_fip_arr[1] = {
    FIP_UUID_MCP_BL2,
};

static struct mod_fip_module_config fip_data = {
    .custom_fip_uuid_desc_arr = custom_fip_arr,
    .custom_uuid_desc_count = 1,
};

struct fwk_module_config config_fip = { .data = &fip_data };

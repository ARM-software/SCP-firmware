/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <n1sdp_fip.h>

#include <mod_fip.h>

#include <fwk_module.h>

#define FIP_UUID_TFA_BL1 \
    (struct fip_uuid_desc) \
    { \
        .image_type = \
            (enum mod_fip_toc_entry_type)MOD_N1SDP_FIP_TOC_ENTRY_TFA_BL1, \
        .uuid = { \
            0xcf, \
            0xac, \
            0xc2, \
            0xc4, \
            0x15, \
            0xe8, \
            0x46, \
            0x68, \
            0x82, \
            0xbe, \
            0x43, \
            0x0a, \
            0x38, \
            0xfa, \
            0xd7, \
            0x05 \
        } \
    }

struct fip_uuid_desc custom_fip_arr[1] = {
    FIP_UUID_TFA_BL1,
};

static struct mod_fip_module_config fip_data = {
    .custom_fip_uuid_desc_arr = custom_fip_arr,
    .custom_uuid_desc_count = 1,
};

struct fwk_module_config config_fip = { .data = &fip_data };

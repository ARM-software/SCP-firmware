/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Source file for N1SDP flash driver module used to parse the firmware
 *     package residing in on-board QSPI flash memory.
 */

#include <mod_n1sdp_flash.h>

#include "fip.h"

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <string.h>

struct mod_n1sdp_flash_entry_id {
    struct fip_uuid uuid;
};

#define FIP_ENTRY_MCP_BL2 \
    { .u = { 0x54, 0x46, 0x42, 0x22, 0xa4, 0xcf, 0x4b, 0xf8, \
             0xb1, 0xb6, 0xce, 0xe7, 0xda, 0xde, 0x53, 0x9e } }

const struct mod_n1sdp_flash_entry_id mod_n1sdp_flash_entry_mcp_bl2 = {
    .uuid = FIP_ENTRY_MCP_BL2
};

const struct mod_n1sdp_flash_entry_id mod_n1sdp_flash_entry_scp_bl2 = {
    .uuid = FIPL_ENTRY_SCP_BL2
};

const struct mod_n1sdp_flash_entry_id mod_n1sdp_flash_entry_tf_bl31 = {
    .uuid = FIP_ENTRY_TF_BL31
};

/*
 * Module context
 */
static struct  {
    struct fip_toc* toc;
} ctx;


static int uuid_cmp(
    const struct fip_uuid * const a,
    const struct fip_uuid * const b)
{
    return memcmp(a, b, sizeof(*a)) == 0;
}

static int uuid_is_null(const struct fip_uuid * const a)
{
    static const struct fip_uuid uuid_null = FIP_ENTRY_NULL;
    return uuid_cmp(a, &uuid_null);
}


static int get_entry(
    const struct mod_n1sdp_flash_entry_id * const id,
    struct mod_n1sdp_flash_entry * const entry)
{
    if (!ctx.toc)
        return FWK_E_INIT;

    const struct fip_entry* e = ctx.toc->entry;
    while (!uuid_cmp(&e->uuid, &id->uuid)) {
        if (uuid_is_null(&e->uuid))
            return FWK_E_RANGE;
        e++;
    }

    if (e->size > SIZE_MAX)
        return FWK_E_DATA;
    if (e->offset_address > UINTPTR_MAX)
        return false;
    uintptr_t address;
    if (__builtin_add_overflow((uintptr_t)ctx.toc,
                               (uintptr_t)e->offset_address,
                               &address)) {
        return FWK_E_DATA;
    }

    entry->p = (void*)address;
    entry->size = e->size;
    entry->flags = e->flags;
    return FWK_SUCCESS;
}

static bool validate_fip_toc(const struct fip_toc * const toc)
{
    return toc->signature == FIP_TOC_SIGNATURE;
}

static const struct mod_n1sdp_flash_api n1sdp_flash_api = {
    .get_entry = get_entry,
};

static int n1sdp_flash_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    const struct mod_n1sdp_flash_config *config = data;
    if ((config == NULL) || (element_count > 0))
        return FWK_E_PANIC;

    struct fip_toc* toc = (void*)config->flash_base_address;
    if (!validate_fip_toc(toc)) {
        toc = (void*)config->flash_base_address_alt;
        if (!validate_fip_toc(toc)) {
            /*
             * If we return a failure here the initializing of ROM images might
             * stall without any log messages.
             * Failure here are not critical as API function will return
             * FWK_E_INIT if this failure is triggered.
             */
            return FWK_SUCCESS;
        }
    }

    ctx.toc = toc;
    return FWK_SUCCESS;
}

static int n1sdp_flash_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &n1sdp_flash_api;
    return FWK_SUCCESS;
}

const struct fwk_module module_n1sdp_flash = {
    .name = "N1SDP Flash",
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = 1,
    .init = n1sdp_flash_init,
    .process_bind_request = n1sdp_flash_process_bind_request,
};

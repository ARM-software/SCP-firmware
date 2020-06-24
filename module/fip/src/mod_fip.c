/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Firmware Image Package (FIP) parser support.
 */

#include <internal/fip.h>

#include <mod_fip.h>

#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <string.h>

/*
 * Module context
 */
struct fip_ctx {
    /* Memory-mapped base address of the FIP */
    struct fip_toc *toc;
    /* FIP module configuration data */
    const struct mod_fip_config *config;
};

static struct fip_ctx module_ctx;

/*
 * Static helpers
 */
static int fip_entry_type_to_uuid(
    enum mod_fip_toc_entry_type type,
    struct fip_uuid *uuid)
{
    switch (type) {
    case MOD_FIP_TOC_ENTRY_SCP_BL2:
        *uuid = FIP_UUID_SCP_BL2;
        break;
    case MOD_FIP_TOC_ENTRY_MCP_BL2:
        *uuid = FIP_UUID_MCP_BL2;
        break;
    case MOD_FIP_TOC_ENTRY_TFA_BL31:
        *uuid = FIP_UUID_TFA_BL31;
        break;
    default:
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int uuid_cmp(
    const struct fip_uuid *const a,
    const struct fip_uuid *const b)
{
    return memcmp(a, b, sizeof(*a)) == 0;
}

static int uuid_is_null(const struct fip_uuid *const a)
{
    static const struct fip_uuid uuid_null = FIP_UUID_NULL;
    return uuid_cmp(a, &uuid_null);
}

static bool validate_fip_toc(const struct fip_toc *const toc)
{
    return toc->header.name == FIP_TOC_HEADER_NAME;
}

/*
 * Module API functions
 */
static int fip_get_entry(
    enum mod_fip_toc_entry_type type,
    struct mod_fip_entry_data *const entry_data)
{
    uintptr_t address;
    struct fip_uuid target_uuid;
    struct fip_toc_entry *toc_entry;
    int status;

    if (!module_ctx.toc)
        return FWK_E_INIT;

    toc_entry = module_ctx.toc->entry;

    status = fip_entry_type_to_uuid(type, &target_uuid);
    if (status != FWK_SUCCESS)
        return status;

    /*
     * Traverse all FIP ToC entries until the desired entry is found or ToC
     * End Marker is reached
     */
    while (!uuid_cmp(&toc_entry->uuid, &target_uuid)) {
        if (uuid_is_null(&toc_entry->uuid))
            return FWK_E_RANGE;
        toc_entry++;
    }

    /* Sanity checks of the retrieved entry data */
    if (__builtin_add_overflow(
            (uintptr_t)module_ctx.toc,
            (uintptr_t)toc_entry->offset_address,
            &address)) {
        return FWK_E_DATA;
    }

    if ((uintptr_t)toc_entry->offset_address + toc_entry->size >
        module_ctx.config->fip_nvm_size) {
        return FWK_E_SIZE;
    }

    entry_data->base = (void *)address;
    entry_data->size = toc_entry->size;
    entry_data->flags = toc_entry->flags;
    return FWK_SUCCESS;
}

static const struct mod_fip_api fip_api = {
    .get_entry = fip_get_entry,
};

/*
 * Framework handler functions
 */
static int fip_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    const struct mod_fip_config *config = data;
    if ((config == NULL) || (element_count > 0))
        return FWK_E_PANIC;

    struct fip_toc *toc = (void *)config->fip_base_address;
    if (!validate_fip_toc(toc)) {
        /*
         * The error log message here requires the platform to enable an
         * always-on logging mechanism in order to detect this failure in
         * early stages, such as in ROM code.
         */
        FWK_LOG_ERR(
            "[FIP] Invalid FIP ToC header name: [0x%08" PRIX32 "]",
            toc->header.name);
        return FWK_E_INIT;
    }

    module_ctx.toc = toc;
    module_ctx.config = config;

    return FWK_SUCCESS;
}

static int fip_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &fip_api;
    return FWK_SUCCESS;
}

const struct fwk_module module_fip = {
    .name = "FIP Parser",
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = 1,
    .init = fip_init,
    .process_bind_request = fip_process_bind_request,
};

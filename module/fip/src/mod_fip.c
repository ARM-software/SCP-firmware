/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Firmware Image Package (FIP) parser support.
 */

#include <mod_fip.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_status.h>
#include <fwk_string.h>

#include <inttypes.h>
#include <string.h>

static const struct fip_uuid_desc fip_uuid_desc_arr[3] = {
    FIP_UUID_NULL,
    FIP_UUID_SCP_BL2,
    FIP_UUID_TFA_BL31,
};

/*
 * Static helpers
 */
static int fip_entry_type_to_uuid(
    enum mod_fip_toc_entry_type type,
    uint8_t *uuid)
{
    const struct mod_fip_module_config *module_config;
    size_t i;

    module_config = fwk_module_get_data(fwk_module_id_fip);
    size_t desc_arr_size =
        sizeof(fip_uuid_desc_arr) / sizeof(fip_uuid_desc_arr[0]);

    if (type < MOD_FIP_TOC_ENTRY_COUNT) {
        for (i = 0; i < desc_arr_size; i++) {
            if (fip_uuid_desc_arr[i].image_type == type) {
                fwk_str_memcpy(
                    uuid, fip_uuid_desc_arr[i].uuid, FIP_UUID_ENTRY_SIZE);
                return FWK_SUCCESS;
            }
        }
    }

    if (type >= MOD_FIP_TOC_ENTRY_COUNT &&
        module_config->custom_fip_uuid_desc_arr != NULL) {
        for (i = 0; i < module_config->custom_uuid_desc_count; i++) {
            if (module_config->custom_fip_uuid_desc_arr[i].image_type == type) {
                fwk_str_memcpy(
                    uuid,
                    module_config->custom_fip_uuid_desc_arr[i].uuid,
                    FIP_UUID_ENTRY_SIZE);
                return FWK_SUCCESS;
            }
        }
    }

    return FWK_E_PARAM;
}

static inline bool uuid_cmp(const uint8_t *a, const uint8_t *b)
{
    int match = 0;

    for (unsigned int i = 0; i < FIP_UUID_ENTRY_SIZE; i++) {
        if (a[i] == b[i]) {
            match++;
        } else {
            match--;
        }
    }

    return (match == FIP_UUID_ENTRY_SIZE);
}

static bool uuid_is_null(const uint8_t *uuid)
{
    static const struct fip_uuid_desc uuid_null = FIP_UUID_NULL;
    return uuid_cmp(uuid, uuid_null.uuid);
}

static bool validate_fip_toc(const struct fip_toc *const toc)
{
    return toc->header.name == FIP_TOC_HEADER_NAME;
}

/*
 * Module API functions
 */
static int fip_get_entry(
    enum mod_fip_toc_entry_type image_type,
    struct mod_fip_entry_data *const entry_data,
    uintptr_t base,
    size_t limit)
{
    uintptr_t address;
    uint8_t target_uuid[FIP_UUID_ENTRY_SIZE];
    struct fip_toc_entry *toc_entry;
    int status;
    struct fip_toc *toc = (void *)base;

    if (!validate_fip_toc(toc)) {
        /*
         * The error log message here requires the platform to enable an
         * always-on logging mechanism in order to detect this failure in
         * early stages, such as in ROM code.
         */
        FWK_LOG_ERR(
            "[FIP] Invalid FIP ToC header name: [0x%08" PRIX32 "]",
            toc->header.name);
        return FWK_E_PARAM;
    }

    toc_entry = toc->entry;

    /* Updates target_uuid field with UUID of corresponding image_type passed */
    status = fip_entry_type_to_uuid(image_type, target_uuid);
    if (status != FWK_SUCCESS)
        return status;

    /*
     * Traverse all FIP ToC entries until the desired entry is found or ToC
     * End Marker is reached
     */
    while (!uuid_cmp(toc_entry->uuid, target_uuid)) {
        if (uuid_is_null(toc_entry->uuid))
            return FWK_E_RANGE;
        toc_entry++;
    }

    /* Sanity checks of the retrieved entry data */
    if (__builtin_add_overflow(
            (uintptr_t)toc, (uintptr_t)toc_entry->offset_address, &address)) {
        return FWK_E_DATA;
    }

    if ((uintptr_t)toc_entry->offset_address + toc_entry->size > limit)
        return FWK_E_SIZE;

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
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = 1,
    .init = fip_init,
    .process_bind_request = fip_process_bind_request,
};

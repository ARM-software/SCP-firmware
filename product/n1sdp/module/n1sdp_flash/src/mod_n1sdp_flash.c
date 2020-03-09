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

#include <internal/n1sdp_flash_layout.h>
#include <internal/uuid.h>

#include <mod_n1sdp_flash.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <string.h>

/*
 * Module context
 */
struct mod_n1sdp_flash_ctx {
    /* Flash descriptor count */
    unsigned int flash_desc_count;

    /* NFIP descriptor count */
    unsigned int n1sdp_fip_desc_count;

    /* Flash descriptor table */
    struct mod_n1sdp_flash_descriptor *flash_desc_table;

    /* FIP descriptor table */
    struct mod_n1sdp_fip_descriptor *n1sdp_fip_desc_table;
};

static struct mod_n1sdp_flash_ctx n1sdp_flash_ctx;

/*
 * Module API Implementation
 */
static int get_flash_descriptor_count(fwk_id_t id, unsigned int *count)
{
    if (count == NULL)
        return FWK_E_PARAM;

    *count = n1sdp_flash_ctx.flash_desc_count;

    return FWK_SUCCESS;
}

static int get_flash_descriptor_table(
    fwk_id_t id,
    struct mod_n1sdp_flash_descriptor **table)
{
    if (table == NULL)
        return FWK_E_PARAM;

    *table = n1sdp_flash_ctx.flash_desc_table;

    return FWK_SUCCESS;
}

static int get_n1sdp_fip_descriptor_count(fwk_id_t id, unsigned int *count)
{
    if (count == NULL)
        return FWK_E_PARAM;

    *count = n1sdp_flash_ctx.n1sdp_fip_desc_count;

    return FWK_SUCCESS;
}

static int get_n1sdp_fip_descriptor_table(
    fwk_id_t id,
    struct mod_n1sdp_fip_descriptor **table)
{
    if (table == NULL)
        return FWK_E_PARAM;

    *table = n1sdp_flash_ctx.n1sdp_fip_desc_table;

    return FWK_SUCCESS;
}

static struct mod_n1sdp_flash_api module_api = {
    .get_flash_descriptor_count = get_flash_descriptor_count,
    .get_flash_descriptor_table = get_flash_descriptor_table,
    .get_n1sdp_fip_descriptor_count = get_n1sdp_fip_descriptor_count,
    .get_n1sdp_fip_descriptor_table = get_n1sdp_fip_descriptor_table,
};

/*
 * Utility functions
 */
static uint16_t crc16(const void *data, uint32_t size)
{
    uint16_t crc = 0;
    uint16_t poly = CRC16_POLYNOMIAL;
    const uint8_t *byte = data;
    uint32_t i = 0, j = 0;

    assert(data != NULL);
    assert(size != 0);

    for (i = 0; i < size; ++i) {
        crc = crc ^ byte[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ poly;
            else
                crc = crc >> 1;
        }
    }

    return crc;
}

/*
 * Flash & FIP parsing functions
 */
static int n1sdp_fip_parse(uintptr_t n1sdp_fip_base)
{
    const struct n1sdp_fip_memory_toc *n1sdp_fip_toc = NULL;
    const struct n1sdp_fip_toc_entry *toc_entry = NULL;
    struct mod_n1sdp_fip_descriptor *n1sdp_fip_desc = NULL;
    struct uuid_t scp_bl2_uuid = UUID_SCP_FIRMWARE_SCP_BL2;
    struct uuid_t mcp_bl2_uuid = UUID_MCP_FIRMWARE_MCP_BL2;
    struct uuid_t bl31_uuid = UUID_EL3_RUNTIME_FIRMWARE_BL31;
    struct uuid_t bl32_uuid = UUID_SECURE_PAYLOAD_BL32;
    struct uuid_t bl33_uuid = UUID_NON_TRUSTED_FIRMWARE_BL33;

    n1sdp_fip_toc = (const struct n1sdp_fip_memory_toc *)n1sdp_fip_base;
    if (n1sdp_fip_toc == NULL)
        return FWK_E_PANIC;

    if (crc16(n1sdp_fip_toc, sizeof(struct n1sdp_fip_memory_toc)) != 0)
        return FWK_E_DATA;

    if (n1sdp_fip_toc->signature != NFIP_TOC_SIGNATURE)
        return FWK_E_DATA;

    if (n1sdp_fip_toc->entry_count == 0)
        return FWK_E_DATA;

    n1sdp_flash_ctx.n1sdp_fip_desc_count = n1sdp_fip_toc->entry_count;

    n1sdp_flash_ctx.n1sdp_fip_desc_table = fwk_mm_calloc(
        n1sdp_flash_ctx.n1sdp_fip_desc_count,
        sizeof(struct mod_n1sdp_fip_descriptor));

    for (unsigned int i = 0; i < n1sdp_flash_ctx.n1sdp_fip_desc_count; i++) {
        toc_entry = &n1sdp_fip_toc->entry[i];
        /* Check if CRC has to be calculated */
        if (toc_entry->flags & MOD_N1SDP_FLASH_DATA_FLAG_CRC16_ENABLED) {
            if (crc16(toc_entry, sizeof(*toc_entry)) != 0)
                return FWK_E_DATA;
        }

        n1sdp_fip_desc = &n1sdp_flash_ctx.n1sdp_fip_desc_table[i];
        n1sdp_fip_desc->address = (n1sdp_fip_base + toc_entry->offset);
        n1sdp_fip_desc->size = toc_entry->size;
        n1sdp_fip_desc->flags = toc_entry->flags;

        if (!memcmp(&toc_entry->uuid,
                    &scp_bl2_uuid,
                    sizeof(struct uuid_t))) {
            n1sdp_fip_desc->type = MOD_N1SDP_FIP_TYPE_SCP_BL2;
        } else if (!memcmp(&toc_entry->uuid,
                    &mcp_bl2_uuid, sizeof(struct uuid_t))) {
            n1sdp_fip_desc->type = MOD_N1SDP_FIP_TYPE_MCP_BL2;
        } else if (!memcmp(&toc_entry->uuid,
                    &bl31_uuid, sizeof(struct uuid_t))) {
            n1sdp_fip_desc->type = MOD_N1SDP_FIP_TYPE_TF_BL31;
        } else if (!memcmp(&toc_entry->uuid,
                    &bl32_uuid, sizeof(struct uuid_t))) {
            n1sdp_fip_desc->type = MOD_N1SDP_FIP_TYPE_TF_BL32;
        } else if (!memcmp(&toc_entry->uuid,
                    &bl33_uuid, sizeof(struct uuid_t))) {
            n1sdp_fip_desc->type = MOD_N1SDP_FIP_TYPE_NS_BL33;
        } else {
            return FWK_E_DATA;
        }
    }

    return FWK_SUCCESS;
}

static int n1sdp_flash_parse(uintptr_t address, uintptr_t offset)
{
    const struct n1sdp_flash_memory_toc *toc = NULL;
    const struct n1sdp_flash_toc_entry *toc_entry = NULL;
    struct mod_n1sdp_flash_descriptor *flash_desc = NULL;
    int status;

    toc = (void *)(address + offset);

    if (crc16(toc, sizeof(struct n1sdp_flash_memory_toc)) != 0)
        return FWK_E_DATA;

    if (toc->signature != FLASH_TOC_SIGNATURE)
        return FWK_E_DATA;

    if (toc->entry_count == 0)
        return FWK_E_DATA;

    n1sdp_flash_ctx.flash_desc_count = toc->entry_count;

    n1sdp_flash_ctx.flash_desc_table = fwk_mm_calloc(
        n1sdp_flash_ctx.flash_desc_count,
        sizeof(struct mod_n1sdp_flash_descriptor));

    for (unsigned int i = 0; i < n1sdp_flash_ctx.flash_desc_count; ++i) {
        toc_entry = &toc->entry[i];
        /* Check if CRC has to be calculated */
        if (toc_entry->flags & MOD_N1SDP_FLASH_DATA_FLAG_CRC16_ENABLED) {
            if (crc16(toc_entry, sizeof(*toc_entry)) != 0)
                return FWK_E_DATA;
        }

        flash_desc = &n1sdp_flash_ctx.flash_desc_table[i];
        flash_desc->address = (address + toc_entry->offset);
        flash_desc->size = toc_entry->size;
        flash_desc->flags = toc_entry->flags;
        flash_desc->type = toc_entry->type;

        /* If flash entry type is N1SDP FIP (NFIP) then parse it */
        if (toc->entry[i].type == MOD_N1SDP_FLASH_DATA_TYPE_NFIP) {
            status = n1sdp_fip_parse(flash_desc->address);
            if (status != FWK_SUCCESS)
                return FWK_E_DATA;
        }
    }
    return FWK_SUCCESS;
}

/*
 * Framework handlers
 */

static int n1sdp_flash_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    int status;
    const struct mod_n1sdp_flash_config *config = NULL;

    config = (const struct mod_n1sdp_flash_config *)data;
    if ((config == NULL) || (element_count > 0))
        return FWK_E_PANIC;

    /* Parse & validate the flash base address */
    status = n1sdp_flash_parse(config->flash_base_address, config->toc_offset);

    /* No valid records found. Parse & validate alternate base address */
    if (status == FWK_E_DATA) {
        return n1sdp_flash_parse(config->flash_base_address_alt,
                                  config->toc_offset);
    }

    return status;
}

static int n1sdp_flash_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &module_api;
    return FWK_SUCCESS;
}

const struct fwk_module module_n1sdp_flash = {
    .name = "N1SDP Flash",
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = 1,
    .init = n1sdp_flash_init,
    .process_bind_request = n1sdp_flash_process_bind_request,
};

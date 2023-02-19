/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Direct ATU management - ATU driver directly manages the ATU.
 */

#include <internal/atu.h>
#include <internal/atu_common.h>
#include <internal/atu_manage_utils.h>

#include <mod_atu.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <inttypes.h>

/*! ATU BC Register NTR bit field offset */
#define ATU_BC_NTR_OFFSET (0u)
/*! ATU BC Register NTR bit field mask */
#define ATU_BC_NTR_MASK (0x7u << ATU_BC_NTR_OFFSET)

/*! ATU BC Register PS bit field offset */
#define ATU_BC_PS_OFFSET (4u)
/*! ATU BC Register PS bit field mask */
#define ATU_BC_PS_MASK (0xFu << ATU_BC_PS_OFFSET)

/*! ATU BC Register PAW bit field offset */
#define ATU_BC_PAW_OFFSET (8u)
/*! ATU BC Register PAW bit field mask */
#define ATU_BC_PAW_MASK (0xFu << ATU_BC_PAW_OFFSET)

/*
 * Mask for reading the output bus attributes to be configured for the ATU
 * region.
 */
#define ATU_REGION_ROBA_MASK (0xFFFFu)

/* Shared ATU module context */
static struct mod_atu_ctx *shared_atu_ctx;

/*
 * ATU API implementation
 */
static int atu_add_region(
    const struct atu_region_map *region,
    fwk_id_t atu_device_id,
    uint8_t *region_idx)
{
    struct atu_device_ctx *device_ctx;
    int status;

    /* Return error if region or region index parameters are null pointers */
    if ((region == NULL) || (region_idx == NULL)) {
        return FWK_E_PARAM;
    }

    /* Validate the output bus attributes for the ATU region */
    status = atu_validate_region_attributes(region->attributes);
    if (status != FWK_SUCCESS) {
        return status;
    }

    device_ctx = &shared_atu_ctx
                      ->device_ctx_table[fwk_id_get_element_idx(atu_device_id)];

    /*
     * Check if the translation region is algined with the ATU page size or if
     * the region overlaps(logical address) any existing translation regions
     * mapped in the ATU.
     */
    status = atu_validate_region(region, device_ctx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = atu_get_available_region_idx((void *)device_ctx, region_idx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* Configure the output bus attributes for the ATU region */
    device_ctx->atu->ATUROBA[*region_idx] |=
        (region->attributes & ATU_REGION_ROBA_MASK);

    return atu_map_region(region, *region_idx, device_ctx);
}

static int atu_remove_region(
    uint8_t region_idx,
    fwk_id_t atu_device_id,
    fwk_id_t requester_id)
{
    struct atu_device_ctx *device_ctx;

    device_ctx = &shared_atu_ctx
                      ->device_ctx_table[fwk_id_get_element_idx(atu_device_id)];

    if ((region_idx >= device_ctx->max_atu_region_count) ||
        !fwk_module_is_valid_entity_id(requester_id)) {
        return FWK_E_PARAM;
    }

    /* Check if the requester has permission to disable the region */
    if (!fwk_id_is_equal(device_ctx->owner_id_list[region_idx], requester_id)) {
        FWK_LOG_ERR("[ATU] Error! Requester does not manage the region");
        return FWK_E_ACCESS;
    }

    return atu_unmap_region(region_idx, device_ctx);
}

const struct mod_atu_api atu_manage_api = {
    .add_region = atu_add_region,
    .remove_region = atu_remove_region,
};

/*
 * framework helper functions
 */
static int atu_device_init(fwk_id_t atu_device_id)
{
    struct atu_device_ctx *device_ctx;

    if (shared_atu_ctx == NULL) {
        return FWK_E_DATA;
    }

    device_ctx = &shared_atu_ctx
                      ->device_ctx_table[fwk_id_get_element_idx(atu_device_id)];

    if (device_ctx->config->atu_base == 0) {
        return FWK_E_PARAM;
    }

    /* Store the ATU base address */
    device_ctx->atu = (struct atu_reg *)device_ctx->config->atu_base;

    /*
     * Read the following configuration data from the ATUBC register:
     *   - Number of translation regions supported.
     *   - Page size.
     *   - SOC's physical address width.
     */
    device_ctx->max_atu_region_count =
        (1u << (device_ctx->atu->ATUBC & ATU_BC_NTR_MASK));
    device_ctx->page_size =
        ((device_ctx->atu->ATUBC & ATU_BC_PS_MASK) >> ATU_BC_PS_OFFSET);
    device_ctx->phy_addr_width =
        ((device_ctx->atu->ATUBC & ATU_BC_PAW_MASK) >> ATU_BC_PAW_OFFSET);

    /* Allocate memory for the region context table */
    device_ctx->active_regions = fwk_mm_calloc(
        device_ctx->max_atu_region_count, sizeof(struct atu_region_map));

    device_ctx->owner_id_list =
        fwk_mm_calloc(device_ctx->max_atu_region_count, sizeof(fwk_id_t));

    /* Initialize the owner id list array to none */
    for (uint8_t i = 0; i < device_ctx->max_atu_region_count; i++) {
        device_ctx->owner_id_list[i] = FWK_ID_NONE;
    }

    /* Enable ATU interrupt */
    device_ctx->atu->ATUIE |= 0x1;

    device_ctx->atu_api = &atu_manage_api;

    /*
     * Iterate through the table of ATU region config data and configure the
     * translation regions in the ATU.
     */
    for (uint8_t idx = 0; idx < device_ctx->config->atu_region_count; idx++) {
        uint8_t region_idx;
        int status;
        const struct atu_region_map *atu_region_config;

        atu_region_config = &device_ctx->config->atu_region_config_table[idx];

        FWK_LOG_ERR("[ATU] Initializing region %u", idx);

        /* Configure the translation region */
        status = atu_add_region(atu_region_config, atu_device_id, &region_idx);

        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR("[ATU] Failed to initialize region %u", idx);
            return status;
        }
    }
    return FWK_SUCCESS;
}

static int atu_manage_set_shared_ctx(struct mod_atu_ctx *atu_ctx_param)
{
    if (atu_ctx_param == NULL) {
        return FWK_E_PARAM;
    }

    shared_atu_ctx = atu_ctx_param;

    return FWK_SUCCESS;
}

/* Pointers to ATU driver module framework helper functions */
static struct mod_atu_ops mod_atu_manage_ops = {
    .atu_init_shared_ctx = atu_manage_set_shared_ctx,
    .atu_device_init = atu_device_init,
};

void atu_get_manage_ops(const struct mod_atu_ops **atu_ops)
{
    *atu_ops = &mod_atu_manage_ops;
}

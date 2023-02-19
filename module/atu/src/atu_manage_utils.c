/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for the ATU driver module
 */

#include <internal/atu.h>
#include <internal/atu_manage_utils.h>

#include <mod_atu.h>

#include <fwk_log.h>
#include <fwk_status.h>
#include <fwk_string.h>

#include <inttypes.h>

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
static int atu_print_region(uint8_t region_idx, void *device_ctx_ptr)
{
    struct atu_device_ctx *device_ctx;
    struct atu_region_map *region;
    uint32_t log_region_end;

    device_ctx = (struct atu_device_ctx *)device_ctx_ptr;

    region = &device_ctx->active_regions[region_idx];

    log_region_end = ((region->log_addr_base + region->region_size) - 1);

    /*
     * The ATU region has been enabled in the ATU but the module is not aware
     * of the region configuration. This implies that the the ATU region has
     * been initialized outside the MSCP ATU driver module.
     */
    if (region->region_size == 0) {
        FWK_LOG_INFO(
            "[ATU] Region %u configured outside the firmware", region_idx);
        return FWK_E_DATA;
    }

    FWK_LOG_INFO(
        "[ATU] Region %u managed by %s",
        region_idx,
        FWK_ID_STR(device_ctx->owner_id_list[region_idx]));

    FWK_LOG_INFO(
        "      [0x%" PRIX32 " - 0x%" PRIX32 "]-->[0x%" PRIX64 " - 0x%" PRIX64
        "]",
        region->log_addr_base,
        log_region_end,
        region->phy_addr_base,
        (region->phy_addr_base + region->region_size - 1));

    return FWK_SUCCESS;
}

static void atu_print_active_regions(void *device_ctx_ptr)
{
    struct atu_device_ctx *device_ctx;
    uint32_t atuc_reg_value;
    uint8_t i, j;
    int status;

    device_ctx = (struct atu_device_ctx *)device_ctx_ptr;

    /* Read the ATU configuration register value */
    atuc_reg_value = device_ctx->atu->ATUC;

    /* Print all the active ATU regions */
    FWK_LOG_INFO("-------------------[ATU]------------------");
    i = 0;
    j = 0;
    while ((i < device_ctx->active_regions_count) ||
           (j < device_ctx->max_atu_region_count)) {
        if ((atuc_reg_value >> j) & 0x1) {
            status = atu_print_region(j, device_ctx);
            /* Skip ATU regions which were configured outside ATU driver */
            if (status == FWK_E_DATA) {
                j++;
                continue;
            }
            i++;
        }
        j++;
    }
    FWK_LOG_INFO("-------------------[/ATU]-----------------");
}
#endif

static int atu_enable_region(uint8_t region_idx, void *device_ctx_ptr)
{
    struct atu_device_ctx *device_ctx;

    device_ctx = (struct atu_device_ctx *)device_ctx_ptr;

    /* Check if the translation region is supported */
    if (region_idx >= device_ctx->max_atu_region_count) {
        return FWK_E_SUPPORT;
    }

    /* Return error if the region is already in use */
    if (device_ctx->atu->ATUC & (0x1 << region_idx)) {
        FWK_LOG_ERR("[ATU] Error! Attempt to configure an active ATU region");
        return FWK_E_STATE;
    }

    /* Enable the translation region */
    device_ctx->atu->ATUC |= (1u << region_idx);

    /* Update the total number of active ATU regions in the context */
    device_ctx->active_regions_count++;

    FWK_LOG_INFO("[ATU] Region %u enabled", region_idx);

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
    /* Print the enabled ATU region */
    atu_print_region(region_idx, device_ctx);
#endif
    return FWK_SUCCESS;
}

static int atu_disable_region(uint8_t region_idx, void *device_ctx_ptr)
{
    struct atu_device_ctx *device_ctx;

    device_ctx = (struct atu_device_ctx *)device_ctx_ptr;

    /* Check if the translation region is supported */
    if (region_idx >= device_ctx->max_atu_region_count) {
        return FWK_E_SUPPORT;
    }

    /* Return if the region is not enabled */
    if (!(device_ctx->atu->ATUC & (0x1 << region_idx))) {
        FWK_LOG_WARN(
            "[ATU] Warning! Attempt to disable an inactive ATU region");
        return FWK_SUCCESS;
    }

    /* Disable the translation region */
    device_ctx->atu->ATUC &= ~(1u << region_idx);

    /* Update the total number of active ATU regions in the context */
    device_ctx->active_regions_count--;

    FWK_LOG_INFO("[ATU] Region %u disabled", region_idx);

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
    /* Print all the active translation regions */
    atu_print_active_regions(device_ctx);
#endif
    return FWK_SUCCESS;
}

/* Compares address ranges to check if there's a overlap */
static int atu_check_addr_overlap(
    uint64_t region_start,
    uint64_t region_end,
    uint64_t active_region_start,
    uint64_t active_region_end)
{
    /* Check if start address falls within the active region address range */
    if ((region_start >= active_region_start) &&
        (region_start <= active_region_end)) {
        return FWK_E_SUPPORT;
    }
    /*
     * The requested tranlation region does not overlap the active
     * translation region.
     */
    return FWK_SUCCESS;
}

/* Checks if the logical address range overlaps between two regions */
static int atu_check_log_addr_overlap(
    const struct atu_region_map *region,
    const struct atu_region_map *active_region)
{
    return atu_check_addr_overlap(
        region->log_addr_base,
        (region->log_addr_base + region->region_size - 1),
        active_region->log_addr_base,
        (active_region->log_addr_base + active_region->region_size - 1));
}

/* Get the index of a disabled ATU region that's available of mapping */
int atu_get_available_region_idx(void *device_ctx_ptr, uint8_t *region_idx)
{
    uint32_t atuc_reg_value;
    struct atu_device_ctx *device_ctx;

    if (region_idx == NULL) {
        return FWK_E_PARAM;
    }

    device_ctx = (struct atu_device_ctx *)device_ctx_ptr;

    /* Read the ATU configuration register value */
    atuc_reg_value = device_ctx->atu->ATUC;

    /*
     * Iterate through the bits in the ATU configuration register value to get
     * the region index that's available. A region index is available when the
     * corresponding bit is not set i.e the region is not enabled in the ATU
     * configuration register.
     */
    for (uint8_t pos = 0; pos < device_ctx->max_atu_region_count; pos++) {
        if (((atuc_reg_value >> pos) & 0x1) == 0) {
            *region_idx = pos;
            return FWK_SUCCESS;
        }
    }

    return FWK_E_SUPPORT;
}

/* Validates if a translation region can be mapped in the ATU */
int atu_validate_region(
    const struct atu_region_map *region,
    void *device_ctx_ptr)
{
    const struct atu_region_map *active_region;
    struct atu_device_ctx *device_ctx;
    uint8_t i, j;
    uint8_t ps;
    int status = 0;

    device_ctx = (struct atu_device_ctx *)device_ctx_ptr;

    ps = device_ctx->page_size;

    /* Check if the requested region is aligned with the ATU page size */
    if ((region->phy_addr_base & ((1 << ps) - 1)) != 0 ||
        (region->log_addr_base & ((1 << ps) - 1)) != 0 ||
        (region->region_size & ((1 << ps) - 1)) != 0) {
        return FWK_E_PARAM;
    }

    /*
     * Iterate through active translation regions to check if there's any
     * overlap.
     */
    i = 0;
    j = 0;
    while (i < device_ctx->active_regions_count &&
           j < device_ctx->max_atu_region_count) {
        if ((device_ctx->atu->ATUC >> j) & 0x1) {
            active_region = &device_ctx->active_regions[j];
            /* Skip the regions configured outside the ATU driver */
            if (active_region->region_size == 0) {
                j++;
                continue;
            }
            /* Check if the logical address range overlap */
            status = atu_check_log_addr_overlap(region, active_region);

            /*
             * Return error if the logical address overlaps an existing
             * translation region,
             */
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(
                    "[ATU] Error! Logical address overlaps with an "
                    "existing ATU region");
                return status;
            }
            i++;
        }
        j++;
    }

    return status;
}

int atu_map_region(
    const struct atu_region_map *region,
    uint8_t region_idx,
    void *device_ctx_ptr)
{
    struct atu_device_ctx *device_ctx;
    uint64_t add_value;
    uint64_t rsla;
    uint64_t rspa;

    device_ctx = (struct atu_device_ctx *)device_ctx_ptr;

    /* Check if the translation region is supported */
    if (region_idx >= device_ctx->max_atu_region_count) {
        return FWK_E_SUPPORT;
    }

    /* Right shift the logical address by the page size to compute RSLA */
    rsla = region->log_addr_base >> device_ctx->page_size;
    /* Configure the right shifted start logical address(RSSLA) of the region */
    device_ctx->atu->ATURSSLA[region_idx] = rsla;

    /* Configure the right shifted end logical address(RSELA) of the region */
    device_ctx->atu->ATURSELA[region_idx] =
        ((region->log_addr_base + region->region_size - 1) >>
         device_ctx->page_size);

    /* Right shift the physical address by the page size to compute RSPA */
    rspa = region->phy_addr_base >> device_ctx->page_size;
    add_value = rspa - rsla;

    /* Configure the add value register of the region */
    device_ctx->atu->ATURAV_L[region_idx] = add_value & 0xFFFFFFFF;
    device_ctx->atu->ATURAV_H[region_idx] =
        (uint32_t)((add_value >> 32) & 0xFFFFFFFF);

    /* Save the ATU region configuration data in the module context */
    fwk_str_memcpy(
        (void *)&device_ctx->active_regions[region_idx],
        (const void *)region,
        sizeof(struct atu_region_map));

    /* Save the identifier that has the ownership of this ATU region */
    device_ctx->owner_id_list[region_idx] = region->region_owner_id;

    /* Enable the ATU region */
    return atu_enable_region(region_idx, device_ctx);
}

int atu_unmap_region(uint8_t region_idx, void *device_ctx_ptr)
{
    struct atu_device_ctx *device_ctx;

    device_ctx = (struct atu_device_ctx *)device_ctx_ptr;

    /* Remove the ownership identifier for the disabled region */
    device_ctx->owner_id_list[region_idx] = FWK_ID_NONE;

    /* Clear the region info from the module context */
    fwk_str_memset(
        (void *)&device_ctx->active_regions[region_idx],
        0,
        sizeof(struct atu_region_map));

    /* Disable the translation region */
    return atu_disable_region(region_idx, device_ctx);
}

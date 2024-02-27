/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <noc_s3.h>
#include <noc_s3_discovery.h>
#include <noc_s3_psam.h>
#include <noc_s3_reg.h>

#include <mod_noc_s3.h>

#include <fwk_log.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

/* Global PSAM control. */
#define NOC_S3_SAM_STATUS_SETUP_COMPLETE 0x1

/* Minimum page size if 4K. Address and size should be 4K aligned. */
#define NOC_S3_ADDRESS_PAGE_SIZE (1ULL << 12)
#define NOC_S3_ADDRESS(addr)     ((addr) & ~(NOC_S3_ADDRESS_PAGE_SIZE - 1))

/* Target xMNI ID configuration. */
#define NOC_S3_TARGET_ID_SIZE       (1ULL << 8)
#define NOC_S3_TARGET_ID(target_id) (target_id & (NOC_S3_TARGET_ID_SIZE - 1))

/* Check if the given address is 4K aligned. */
#define NOC_S3_ADDRESS_VALID(addr) \
    (((addr) & (NOC_S3_ADDRESS_PAGE_SIZE - 1)) == 0)

/* Helpers for enabling and disabling a region in PSAM. */
#define NOC_S3_NH_REGION_VALID     1UL
#define NOC_S3_DISABLE_REGION(cfg) ((cfg) &= ~(NOC_S3_NH_REGION_VALID))
#define NOC_S3_ENABLE_REGION(cfg)  ((cfg) |= (NOC_S3_NH_REGION_VALID))

/* Non hash region count mask. */
#define NUM_NH_REGION_MASK       0xFF
#define GET_NH_REGION_COUNT(reg) (reg & NUM_NH_REGION_MASK)

/* Only xSNI support PSAM programming. */
bool is_psam_supported(enum mod_noc_s3_node_type type)
{
    bool psam_supported;

    switch (type) {
    case MOD_NOC_S3_NODE_TYPE_ASNI:
    case MOD_NOC_S3_NODE_TYPE_HSNI:
        psam_supported = true;
        break;
    default:
        psam_supported = false;
        break;
    }

    return psam_supported;
}

/* Check if the region is enabled by checking the 0 bit. */
static bool region_enabled(uint64_t cfg1_cfg0)
{
    return (cfg1_cfg0 & NOC_S3_NH_REGION_VALID) == NOC_S3_NH_REGION_VALID;
}

/*
 * PSAM region de-initialization by setting the nh_region register to 0.
 */
static int psam_nhregion_deinit(struct noc_s3_psam_reg *reg, uint64_t region)
{
    uint8_t region_count;

    region_count = GET_NH_REGION_COUNT(reg->sam_unit_info);
    if (region >= region_count) {
        return FWK_E_RANGE;
    }

    FWK_TRACE(MOD_NAME "Removing Region: %lld region at: %p", region, reg);

    /* Clear base address */
    reg->nh_region[region].cfg1_cfg0 = 0;
    /* Clear end address */
    reg->nh_region[region].cfg3_cfg2 = 0;
    __DMB();

    return FWK_SUCCESS;
}

/*
 * Initialize a non hashed region in the PSAM.
 */
static int psam_nhregion_init(
    struct noc_s3_psam_reg *reg,
    uint64_t base_addr,
    uint64_t end_addr,
    uint64_t target_id,
    uint64_t region)
{
    uint8_t region_count;

    region_count = GET_NH_REGION_COUNT(reg->sam_unit_info);
    if (region >= region_count) {
        return FWK_E_RANGE;
    }

    if (!NOC_S3_ADDRESS_VALID(base_addr)) {
        return FWK_E_PARAM;
    }

    if (!NOC_S3_ADDRESS_VALID(end_addr + 1)) {
        return FWK_E_PARAM;
    }

    FWK_TRACE(
        MOD_NAME "Programming Region: %" PRId64 " region at: %p", region, reg);
    FWK_TRACE(
        MOD_NAME "Address: Start: 0x%" PRIx64 ", End: 0x%" PRIx64 "",
        base_addr,
        end_addr);
    FWK_TRACE(MOD_NAME "Target: 0x%" PRIx64 "", target_id);

    NOC_S3_DISABLE_REGION(reg->nh_region[region].cfg1_cfg0);

    __DMB();
    /* Set base address */
    reg->nh_region[region].cfg1_cfg0 = NOC_S3_ADDRESS(base_addr);
    /* Set end address */
    reg->nh_region[region].cfg3_cfg2 =
        NOC_S3_ADDRESS(end_addr) | NOC_S3_TARGET_ID(target_id);
    __DMB();

    NOC_S3_ENABLE_REGION(reg->nh_region[region].cfg1_cfg0);

    return FWK_SUCCESS;
}

/*
 * Return the number of the unmapped non hashed region. This also checks if the
 * incoming region is overlapping with the mapped region.
 */
static int find_region_in_psam(
    struct noc_s3_psam_reg *reg,
    uint64_t base_addr,
    uint64_t end_addr,
    uint8_t *region)
{
    uint64_t mapped_start;
    uint64_t mapped_end;
    uint8_t region_count;
    uint8_t count;
    bool found;

    found = false;
    region_count = GET_NH_REGION_COUNT(reg->sam_unit_info);
    fwk_check(region_count != 0);
    for (count = 0; count < region_count; count++) {
        mapped_start = NOC_S3_ADDRESS(reg->nh_region[count].cfg1_cfg0);
        mapped_end = NOC_S3_ADDRESS(reg->nh_region[count].cfg3_cfg2);
        /*
         * This makes sure that all the regions are checked for the overlap.
         * Even if the region is found, this will loop over all of the regions
         * to make sure that there is no overlap with the mapped regions.
         */
        if (!region_enabled(reg->nh_region[count].cfg1_cfg0)) {
            if (!found) {
                found = true;
                *region = count;
            }
            continue;
        }

        if (noc_s3_check_overlap(
                base_addr, end_addr, mapped_start, mapped_end)) {
            FWK_LOG_ERR("Region overlap found");
            FWK_LOG_ERR(
                "Incoming: Start: 0x%" PRIx64 ", End: 0x%" PRIx64 "",
                base_addr,
                end_addr);
            FWK_LOG_ERR(
                "Mapped: Start: 0x%" PRIx64 ", End: 0x%" PRIx64 "",
                mapped_start,
                mapped_end);
            return FWK_E_PARAM;
        }
    }

    if (found) {
        return FWK_SUCCESS;
    }

    FWK_LOG_ERR("Out of PSAM empty regions");
    return FWK_E_RANGE;
}

/* Global PSAM enable. */
static void psam_enable(struct noc_s3_psam_reg *reg)
{
    reg->sam_status |= NOC_S3_SAM_STATUS_SETUP_COMPLETE;
    __DMB();
}

/* Global PSAM disable. */
static void psam_disable(struct noc_s3_psam_reg *reg)
{
    reg->sam_status &= ~NOC_S3_SAM_STATUS_SETUP_COMPLETE;
    __DMB();
}

/*
 * Extract the base offset from the discovery table of the target xSNI node and
 * then parse the list of subfeatures it supports. If the target subfeature is
 * found then its address is returned.
 */
static int get_psam_base(
    struct mod_noc_s3_dev *dev,
    struct noc_s3_psam_reg **psam_reg,
    enum mod_noc_s3_node_type type,
    uint32_t xsni_id)
{
    uintptr_t subfeature_base;
    int status;

    if (!is_psam_supported(type)) {
        FWK_LOG_ERR(MOD_NAME "PSAM not supported for Node(%d)", type);
        return FWK_E_PARAM;
    }

    status = noc_s3_get_subfeature_address(
        dev, type, NOC_S3_NODE_TYPE_PSAM, xsni_id, &subfeature_base);
    if (status == FWK_SUCCESS) {
        *psam_reg = (struct noc_s3_psam_reg *)subfeature_base;
    }

    return status;
}

/*
 * Program regions in the PSAM. This function is called for mapping static
 * regions and assumes that the interconnect is being programmed for the first
 * time and starts from region 0.
 */
static int program_psam_regions(
    struct noc_s3_psam_reg *psam_reg,
    struct mod_noc_s3_psam_region *psam_regions,
    size_t count)
{
    size_t region_idx;
    int status;

    /* Disable all PSAM regions for this node. */
    psam_disable(psam_reg);

    for (region_idx = 0; region_idx < count; region_idx++) {
        status = psam_nhregion_init(
            psam_reg,
            psam_regions[region_idx].base_address,
            (psam_regions[region_idx].base_address +
             psam_regions[region_idx].size) -
                1,
            psam_regions[region_idx].target_id,
            region_idx);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    /* Enable all the regions. */
    psam_enable(psam_reg);

    return FWK_SUCCESS;
}

/* Maps the list of regions for each xSNI node passed in the element config. */
int program_static_mapped_regions(
    struct mod_noc_s3_element_config *config,
    struct mod_noc_s3_dev *dev)
{
    struct mod_noc_s3_comp_config *component_config;
    struct noc_s3_psam_reg *psam_base;
    uint8_t idx;
    int status;

    if ((config == NULL) || (dev == NULL)) {
        return FWK_E_PARAM;
    }

    /* Make sure that the discovery is completed and the table is populated. */
    if (!dev->discovery_completed) {
        status = noc_s3_discovery(dev);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "NoC S3 Device discovery failed.");
            return status;
        }
    }

    component_config = config->component_config;
    /* Program all the xSNI ports. */
    for (idx = 0; idx < config->component_count; idx++) {
        /*
         * Find the target xSNI node and get register base address for system
         * address map subfeature.
         */
        status = get_psam_base(
            dev,
            &psam_base,
            component_config[idx].type,
            component_config[idx].id);
        if (status != FWK_SUCCESS) {
            return status;
        }

        /* Program all the regions in the target xSNI port. */
        status = program_psam_regions(
            psam_base,
            component_config[idx].psam_regions,
            component_config[idx].psam_region_count);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

int map_region_in_psam(
    struct mod_noc_s3_dev *dev,
    struct mod_noc_s3_comp_config *component_config,
    uint8_t *region)
{
    struct mod_noc_s3_psam_region *psam_region;
    struct noc_s3_psam_reg *psam_base;
    uint64_t base_address;
    uint64_t end_address;
    uint32_t target_id;
    int status;

    if ((dev == NULL) || (dev->periphbase == 0)) {
        return FWK_E_PARAM;
    }

    if ((component_config == NULL) ||
        (component_config->psam_regions == NULL) ||
        (component_config->psam_region_count != 1)) {
        return FWK_E_PARAM;
    }

    /* Make sure that the discovery is completed and the table is populated. */
    if (!dev->discovery_completed) {
        status = noc_s3_discovery(dev);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "NoC S3 Device discovery failed.");
            return status;
        }
    }

    status = get_psam_base(
        dev, &psam_base, component_config->type, component_config->id);
    if (status != FWK_SUCCESS) {
        return status;
    }

    psam_region = component_config->psam_regions;
    /* Program all the regions in the the target xSNI. */
    base_address = psam_region->base_address;
    target_id = psam_region->target_id;
    end_address = (psam_region->base_address + psam_region->size) - 1;

    /*
     * Find and return the region number of an empty slot.
     * If the region overlaps with another one then return error.
     */
    status = find_region_in_psam(psam_base, base_address, end_address, region);
    if (status != FWK_SUCCESS) {
        return status;
    }

    psam_disable(psam_base);

    /* Map the address in the target non hashed region. */
    status = psam_nhregion_init(
        psam_base, base_address, end_address, target_id, *region);
    if (status != FWK_SUCCESS) {
        return status;
    }

    psam_enable(psam_base);

    return FWK_SUCCESS;
}

int unmap_region_in_psam(
    struct mod_noc_s3_dev *dev,
    struct mod_noc_s3_comp_config *component_config,
    uint8_t region)
{
    struct noc_s3_psam_reg *psam_base;
    int status;

    if ((dev == NULL) || (dev->periphbase == 0)) {
        return FWK_E_PARAM;
    }

    if (component_config == NULL) {
        return FWK_E_PARAM;
    }

    /* Make sure that the discovery is completed and the table is populated. */
    if (!dev->discovery_completed) {
        status = noc_s3_discovery(dev);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "NoC S3 Device discovery failed.");
            return status;
        }
    }

    /*
     * Find the target xSNI node and get register base address for system
     * address map subfeature.
     */
    status = get_psam_base(
        dev, &psam_base, component_config->type, component_config->id);
    if (status != FWK_SUCCESS) {
        return status;
    }

    psam_disable(psam_base);

    /* Remove the target region from the PSAM mappings. */
    status = psam_nhregion_deinit(psam_base, region);
    if (status != FWK_SUCCESS) {
        return status;
    }

    psam_enable(psam_base);

    return FWK_SUCCESS;
}

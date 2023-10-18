/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for the programming CMN-Cyprus RNSAM.
 */

#include <internal/cmn_cyprus_ctx.h>
#include <internal/cmn_cyprus_reg.h>
#include <internal/cmn_cyprus_rnsam_reg.h>

#include <mod_cmn_cyprus.h>

#include <fwk_log.h>

#include <stddef.h>
#include <stdint.h>

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
/*
 * CMN Cyprus Memory region type name.
 */
static const char
    *const mmap_type_name[MOD_CMN_CYPRUS_MEM_REGION_TYPE_COUNT] = {
        [MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO] = "I/O",
        [MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE] = "System Cache",
        [MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE_SUB] = "Sub-System Cache",
    };
#endif

/* Shared driver context pointer */
static struct cmn_cyprus_ctx *shared_ctx;

static void stall_rnsam_requests(void)
{
    unsigned int rnsam_idx;
    struct cmn_cyprus_rnsam_reg *rnsam_reg;

    for (rnsam_idx = 0; rnsam_idx < shared_ctx->rnsam_count; rnsam_idx++) {
        rnsam_reg = shared_ctx->rnsam_table[rnsam_idx];

        /* Stall the RNSAM requests and enable default target ID selection */
        rnsam_stall(rnsam_reg);
    }
    __sync_synchronize();
}

static void unstall_rnsam_requests(void)
{
    unsigned int rnsam_idx;
    struct cmn_cyprus_rnsam_reg *rnsam_reg;

    for (rnsam_idx = 0; rnsam_idx < shared_ctx->rnsam_count; rnsam_idx++) {
        rnsam_reg = shared_ctx->rnsam_table[rnsam_idx];

        /* Unstall RNSAM requests and disable default target ID selection */
        rnsam_unstall(rnsam_reg);
    }
    __sync_synchronize();
}

static void print_rnsam_config_info(
    const struct mod_cmn_cyprus_mem_region_map *mmap_table,
    size_t mmap_count)
{
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_INFO
    unsigned int idx;
    const struct mod_cmn_cyprus_mem_region_map *region;

    FWK_LOG_INFO(MOD_NAME "Regions to be mapped in RNSAM:");

    for (idx = 0; idx < mmap_count; idx++) {
        region = &mmap_table[idx];

        FWK_LOG_INFO(
            MOD_NAME "  [0x%llx - 0x%llx] %s",
            region->base,
            (region->base + region->size - 1),
            mmap_type_name[region->type]);
    }
#endif
}

static int program_io_region(
    uint64_t region_base,
    uint64_t region_size,
    uint32_t region_idx,
    unsigned int target_node_id)
{
    int status;
    unsigned int rnsam_idx;
    struct cmn_cyprus_rnsam_reg *rnsam;

    if (region_idx >= RNSAM_NON_HASH_MEM_REGION_COUNT) {
        FWK_LOG_ERR(
            MOD_NAME "Error! Invalid non-hashed region %lu", region_idx);
        FWK_LOG_ERR(
            MOD_NAME "Max non-hashed region supported is %u",
            RNSAM_NON_HASH_MEM_REGION_COUNT);
        return FWK_E_DATA;
    }

    /* Iterate through each RNSAM node and configure the region */
    for (rnsam_idx = 0; rnsam_idx < shared_ctx->rnsam_count; rnsam_idx++) {
        rnsam = shared_ctx->rnsam_table[rnsam_idx];

        /* Configure the I/O memory region range */
        status = rnsam_configure_non_hashed_region(
            rnsam, region_idx, region_base, region_size, SAM_NODE_TYPE_HN_I);
        if (status != FWK_SUCCESS) {
            return status;
        }

        /* Configure the I/O memory region target node ID */
        rnsam_set_non_hashed_region_target(rnsam, region_idx, target_node_id);

        /* Mark the region as valid */
        rnsam_set_non_hash_region_valid(rnsam, region_idx);
    }

    return FWK_SUCCESS;
}

static void configure_scg_target_nodes(
    struct cmn_cyprus_rnsam_reg *rnsam,
    const struct mod_cmn_cyprus_mem_region_map *region,
    uint32_t scg_idx)
{
    uint16_t hns_ldid;
    uint16_t hns_count_in_scg;
    unsigned int hn_node_id_idx;
    unsigned int node_id;
    struct cmn_cyprus_hns_info *hns_info;
    const struct mod_cmn_cyprus_config *config;

    config = shared_ctx->config;
    hns_count_in_scg = 0;
    hn_node_id_idx = 0;

    /*
     * Iterate through each HN-S node and configure the target node ID if it
     * falls within the arbitrary SCG square/rectange.
     */
    for (hns_ldid = 0; hns_ldid < shared_ctx->hns_count; hns_ldid++) {
        hns_info = &shared_ctx->hns_info_table[hns_ldid];
        node_id = hns_info->node_id;

        /* Ignore odd node ids if cal mode is set */
        if ((config->hns_cal_mode) && ((node_id % 2) == 1)) {
            continue;
        }

        /* Skip the HN-F node if it is not part of the SCG */
        if (hns_info->scg_idx != scg_idx) {
            continue;
        }

        /* Configure target HN-F node ID */
        rnsam_set_htg_target_hn_nodeid(rnsam, node_id, hn_node_id_idx);

        hn_node_id_idx++;
    }

    /* Adjust the number of HN-S nodes in the SCG based on CAL mode */
    if (config->hns_cal_mode) {
        hns_count_in_scg = (shared_ctx->scg_hns_count[scg_idx] / 2);
    } else {
        hns_count_in_scg = shared_ctx->scg_hns_count[scg_idx];
    }

    /* Configure the number of target HN-S nodes in this syscache group */
    rnsam_set_htg_target_hn_count(rnsam, scg_idx, hns_count_in_scg);
}

static int program_scg_region(
    const struct mod_cmn_cyprus_mem_region_map *scg_region,
    uint32_t scg_idx)
{
    int status;
    unsigned int rnsam_idx;
    struct cmn_cyprus_rnsam_reg *rnsam;

    if (scg_idx >= MAX_SCG_COUNT) {
        FWK_LOG_ERR(MOD_NAME "Error! Invalid SCG region %lu", scg_idx);
        FWK_LOG_ERR(MOD_NAME "Max SCG region supported is %u ", MAX_SCG_COUNT);
        return FWK_E_DATA;
    }

    /* Iterate through each RNSAM node and configure the region */
    for (rnsam_idx = 0; rnsam_idx < shared_ctx->rnsam_count; rnsam_idx++) {
        rnsam = shared_ctx->rnsam_table[rnsam_idx];

        /* Configure the hashed region address range in RNSAM */
        status = rnsam_configure_hashed_region(
            rnsam,
            scg_idx,
            scg_region->base,
            scg_region->size,
            SAM_NODE_TYPE_HN_F);
        if (status != FWK_SUCCESS) {
            return status;
        }

        /* Configure the target nodes for the SCG */
        configure_scg_target_nodes(rnsam, scg_region, scg_idx);

        if (shared_ctx->config->hns_cal_mode) {
            /* Configure the SCG CAL mode support */
            rnsam_enable_htg_cal_mode(rnsam, scg_idx);
        }

        /* Mark the region as valid for comparison */
        rnsam_set_htg_region_valid(rnsam, scg_idx);
    }

    return FWK_SUCCESS;
}

static int program_rnsam_region(
    const struct mod_cmn_cyprus_mem_region_map *region)
{
    int status;
    unsigned int region_idx;

    switch (region->type) {
    case MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO:
        region_idx = shared_ctx->io_region_count++;
        /* Configure non-hashed region */
        status = program_io_region(
            region->base, region->size, region_idx, region->node_id);
        break;

    case MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE:
        region_idx = shared_ctx->scg_count++;
        /* Configure SCG region */
        status = program_scg_region(region, region_idx);
        break;

    default:
        status = FWK_E_DATA;
    }

    return status;
}

int cmn_cyprus_setup_rnsam(struct cmn_cyprus_ctx *ctx)
{
    int status;
    unsigned int region_idx;
    const struct mod_cmn_cyprus_config *config;
    const struct mod_cmn_cyprus_mem_region_map *region;

    if (ctx == NULL) {
        return FWK_E_PARAM;
    }

    /* Initialize the shared context pointer */
    shared_ctx = ctx;

    config = shared_ctx->config;

    FWK_LOG_INFO(MOD_NAME "RNSAM setup start...");

    /* Setup the RNSAM programming context */
    status = setup_rnsam_ctx(shared_ctx->rnsam_table[0]);
    if (status != FWK_SUCCESS) {
        return status;
    }

    print_rnsam_config_info(config->mmap_table, config->mmap_count);

    stall_rnsam_requests();

    /* Configure the regions in RNSAM */
    for (region_idx = 0; region_idx < config->mmap_count; region_idx++) {
        region = &config->mmap_table[region_idx];

        /*
         * Syscache sub-regions are configured as range-based non-hashed regions
         * in HN-F SAM. So, skip configuring these regions in RNSAM.
         */
        if (region->type == MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE_SUB) {
            continue;
        }

        status = program_rnsam_region(region);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    unstall_rnsam_requests();

    FWK_LOG_INFO(MOD_NAME "RNSAM setup complete");

    return FWK_SUCCESS;
}

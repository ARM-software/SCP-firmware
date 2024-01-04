/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for the programming CMN-Cyprus RNSAM.
 */

#include <internal/cmn_cyprus_common.h>
#include <internal/cmn_cyprus_ctx.h>
#include <internal/cmn_cyprus_hns_reg.h>
#include <internal/cmn_cyprus_node_info_reg.h>
#include <internal/cmn_cyprus_reg.h>
#include <internal/cmn_cyprus_rnsam_reg.h>

#include <mod_cmn_cyprus.h>

#include <interface_cmn.h>

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
        [MOD_CMN_CYPRUS_MEM_REGION_TYPE_REMOTE_NON_HASHED] = "Remote Non-Hash",
        [MOD_CMN_CYPRUS_MEM_REGION_TYPE_REMOTE_HASHED] = "Remote Hashed",
    };
#endif

/* Shared driver context pointer */
static struct cmn_cyprus_ctx *shared_ctx;

/*
 * Target Home Node index. Denotes the index at which a given home node ID will
 * programmed in the SYS_CACHE_GRP_HN_NODEID register in RNSAM. This variable
 * is used while programming the target node IDs for SCGs.
 */
static unsigned int hn_node_id_idx = 0;

static inline bool is_non_hash_region_idx_valid(unsigned int region_idx)
{
    if (region_idx < RNSAM_NON_HASH_MEM_REGION_COUNT) {
        return true;
    }

    FWK_LOG_ERR(MOD_NAME "Error! Invalid non-hashed region %u", region_idx);
    FWK_LOG_ERR(
        MOD_NAME "Max non-hashed region supported is %u",
        RNSAM_NON_HASH_MEM_REGION_COUNT);

    return false;
}

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
    uint64_t base;
    unsigned int idx;
    const struct mod_cmn_cyprus_mem_region_map *region;

    FWK_LOG_INFO(MOD_NAME "Regions to be mapped in RNSAM:");

    for (idx = 0; idx < mmap_count; idx++) {
        region = &mmap_table[idx];

        /* Offset the base with chip address space */
        base = (region->base + GET_CHIP_ADDR_OFFSET(shared_ctx));

        FWK_LOG_INFO(
            MOD_NAME "  [0x%llx - 0x%llx] %s",
            base,
            (base + region->size - 1),
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
            MOD_NAME "Error! Invalid non-hashed region %u",
            (unsigned int)region_idx);
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
    const struct mod_cmn_cyprus_mem_region_map *region,
    uint32_t scg_idx)
{
    uint16_t hns_ldid;
    unsigned int node_id;
    unsigned int rnsam_idx;
    struct cmn_cyprus_hns_info *hns_info;
    const struct mod_cmn_cyprus_config *config;
    struct cmn_cyprus_rnsam_reg *rnsam;
    enum mod_cmn_cyprus_mem_region_type region_type;

    config = shared_ctx->config;
    region_type = region->type;

    fwk_assert(
        (region_type == MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE) ||
        (region_type == MOD_CMN_CYPRUS_MEM_REGION_TYPE_REMOTE_HASHED));

    /*
     * Iterate through each HN-S node and configure the target node ID if it
     * falls within the arbitrary SCG square/rectange.
     *
     * Note: For multichip configurations with LCN enabled, the remote chip
     * mesh is assumed to be identical to the local mesh and the HN-S nodes
     * in the local mesh are re-used as target nodes for the remote SCG.
     */
    for (hns_ldid = 0; hns_ldid < shared_ctx->hns_count; hns_ldid++) {
        hns_info = &shared_ctx->hns_info_table[hns_ldid];
        node_id = hns_info->node_id;

        /* Ignore odd node ids if cal mode is set */
        if ((config->hns_cal_mode) && ((node_id % 2) == 1)) {
            continue;
        }

        /* Skip the HN-S node if it is isolated */
        if (is_hns_node_isolated(hns_info->hns) == true) {
            continue;
        }

        /*
         * For local SCGs, skip the HN-S node if it belongs to a different SCG.
         */
        if ((region_type == MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE) &&
            (hns_info->scg_idx != scg_idx)) {
            continue;
        }

        /* Iterate through each RNSAM node and configure the region */
        for (rnsam_idx = 0; rnsam_idx < shared_ctx->rnsam_count; rnsam_idx++) {
            rnsam = shared_ctx->rnsam_table[rnsam_idx];

            /* Configure target HN-S node ID */
            rnsam_set_htg_target_hn_nodeid(rnsam, node_id, hn_node_id_idx);
        }

        /* Increment the global target home node index */
        hn_node_id_idx++;
    }
}

static void configure_scg_hier_hashing(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t scg_idx,
    uint8_t num_cluster_groups)
{
    uint8_t num_hns_per_cluster;
    unsigned int hns_count;

    hns_count = get_scg_hns_count(shared_ctx, scg_idx);

    /* Number of HN-S nodes in a cluster */
    num_hns_per_cluster = (hns_count / num_cluster_groups);

    /* Configure the number of clusters in the first level hierarchy */
    rnsam_set_hier_hash_cluster_groups(rnsam, scg_idx, num_cluster_groups);

    /* Configure the number of hns nodes per cluster */
    rnsam_set_hier_hash_num_hns_per_cluster(
        rnsam, scg_idx, num_hns_per_cluster);

    /*
     * Configure number of address bits needs to shuttered (removed) at
     * second level hierarchy hash based on the cluster count.
     */
    rnsam_set_hier_hash_addr_striping(rnsam, scg_idx, num_cluster_groups);

    /* Enable hierarchical hashing mode */
    rnsam_enable_hier_hash_mode(rnsam, scg_idx);
}

static int program_scg_region(
    const struct mod_cmn_cyprus_mem_region_map *scg_region,
    uint32_t scg_idx)
{
    int status;
    enum sam_node_type target_type;
    unsigned int rnsam_idx;
    uint16_t hns_count_in_scg;
    struct cmn_cyprus_rnsam_reg *rnsam;
    struct mod_cmn_cyprus_rnsam_scg_config *scg_config;

    scg_config = (struct mod_cmn_cyprus_rnsam_scg_config *)&shared_ctx->config
                     ->rnsam_scg_config;

    if (scg_idx >= MAX_SCG_COUNT) {
        FWK_LOG_ERR(
            MOD_NAME "Error! Invalid SCG region %u", (unsigned int)scg_idx);
        FWK_LOG_ERR(MOD_NAME "Max SCG region supported is %u ", MAX_SCG_COUNT);
        return FWK_E_DATA;
    }

    /*
     * If LCN mode is enabled (typically used in multichip configurations),
     * select the target type as HN-S as the Super Home Node (HN-S) has dual
     * functionality, acting as an HN-F for local coherent memory and LCN for
     * remote coherent memory.
     */
    target_type = (shared_ctx->config->enable_lcn == true) ?
        SAM_NODE_TYPE_HN_S :
        SAM_NODE_TYPE_HN_F;

    /* Iterate through each RNSAM node and configure the region */
    for (rnsam_idx = 0; rnsam_idx < shared_ctx->rnsam_count; rnsam_idx++) {
        rnsam = shared_ctx->rnsam_table[rnsam_idx];

        /* Configure the hashed region address range in RNSAM */
        status = rnsam_configure_hashed_region(
            rnsam, scg_idx, scg_region->base, scg_region->size, target_type);
        if (status != FWK_SUCCESS) {
            return status;
        }

        if (scg_region->sec_region_size != 0) {
            /* Configure the secondary hashed region address range in RNSAM */
            status = rnsam_configure_sec_hashed_region(
                rnsam,
                scg_idx,
                scg_region->sec_region_base,
                scg_region->sec_region_size,
                target_type);
            if (status != FWK_SUCCESS) {
                return status;
            }
        }

        if (shared_ctx->config->hns_cal_mode) {
            /* Configure the SCG CAL mode support */
            rnsam_enable_htg_cal_mode(rnsam, scg_idx);
        }

        /* Configure Hierarchical hashing if enabled in config data */
        if (scg_config->scg_hashing_mode ==
            MOD_CMN_CYPRUS_RNSAM_SCG_HIERARCHICAL_HASHING) {
            /* Configure SCG hierarchical hashing */
            configure_scg_hier_hashing(
                rnsam, scg_idx, scg_config->hier_hash_cfg.num_cluster_groups);
        }

        /* Mark the region as valid for comparison */
        rnsam_set_htg_region_valid(rnsam, scg_idx);

        if (scg_region->sec_region_size != 0) {
            rnsam_set_htg_secondary_region_valid(rnsam, scg_idx);
        }

        hns_count_in_scg = get_scg_hns_count(shared_ctx, scg_idx);
        /* Configure the number of target HN-S nodes in this syscache group */
        rnsam_set_htg_target_hn_count(rnsam, scg_idx, hns_count_in_scg);
    }

    /* Set the target Home Nodes for the SCG */
    configure_scg_target_nodes(scg_region, scg_idx);

    return FWK_SUCCESS;
}

static int program_remote_non_hashed_region(
    uint64_t region_base,
    uint64_t region_size,
    unsigned int region_idx,
    unsigned int target_node_id)
{
    int status;
    unsigned int rnsam_idx;
    struct cmn_cyprus_rnsam_reg *rnsam;

    if (is_non_hash_region_idx_valid(region_idx) != true) {
        FWK_LOG_ERR(MOD_NAME "Invalid non-hashed region idx: %u", region_idx);
        return FWK_E_RANGE;
    }

    for (rnsam_idx = 0; rnsam_idx < shared_ctx->rnsam_count; rnsam_idx++) {
        rnsam = shared_ctx->rnsam_table[rnsam_idx];

        /* Configure the remote memory region range and target type */
        status = rnsam_configure_non_hashed_region(
            rnsam, region_idx, region_base, region_size, SAM_NODE_TYPE_CXRA);
        if (status != FWK_SUCCESS) {
            return status;
        }

        /* Configure the remote memory region target node ID */
        rnsam_set_non_hashed_region_target(rnsam, region_idx, target_node_id);

        /* Mark the region as valid */
        rnsam_set_non_hash_region_valid(rnsam, region_idx);
    }

    return FWK_SUCCESS;
}

static int program_rnsam_region(
    const struct mod_cmn_cyprus_mem_region_map *region)
{
    int status;
    unsigned int region_idx;
    uint64_t base;

    switch (region->type) {
    case MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO:
        region_idx = shared_ctx->io_region_count++;
        /* Offset the base with chip address space */
        base = (region->base + GET_CHIP_ADDR_OFFSET(shared_ctx));

        /* Configure non-hashed region */
        status =
            program_io_region(base, region->size, region_idx, region->node_id);
        break;

    case MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE:
        region_idx = shared_ctx->scg_count++;
        /* Configure SCG region */
        status = program_scg_region(region, region_idx);
        break;

    case MOD_CMN_CYPRUS_MEM_REGION_TYPE_REMOTE_NON_HASHED:
        region_idx = shared_ctx->io_region_count++;
        /* Configure remote non-hashed region */
        status = program_remote_non_hashed_region(
            region->base, region->size, region_idx, region->node_id);
        break;

    case MOD_CMN_CYPRUS_MEM_REGION_TYPE_REMOTE_HASHED:
        region_idx = shared_ctx->scg_count++;
        /*
         * When LCN is enabled, for remote SCGs, assume that the number of HN-S
         * nodes is same as that of the SCG0 and override the HNS count.
         */
        shared_ctx->scg_hns_count[region_idx] = shared_ctx->scg_hns_count[0];

        /* Configure remote SCG region */
        status = program_scg_region(region, region_idx);
        break;

    default:
        status = FWK_E_DATA;
    }

    return status;
}

/*
 * Check if the base address of a given region overlaps with another region's
 * address range.
 */
static bool check_addr_overlap(
    uint64_t region1_start,
    uint64_t region2_start,
    uint64_t region2_end)
{
    /* Check if start address falls within the given region address range */
    return ((region1_start >= region2_start) && (region1_start <= region2_end));
}

/*
 * Helper function to check if the given non-hashed region is mapped in the
 * RNSAM. If mapped, check if the target node-id matches the request and
 * return the non-hashed region index.
 */
static bool is_non_hashed_region_mapped(
    struct mod_cmn_cyprus_mem_region_map *mmap,
    unsigned int *region_idx)
{
    int idx;
    unsigned int mapped_node_id;
    uint64_t mapped_base_addr;
    uint64_t mapped_end_addr;
    uint64_t mapped_size;
    struct cmn_cyprus_rnsam_reg *rnsam;

    /*
     * All the regions are identically mapped in all the RNSAM registers.
     * Use only one RNSAM register to check if the requested I/O region is
     * already mapped.
     */
    rnsam = shared_ctx->rnsam_table[0];

    /* Iterate through each non-hashed region mapped in RNSAM */
    for (idx = shared_ctx->io_region_count - 1; idx >= 0; idx--) {
        /*
         * Get the base address and end address of the mapped non-hashed
         * region.
         */
        mapped_base_addr = rnsam_get_non_hashed_region_base(rnsam, idx);
        mapped_size = rnsam_get_non_hashed_region_size(rnsam, idx);
        mapped_end_addr = mapped_size + mapped_base_addr - 1;

        /* Check if the requested region has already been mapped in RNSAM */
        if (!check_addr_overlap(
                mmap->base, mapped_base_addr, mapped_end_addr)) {
            /* Skip the region */
            continue;
        }

        /* Get the programmed target node ID */
        mapped_node_id = rnsam_get_non_hashed_region_target_id(rnsam, idx);

        FWK_LOG_INFO(
            MOD_NAME "Found IO region: %d mapped to Node: %u ",
            idx,
            mapped_node_id);

        /*
         * Check if the programmed target node ID matches the given target node
         * ID.
         */
        if (mapped_node_id !=
            (mmap->node_id &= RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK)) {
            FWK_LOG_ERR(MOD_NAME "Error! Invalid IO region memmap request");
            FWK_LOG_ERR(
                MOD_NAME "0x%llx mapped to different node id", mmap->base);
            FWK_LOG_ERR(
                MOD_NAME "mapped node id %u | requested node id : %u\n",
                mapped_node_id,
                mmap->node_id);

            fwk_trap();
        }

        /* Return the region index of the programmed region */
        *region_idx = idx;

        return true;
    }

    /* The requested non-hashed region is not mapped in the RNSAM */
    return false;
}

static void update_io_region(
    struct mod_cmn_cyprus_mem_region_map *mmap,
    unsigned int region_idx)
{
    unsigned int rnsam_idx;
    struct cmn_cyprus_rnsam_reg *rnsam;

    FWK_LOG_INFO(MOD_NAME "Updating IO region %u", region_idx);
    FWK_LOG_INFO(
        MOD_NAME "  [0x%llx - 0x%llx] %s",
        mmap->base,
        mmap->base + mmap->size - 1,
        mmap_type_name[mmap->type]);

    /* Update the IO region in RNSAM */
    for (rnsam_idx = 0; rnsam_idx < shared_ctx->rnsam_count; rnsam_idx++) {
        rnsam = shared_ctx->rnsam_table[rnsam_idx];

        rnsam_configure_non_hashed_region(
            rnsam, region_idx, mmap->base, mmap->size, SAM_NODE_TYPE_HN_I);

        /* Mark the region as valid */
        rnsam_set_non_hash_region_valid(rnsam, region_idx);
    }
}

static int map_io_region(uint64_t base, size_t size, uint32_t node_id)
{
    int status;
    unsigned int region_idx;
    struct mod_cmn_cyprus_mem_region_map mmap = {
        .base = base,
        .size = size,
        .type = MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,
        .node_id = node_id,
    };

    /* Stall RNSAM requests as the RNSAM is about to be re-configured */
    stall_rnsam_requests();

    /* Check if the given non-hashed region has been mapped */
    if (is_non_hashed_region_mapped(&mmap, &region_idx)) {
        /* Update the exisiting IO region in RNSAM */
        update_io_region(&mmap, region_idx);
    } else {
        region_idx = shared_ctx->io_region_count++;

        FWK_LOG_INFO(MOD_NAME "Mapping IO region in RNSAM");
        FWK_LOG_INFO(
            MOD_NAME "  [0x%llx - 0x%llx] %s",
            base,
            base + size - 1,
            mmap_type_name[mmap.type]);

        /* Program the IO region in RNSAM */
        status = program_io_region(base, size, region_idx, node_id);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    /* Unstall the RNSAM requests */
    unstall_rnsam_requests();

    return FWK_SUCCESS;
}

static struct interface_cmn_memmap_rnsam_api memmap_rnsam_api = {
    .map_io_region = map_io_region,
};

static int configure_cpag_target_nodes(
    uint8_t cpag_id,
    uint8_t ccg_count,
    unsigned int *ccg_ldid,
    struct cmn_cyprus_rnsam_reg *rnsam)
{
    int status;
    uint8_t ccg_idx;
    uint8_t cpag_tgt_idx;
    unsigned int ccg_ra_node_id;
    unsigned int ccg_ra_ldid;
    struct ccg_ra_info *ccg_ra_info_table;
    struct cmn_cyprus_ccg_ra_reg *ccg_ra;

    ccg_ra_info_table = shared_ctx->ccg_ra_info_table;

    /* Configure target CCG RA node IDs in CPAG */
    for (ccg_idx = 0; ccg_idx < ccg_count; ccg_idx++) {
        ccg_ra_ldid = ccg_ldid[ccg_idx];
        ccg_ra = ccg_ra_info_table[ccg_ra_ldid].ccg_ra;
        ccg_ra_node_id = node_info_get_id(ccg_ra->CCG_RA_NODE_INFO);

        /*
         * Note: The following calculation is based on the assumption that the
         * number of CCG ports per CPAG is the same for all the CPAGs in the
         * mesh.
         */
        cpag_tgt_idx = ((cpag_id * ccg_count) + ccg_idx);

        /* Program the CHI node ID of the CCG RA node in the CPAG */
        status =
            rnsam_configure_cpag_target_id(rnsam, cpag_tgt_idx, ccg_ra_node_id);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                MOD_NAME
                "Error! Failed to set CPAG TGT Node ID: %u at idx %u in RNSAM",
                ccg_ra_node_id,
                cpag_tgt_idx);
            return status;
        }
    }

    return FWK_SUCCESS;
}

/*
 * Set CPA mode and CPA Group ID for remote hashed regions. Additionally, if LCN
 * mode is specified, set the target HN-S nodes as LCN bound.
 */
static int configure_remote_hash_region_cpa(
    uint8_t region_idx,
    uint8_t cpag_id,
    struct cmn_cyprus_rnsam_reg *rnsam)
{
    int status;
    unsigned int hns_count;

    /* Enable CPA mode for the remote hashed region */
    hns_count = get_scg_hns_count(shared_ctx, region_idx);
    status = rnsam_enable_hash_region_cpa_mode(rnsam, region_idx, hns_count);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! Failed to enable CPA for hashed region");
        return status;
    }

    /* Set target CPAG ID for the remote hashed region */
    status = rnsam_configure_hash_region_cpag_id(rnsam, region_idx, cpag_id);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! Failed to set CPAG ID for hashed region");
        return status;
    }

    /*
     * If LCN mode is enabled, set the target HN-S nodes in the remote hashed
     * region as LCN bound.
     */
    if (shared_ctx->config->enable_lcn == true) {
        status = rnsam_set_hash_region_lcn_bound(rnsam, region_idx, hns_count);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                MOD_NAME
                "Error! Failed to set LCN bound HN-S nodes for hashed region");
            return status;
        }
    }

    return status;
}

/* Set CPA mode and CPA Group ID for remote non-hashed regions */
static int configure_remote_non_hash_region_cpa(
    uint8_t region_idx,
    uint8_t cpag_id,
    struct cmn_cyprus_rnsam_reg *rnsam)
{
    int status;

    /* Enable CPA mode for the non-hashed region */
    status = rnsam_enable_non_hash_region_cpa_mode(rnsam, region_idx);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! Failed to enable CPA for non-hash region");
        return status;
    }

    /* Configure the CPAG that the requests must be hashed across */
    status =
        rnsam_configure_non_hash_region_cpag_id(rnsam, region_idx, cpag_id);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME
                    "Error! Failed to set CPAG ID for non-hash region");
        return status;
    }

    return status;
}

/* Configure CPA for remote regions in RNSAM */
static int program_rnsam_cpa(
    unsigned int *ccg_ldid,
    const struct mod_cmn_cyprus_cpag_config *cpag_config,
    bool enable_smp_mode,
    enum mod_cmn_cyprus_mem_region_type region_type)
{
    int status;
    uint8_t ccg_count;
    uint8_t cpag_base_idx;
    uint8_t cpag_id;
    uint8_t region_idx;
    unsigned int rnsam_idx;
    struct cmn_cyprus_rnsam_reg *rnsam;

    if ((cpag_config == NULL) || (cpag_config->ccg_count == 0) ||
        (ccg_ldid == NULL)) {
        FWK_LOG_ERR(MOD_NAME "Error! Invalid CPA config");
        return FWK_E_DATA;
    }

    cpag_id = cpag_config->cpag_id;
    ccg_count = cpag_config->ccg_count;

    for (rnsam_idx = 0; rnsam_idx < shared_ctx->rnsam_count; rnsam_idx++) {
        rnsam = shared_ctx->rnsam_table[rnsam_idx];

        if (region_type == MOD_CMN_CYPRUS_MEM_REGION_TYPE_REMOTE_NON_HASHED) {
            /*
             * Region index is the recently configured non-hashed region in the
             * RNSAM.
             */
            region_idx = (shared_ctx->io_region_count - 1);

            status = configure_remote_non_hash_region_cpa(
                region_idx, cpag_id, rnsam);
            if (status != FWK_SUCCESS) {
                return status;
            }
        } else if (
            region_type == MOD_CMN_CYPRUS_MEM_REGION_TYPE_REMOTE_HASHED) {
            /*
             * Region index is the recently configured hashed region in the
             * RNSAM.
             */
            region_idx = (shared_ctx->scg_count - 1);

            status =
                configure_remote_hash_region_cpa(region_idx, cpag_id, rnsam);
            if (status != FWK_SUCCESS) {
                return status;
            }
        } else {
            return FWK_E_PARAM;
        }

        /* Program the number of CML gateways in the CPAG */
        status = rnsam_configure_cpag_ccg_count(rnsam, cpag_id, ccg_count);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME
                        "Error! Failed to set CPAG CCG count in RNSAM");
            return status;
        }

        if (enable_smp_mode == true) {
            /* Configure CPAG port type as SMP */
            status =
                rnsam_set_cpag_port_type(rnsam, cpag_id, CPA_PORT_TYPE_SMP);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(MOD_NAME
                            "Error! Failed to set CPAG port type in RNSAM");
                return status;
            }
        }

        /* Program the CHI node ID of each CML gateway in the CPAG */
        status =
            configure_cpag_target_nodes(cpag_id, ccg_count, ccg_ldid, rnsam);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME
                        "Error! Failed to set CPAG target nodes in RNSAM");
            return status;
        }

        /*
         * The following CPAG base index calculation is based on the assumption
         * that all the CPAG have equal number of CCG nodes.
         */
        cpag_base_idx = (cpag_id * ccg_count);
        /* Set the base index for the CPAG */
        status = rnsam_configure_cpag_base_index(rnsam, cpag_id, cpag_base_idx);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME
                        "Error! Failed to set CPAG base index in RNSAM");
            return status;
        }
    }

    return FWK_SUCCESS;
}

static int program_rnsam_remote_regions(
    const struct mod_cmn_cyprus_cml_config *cml_config)
{
    int status;
    uint8_t region_idx;
    uint8_t region_idx_max;
    const struct mod_cmn_cyprus_remote_region *region;
    const struct mod_cmn_cyprus_mem_region_map *region_mmap;

    region_idx_max = CMN_CYPRUS_MAX_RA_SAM_ADDR_REGION;

    /* Iterate through each remote memory map entry in the CML config */
    for (region_idx = 0; region_idx < region_idx_max; region_idx++) {
        region = &cml_config->remote_mmap_table[region_idx];
        region_mmap = &region->region_mmap;

        if (region_mmap->size == 0) {
            /* Skip empty entries in the table */
            continue;
        }

        FWK_LOG_INFO(
            MOD_NAME "  [0x%llx - 0x%llx] %s",
            region_mmap->base,
            (region_mmap->base + region_mmap->size - 1),
            mmap_type_name[region_mmap->type]);

        /* Configure the region in RNSAM */
        status = program_rnsam_region(region_mmap);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "Error! Failed to configure RNSAM region");
            return status;
        }

        if (cml_config->enable_cpa_mode == true) {
            /*
             * Program CML Port Aggregation (CPA) to enable the RN SAM to
             * distribute requests to CPAGs.
             */
            status = program_rnsam_cpa(
                cml_config->ccg_ldid,
                &cml_config->cpag_config,
                cml_config->enable_smp_mode,
                region_mmap->type);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(MOD_NAME
                            "Error! Failed to enable CPA for remote region");
                return status;
            }
        }
    }

    return FWK_SUCCESS;
}

static int setup_rnsam_remote_regions(void)
{
    int status;
    uint8_t idx;
    const struct mod_cmn_cyprus_config *config;
    const struct mod_cmn_cyprus_cml_config *cml_config;

    config = shared_ctx->config;

    FWK_LOG_INFO(MOD_NAME "RNSAM remote memory regions setup...");

    /* Iterate through each CML configuration */
    for (idx = 0; idx < config->cml_table_count; idx++) {
        cml_config = &config->cml_config_table[idx];

        status = program_rnsam_remote_regions(cml_config);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME
                        "Error! Failed to program RNSAM remote regions");
            return status;
        }
    }

    FWK_LOG_INFO(MOD_NAME "RNSAM remote memory regions setup...Done");

    return FWK_SUCCESS;
}

void get_rnsam_memmap_api(const void **api)
{
    *api = &memmap_rnsam_api;
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

    if ((shared_ctx->multichip_mode == true) &&
        (shared_ctx->config_table->chip_count > 1)) {
        status = setup_rnsam_remote_regions();
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME
                        "Error! Failed to setup remote regions in RNSAM");
            return status;
        }
    }

    unstall_rnsam_requests();

    FWK_LOG_INFO(MOD_NAME "RNSAM setup complete");

    return FWK_SUCCESS;
}

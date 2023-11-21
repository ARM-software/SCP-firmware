/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for the programming HN-SAM.
 */

#include <internal/cmn_cyprus_common.h>
#include <internal/cmn_cyprus_ctx.h>
#include <internal/cmn_cyprus_hns_reg.h>
#include <internal/cmn_cyprus_hnsam_setup.h>
#include <internal/cmn_cyprus_node_info_reg.h>
#include <internal/cmn_cyprus_reg.h>

#include <mod_cmn_cyprus.h>

#include <fwk_log.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Shared driver context pointer */
static const struct cmn_cyprus_ctx *shared_ctx;

static inline bool is_hns_node_isolated(struct cmn_cyprus_hns_reg *hns)
{
    return hns == 0;
}

/*
 * Configure Direct SN mapping.
 */
static int program_hnf_sam_direct_mapping(
    const struct mod_cmn_cyprus_hnf_sam_config *hnf_sam_config)
{
    unsigned int hns_idx;
    unsigned int hns_ldid;
    uint8_t sn_idx;
    struct cmn_cyprus_hns_reg *hns;

    /* Program the SN node id in SN0 index */
    sn_idx = 0;

    /* Program target SN node id in HN-F SAM */
    for (hns_idx = 0; hns_idx < shared_ctx->hns_count; hns_idx++) {
        hns = shared_ctx->hns_info_table[hns_idx].hns;

        if (is_hns_node_isolated(hns)) {
            continue;
        }

        hns_ldid = node_info_get_ldid(hns->NODE_INFO);

        /* Incorrect SN-F table configuration */
        if (hns_ldid >= hnf_sam_config->snf_count) {
            return FWK_E_DATA;
        }

        /* Program SN-F Node ID at the SN0 index */
        hns_configure_sn_node_id(
            hns, hnf_sam_config->snf_table[hns_ldid], sn_idx);
    }

    return FWK_SUCCESS;
}

static uint8_t get_num_hns_per_cluster(void)
{
    uint8_t num_cluster_groups;
    const struct mod_cmn_cyprus_rnsam_scg_config *rnsam_scg_config;

    rnsam_scg_config = &shared_ctx->config->rnsam_scg_config;

    if (rnsam_scg_config->scg_hashing_mode !=
        MOD_CMN_CYPRUS_RNSAM_SCG_HIERARCHICAL_HASHING) {
        /*
         * HN-S clusters are applicable only when hierarchical hashing
         * is enabled.
         */
        return 0;
    }

    num_cluster_groups = rnsam_scg_config->hier_hash_cfg.num_cluster_groups;

    return shared_ctx->hns_count / num_cluster_groups;
}

static uint8_t get_hns_cluster_idx(unsigned int hns_ldid)
{
    uint8_t num_hns_per_cluster;

    num_hns_per_cluster = get_num_hns_per_cluster();

    if (num_hns_per_cluster != 0) {
        return (hns_ldid / num_hns_per_cluster);
    } else {
        return 0;
    }
}

/*
 * Program the target SN-F node IDs based on the range-based hashing SN mode.
 */
static int configure_hashed_sn_nodes(
    struct cmn_cyprus_hns_reg *hns,
    enum mod_cmn_cyprus_hnf_sam_hashed_mode sn_mode,
    const unsigned int *snf_table,
    size_t snf_count)
{
    unsigned int hns_ldid;
    unsigned int snf_table_idx;
    unsigned int sn_node_id;
    uint8_t hns_cluster_idx;
    uint8_t sn_idx;
    uint8_t sn_count;

    if (sn_mode == MOD_CMN_CYPRUS_HNF_SAM_HASHED_MODE_3_SN) {
        sn_count = 3;
    } else {
        FWK_LOG_ERR(MOD_NAME "Error! Unsupported SN mode");
        return FWK_E_DATA;
    }

    /* Read the logical id of the HN-S node */
    hns_ldid = node_info_get_ldid(hns->NODE_INFO);

    /* Iterate through indices of SN-F nodes present within the cluster */
    for (sn_idx = 0; sn_idx < sn_count; sn_idx++) {
        if (shared_ctx->config->rnsam_scg_config.scg_hashing_mode ==
            MOD_CMN_CYPRUS_RNSAM_SCG_HIERARCHICAL_HASHING) {
            /*
             * If hierarchical hashing is enabled, then HN-Fs are grouped
             * together as clusters and each cluster has the same SN-F target
             * ID. Use the RNSAM hierarchical hashing configuration to calculate
             * the cluster index to which the current HN-S node belongs to. This
             * info is used to select the target SN-F from the SN-F table.
             */
            hns_cluster_idx = get_hns_cluster_idx(hns_ldid);

            /* Get the index of the SN-F node in the SN-F table */
            snf_table_idx = ((hns_cluster_idx * sn_count) + sn_idx);
        } else {
            /* Get the index of the SN-F node in the SN-F table */
            snf_table_idx = ((hns_ldid * sn_count) + sn_idx);
        }

        /* Check if the snf table index is valid */
        if (snf_table_idx >= snf_count) {
            return FWK_E_DATA;
        }

        /* Get the target SN-F node ID from the SN-F table */
        sn_node_id = snf_table[snf_table_idx];

        /* Configure the SN node ID */
        hns_configure_sn_node_id(hns, sn_node_id, sn_idx);
    }

    return FWK_SUCCESS;
}

static int program_hnf_sam_range_based_hashing(
    const struct mod_cmn_cyprus_hnf_sam_config *hnf_sam_config)
{
    int status;
    unsigned int hns_idx;
    struct cmn_cyprus_hns_reg *hns;
    struct mod_cmn_cyprus_hnf_sam_range_based_hashing_config *hashing_cfg;

    hashing_cfg = (struct mod_cmn_cyprus_hnf_sam_range_based_hashing_config
                       *)&hnf_sam_config->hashed_mode_config;

    /* Only 3-SN hashing mode is supported */
    if (hashing_cfg->sn_mode != MOD_CMN_CYPRUS_HNF_SAM_HASHED_MODE_3_SN) {
        return FWK_E_DATA;
    }

    for (hns_idx = 0; hns_idx < shared_ctx->hns_count; hns_idx++) {
        hns = shared_ctx->hns_info_table[hns_idx].hns;

        if (is_hns_node_isolated(hns)) {
            continue;
        }

        /* Program the target SN node IDs */
        status = configure_hashed_sn_nodes(
            hns,
            hashing_cfg->sn_mode,
            hnf_sam_config->snf_table,
            hnf_sam_config->snf_count);
        if (status != FWK_SUCCESS) {
            return status;
        }

        /* Configure bit position of top_address_bit0 */
        hns_configure_top_address_bit(
            hns,
            MOD_CMN_CYPRUS_HNS_SAM_TOP_ADDRESS_BIT0,
            hashing_cfg->top_address_bit0);

        /* Configure bit position of top_address_bit1 */
        hns_configure_top_address_bit(
            hns,
            MOD_CMN_CYPRUS_HNS_SAM_TOP_ADDRESS_BIT1,
            hashing_cfg->top_address_bit1);

        hns_enable_sn_mode(hns, hashing_cfg->sn_mode);
    }

    return FWK_SUCCESS;
}

/* Configure range based non-hashed memory region */
static int configure_non_hashed_region(
    const struct mod_cmn_cyprus_mem_region_map *region,
    unsigned int non_hashed_region_idx)
{
    bool hnsam_range_comp_en_mode;
    unsigned int hns_idx;
    uint64_t base;
    struct cmn_cyprus_hns_reg *hns;

    /* Only 2 range-based memory regions can be configured */
    if (non_hashed_region_idx > 1) {
        return FWK_E_DATA;
    }

    /*
     * Get Range based address comparison mode status in HN-F SAM.
     *
     * Note: This value is read from only one HN-S node as it is identical
     * in all the HN-S nodes.
     */
    hns = shared_ctx->hns_info_table[0].hns;
    hnsam_range_comp_en_mode = hns_is_range_comparison_mode_enabled(hns);

    /* Offset the base with chip address space */
    base = (region->base + GET_CHIP_ADDR_OFFSET(shared_ctx));

    /* Program target SN node id in HN-F SAM */
    for (hns_idx = 0; hns_idx < shared_ctx->hns_count; hns_idx++) {
        hns = shared_ctx->hns_info_table[hns_idx].hns;

        if (is_hns_node_isolated(hns)) {
            continue;
        }

        /* Configure non-hashed region address range */
        hns_configure_non_hashed_region_addr_range(
            hns,
            hnsam_range_comp_en_mode,
            base,
            region->size,
            non_hashed_region_idx);

        /* Configure the target SN node ID for direct mapping mode */
        hns_configure_non_hashed_region_sn_node_id(
            hns, region->node_id, non_hashed_region_idx);

        /* Set the region as valid */
        hns_set_non_hashed_region_valid(hns, non_hashed_region_idx);
    }
    return FWK_SUCCESS;
}

/* Program System Cache Sub regions in HN-F SAM */
static int program_syscache_sub_regions(void)
{
    int status;
    unsigned int region_idx;
    unsigned int syscache_sub_region_count;
    const struct mod_cmn_cyprus_config *config;

    config = shared_ctx->config;
    syscache_sub_region_count = 0;

    /*
     * Program syscache sub-regions in HN-F SAM.
     */
    for (region_idx = 0; region_idx < config->mmap_count; region_idx++) {
        const struct mod_cmn_cyprus_mem_region_map *region =
            &config->mmap_table[region_idx];

        /* Skip non-syscache sub-regions */
        if (region->type != MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE_SUB) {
            continue;
        }

        status = configure_non_hashed_region(region, syscache_sub_region_count);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                MOD_NAME "Error! Unable to program syscache sub-region %u",
                syscache_sub_region_count);
            return status;
        }

        syscache_sub_region_count++;
    }

    return FWK_SUCCESS;
}

static void configure_hns_pwpr(void)
{
    unsigned int hns_idx;
    struct cmn_cyprus_hns_reg *hns;

    /* Iterate through each HN-S node and configure the HN-F SAM */
    for (hns_idx = 0; hns_idx < shared_ctx->hns_count; hns_idx++) {
        hns = shared_ctx->hns_info_table[hns_idx].hns;

        if (is_hns_node_isolated(hns)) {
            continue;
        }

        /* Set policy to ON */
        hns_set_pwpr_policy(hns, MOD_CMN_CYPRUS_HNS_PWPR_POLICY_ON);

        /* Set operational mode to Full Associativity Mode (FAM) */
        hns_set_pwpr_op_mode(hns, MOD_CMN_CYPRUS_HNS_PWPR_OP_MODE_FAM);

        /* Enable dynamic transition */
        hns_set_pwpr_dynamic_enable(hns);
    }
}

int cmn_cyprus_setup_hnf_sam(struct cmn_cyprus_ctx *ctx)
{
    int status;
    unsigned int hns_idx;
    struct cmn_cyprus_hns_reg *hns;
    const struct mod_cmn_cyprus_hnf_sam_config *hnf_sam_config;

    if (ctx == NULL) {
        return FWK_E_DATA;
    }

    /* Initialize the shared context pointer */
    shared_ctx = (const struct cmn_cyprus_ctx *)ctx;
    hnf_sam_config = &shared_ctx->config->hnf_sam_config;

    FWK_LOG_INFO(MOD_NAME "HN-F SAM setup start...");

    /* Setup the HN-S programming context */
    for (hns_idx = 0; hns_idx < shared_ctx->hns_count; hns_idx++) {
        hns = shared_ctx->hns_info_table[hns_idx].hns;

        if (is_hns_node_isolated(hns) == true) {
            continue;
        }

        /*
         * Read the static configuration from the first non-isolated HN-S node
         * in the mesh and setup the HN-S context.
         */
        status = setup_hns_ctx(hns);
        if (status != FWK_SUCCESS) {
            return status;
        }
        break;
    }

    switch (hnf_sam_config->hnf_sam_mode) {
    case MOD_CMN_CYPRUS_HNF_SAM_MODE_DIRECT_MAPPING:
        /* Legacy CMN mode: Direct SN mapping */
        status = program_hnf_sam_direct_mapping(hnf_sam_config);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "Error in direct SN mapping");
        }
        break;

    case MOD_CMN_CYPRUS_HNF_SAM_MODE_RANGE_BASED_HASHING:
        /* Range based: Hashed SN mapping */
        status = program_hnf_sam_range_based_hashing(hnf_sam_config);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "Error in range based hashed SN mapping");
        }
        break;

    default:
        status = FWK_E_DATA;
    }

    if (status != FWK_SUCCESS) {
        return status;
    }

    /* Program syscache sub-regions as range-based non-hashed regions */
    status = program_syscache_sub_regions();
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* Configure the power policy registers */
    configure_hns_pwpr();

    FWK_LOG_INFO(MOD_NAME "HN-F SAM setup complete");

    return FWK_SUCCESS;
}

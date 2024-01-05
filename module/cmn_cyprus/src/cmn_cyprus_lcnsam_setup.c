/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for the programming LCN SAM.
 */

#include <internal/cmn_cyprus_common.h>
#include <internal/cmn_cyprus_ctx.h>
#include <internal/cmn_cyprus_hns_reg.h>
#include <internal/cmn_cyprus_lcnsam_setup.h>
#include <internal/cmn_cyprus_reg.h>

#include <mod_cmn_cyprus.h>

#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* SCG region index 0 */
#define SCG_IDX_0 0

/* Shared driver context pointer */
static const struct cmn_cyprus_ctx *shared_ctx;

/* Global variable to keep track of the LCN HTG region indices */
static uint8_t lcn_htg_idx;

/*
 * Helper function to check if the given region is a remote HTG with non-zero
 * region size.
 */
static inline bool is_remote_htg_region(
    const struct mod_cmn_cyprus_mem_region_map *mmap)
{
    return (
        (mmap->size != 0) &&
        (mmap->type == MOD_CMN_CYPRUS_MEM_REGION_TYPE_REMOTE_HASHED));
}

/* Helper function to program remote HTG region in the LCN SAM */
static int program_lcn_htg_region(
    uint8_t region_idx,
    const struct mod_cmn_cyprus_mem_region_map *region_mmap,
    unsigned int scg_hns_count,
    uint8_t cpag_id)
{
    int status;
    unsigned int hns_idx;
    struct cmn_cyprus_hns_reg *hns;

    for (hns_idx = 0; hns_idx < shared_ctx->hns_count; hns_idx++) {
        hns = shared_ctx->hns_info_table[hns_idx].hns;

        /* Skip isolated HN-S nodes */
        if (is_hns_node_isolated(hns) == false) {
            continue;
        }

        /* Configure the address range of the HTG region */
        status =
            hns_configure_lcn_htg_region_range(hns, region_idx, region_mmap);
        if (status != FWK_SUCCESS) {
            return status;
        }

        /* Set the target type as CXRA */
        hns_set_lcn_htg_region_target_type(
            hns, region_idx, LCN_SAM_NODE_TYPE_CXRA);

        /* Set the target HN-S count in the remote HTG */
        hns_set_lcn_htg_region_hn_count(hns, region_idx, scg_hns_count);

        /* Enable CAL mode */
        if (shared_ctx->config->hns_cal_mode == true) {
            hns_enable_lcn_htg_cal_mode(hns, region_idx);
        }

        /* Enable CPA mode for the remote HTG */
        status = hns_enable_lcn_htg_cpa_mode(hns, region_idx, scg_hns_count);
        if (status != FWK_SUCCESS) {
            return status;
        }

        /* Set target CPA group ID for the remote HTG */
        hns_set_lcn_htg_cpag_id(hns, region_idx, scg_hns_count, cpag_id);

        /* Mark the remote HTG region as valid for comparison */
        hns_set_lcn_htg_region_valid(hns, region_idx);

        /* Set the secondary address range as valid for comparison */
        if (region_mmap->sec_region_size != 0) {
            hns_set_lcn_htg_sec_region_valid(hns, region_idx);
        }
    }

    return FWK_SUCCESS;
}

static int program_remote_htg_regions(
    const struct mod_cmn_cyprus_remote_region *remote_mmap_table,
    const struct mod_cmn_cyprus_cpag_config *cpag_config)
{
    int status;
    uint8_t idx;
    uint8_t cpag_id;
    unsigned int scg_hns_count;
    const struct mod_cmn_cyprus_remote_region *region;

    cpag_id = cpag_config->cpag_id;

    for (idx = 0; idx < CMN_CYPRUS_MAX_RA_SAM_ADDR_REGION; idx++) {
        region = &remote_mmap_table[idx];

        /* Skip regions other than remote HTG */
        if (is_remote_htg_region(&region->region_mmap) != true) {
            continue;
        }

        /*
         * The following calculation is based on the assumption that there's one
         * local SCG per chip and the remote SCG's HN-S count is equal to that
         * of the local SCG when LCN is enabled.
         */
        scg_hns_count = get_scg_hns_count(shared_ctx, SCG_IDX_0);

        /* Program the remote HTG region in LCN SAM */
        status = program_lcn_htg_region(
            lcn_htg_idx, &region->region_mmap, scg_hns_count, cpag_id);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                MOD_NAME "Error! LCN HTG region %u setup failed", lcn_htg_idx);
            return status;
        }

        lcn_htg_idx++;
    }

    return FWK_SUCCESS;
}

int cmn_cyprus_setup_lcn_sam(struct cmn_cyprus_ctx *ctx)
{
    int status;
    uint8_t idx;
    const struct mod_cmn_cyprus_cml_config *cml_config;
    const struct mod_cmn_cyprus_cpag_config *cpag_config;

    if (ctx == NULL) {
        return FWK_E_PARAM;
    }

    FWK_LOG_INFO(MOD_NAME "LCN SAM setup start...");

    shared_ctx = ctx;

    /* Setup LCN SAM programming context */
    status = setup_lcnsam_ctx(shared_ctx->rnsam_table[0]);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! Failed to setup LCN SAM context");
        return status;
    }

    /* Program the LCN SAM to direct the LCN bound traffic to remote chips */
    for (idx = 0; idx < shared_ctx->config->cml_table_count; idx++) {
        cml_config = &shared_ctx->config->cml_config_table[idx];
        cpag_config = &cml_config->cpag_config;

        /* Configure the remote HTG regions in LCN SAM */
        status = program_remote_htg_regions(
            cml_config->remote_mmap_table, cpag_config);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "Error! LCN SAM setup failed");
            return status;
        }
    }

    FWK_LOG_INFO(MOD_NAME "LCN SAM setup start...Done");

    return FWK_SUCCESS;
}

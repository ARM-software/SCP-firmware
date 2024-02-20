/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for programming the CML.
 */

#include <internal/cmn_cyprus_ccg_ha_reg.h>
#include <internal/cmn_cyprus_ccg_ra_reg.h>
#include <internal/cmn_cyprus_cml_setup.h>
#include <internal/cmn_cyprus_ctx.h>
#include <internal/cmn_cyprus_reg.h>

#include <mod_cmn_cyprus.h>

#include <fwk_log.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>

#define MAX_RA_SAM_REGION_IDX CMN_CYPRUS_MAX_RA_SAM_ADDR_REGION

/* Maximum Agent ID in AgentID-to-LinkID mapping in CCG nodes */
#define MAX_AGENT_ID 63

/* Shared driver context pointer */
static const struct cmn_cyprus_ctx *shared_ctx;

/* Program Requesting Agent System Address Map (RA SAM) */
static int program_ra_sam(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra,
    const struct mod_cmn_cyprus_remote_region *remote_mmap_table)
{
    int status;
    uint8_t region_idx;
    uint64_t base;
    uint64_t size;
    const struct mod_cmn_cyprus_remote_region *region;

    FWK_LOG_INFO(MOD_NAME "Programming RA SAM...");

    for (region_idx = 0; region_idx < MAX_RA_SAM_REGION_IDX; region_idx++) {
        region = &remote_mmap_table[region_idx];
        base = region->region_mmap.base;
        size = region->region_mmap.size;

        /* Skip empty entries in the table */
        if (size == 0) {
            continue;
        }

        FWK_LOG_INFO(
            MOD_NAME "  [0x%llx - 0x%llx] Target HAID: %u",
            base,
            (base + size - 1),
            region->target_haid);

        /* Configure the remote region address range */
        status =
            ccg_ra_configure_sam_addr_region(ccg_ra, region_idx, base, size);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                MOD_NAME "Error! Failed to configure RA SAM region %u",
                region_idx);
            return status;
        }

        /* Configure the target HAID for remote region */
        status = ccg_ra_configure_sam_addr_target_haid(
            ccg_ra, region_idx, region->target_haid);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                MOD_NAME "Error! Failed to set target for RA SAM region %u",
                region_idx);
            return status;
        }

        /* Set the remote region as valid for address range comparison */
        status = ccg_ra_set_sam_addr_region_valid(ccg_ra, region_idx);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                MOD_NAME "Error! Failed to set RA SAM region %u as valid",
                region_idx);
            return status;
        }
    }

    FWK_LOG_INFO(MOD_NAME "Programming RA SAM...Done");

    return FWK_SUCCESS;
}

static int program_agentid_to_linkid(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra,
    struct cmn_cyprus_ccg_ha_reg *ccg_ha)
{
    int status;
    uint8_t agent_id;

    for (agent_id = 0; agent_id <= MAX_AGENT_ID; agent_id++) {
        /*
         * Set the AgentID-to-LinkID mapping as valid.
         *
         * Note: CCG nodes only support LinkID 0 which is the reset value in the
         * CCG node AgentID-to-LinkID LUT registers. Hence, just set the LinkID
         * mapping as valid, skipping the configuration.
         */
        status = ccg_ra_set_agentid_to_linkid_valid(ccg_ra, agent_id);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = ccg_ha_set_agentid_to_linkid_valid(ccg_ha, agent_id);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

static int program_cml(const struct mod_cmn_cyprus_cml_config *cml_config)
{
    int status;
    struct cmn_cyprus_ccg_ha_reg *ccg_ha;
    struct ccg_ha_info *ccg_ha_info_table;
    struct cmn_cyprus_ccg_ra_reg *ccg_ra;
    struct ccg_ra_info *ccg_ra_info_table;

    ccg_ra_info_table = shared_ctx->ccg_ra_info_table;
    ccg_ra = ccg_ra_info_table[cml_config->ccg_ldid].ccg_ra;

    FWK_LOG_INFO(
        MOD_NAME "Configuring Chip%u CCG%u",
        shared_ctx->chip_id,
        cml_config->ccg_ldid);

    /*
     * Program Requesting Agent System Address Map (RA SAM) to determine the
     * target Home Agent ID (HAID) for remote address regions.
     */
    status = program_ra_sam(ccg_ra, cml_config->remote_mmap_table);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! RA SAM programming failed");
        return status;
    }

    ccg_ha_info_table = shared_ctx->ccg_ha_info_table;
    ccg_ha = ccg_ha_info_table[cml_config->ccg_ldid].ccg_ha;

    /*
     * Assign LinkIDs to remote CML protocol links. Remote agents, RAs or HAs,
     * are identified using their RAID or HAID. Each remote RA or HA that a CML
     * gateway can communicate with must be behind only one link.
     */
    status = program_agentid_to_linkid(ccg_ra, ccg_ha);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! AgentID-to-LinkID programming failed");
        return status;
    }

    return status;
}

int cmn_cyprus_setup_cml(struct cmn_cyprus_ctx *ctx)
{
    int status;
    uint8_t idx;
    const struct mod_cmn_cyprus_cml_config *cml_config;

    if (ctx == NULL) {
        return FWK_E_PARAM;
    }

    shared_ctx = ctx;

    FWK_LOG_INFO(MOD_NAME "Programming CML...");

    for (idx = 0; idx < shared_ctx->config->cml_table_count; idx++) {
        cml_config = &shared_ctx->config->cml_config_table[idx];

        status = program_cml(cml_config);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "Error! CML Programming failed");
            return status;
        }
    }

    FWK_LOG_INFO(MOD_NAME "Programming CML...Done");

    return FWK_SUCCESS;
}

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
#include <internal/cmn_cyprus_ccla_reg.h>
#include <internal/cmn_cyprus_cml_setup.h>
#include <internal/cmn_cyprus_ctx.h>
#include <internal/cmn_cyprus_hns_reg.h>
#include <internal/cmn_cyprus_node_info_reg.h>
#include <internal/cmn_cyprus_reg.h>

#include <mod_cmn_cyprus.h>
#include <mod_timer.h>

#include <fwk_log.h>
#include <fwk_math.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_RA_SAM_REGION_IDX CMN_CYPRUS_MAX_RA_SAM_ADDR_REGION

/* Maximum Agent ID in AgentID-to-LinkID mapping in CCG nodes */
#define MAX_AGENT_ID 63

/* CML Protocol Link 0 */
#define CML_LINK_0 0

/* CCG RA and CCG HA link control register macros */
#define LINK_CTL_SMP_EN_VAL 1

/* CCLA Upper Link Layer (ULL) CTL register bitfield values */
#define ULL_CTL_ULL_TO_ULL_EN_VAL   1
#define ULL_CTL_SEND_VD_INIT_EN_VAL 1

/* CCLA Upper Link Layer (ULL) Status register bitfield values */
#define ULL_STATUS_ULL_RUN_STATE 1

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

/* Helper function to program the LDID to RAID LUT in CCG RA node */
static int program_ccg_ra_ldid_to_raid_lut(struct cmn_cyprus_ccg_ra_reg *ccg_ra)
{
    int status;
    uint8_t rnf_ldid;
    uint8_t rni_ldid;
    uint8_t rnd_ldid;
    uint16_t offset_idx;
    uint16_t raid;
    uint16_t raid_idx;

    /* Get chip specific RAID base index */
    offset_idx = (shared_ctx->chip_id * shared_ctx->max_rn_count);

    /* Assign expanded RAID to RN-F LDIDs */
    for (rnf_ldid = 0; rnf_ldid < shared_ctx->rnf_count; rnf_ldid++) {
        raid_idx = (offset_idx + rnf_ldid);
        raid = shared_ctx->raid_table[raid_idx];

        status = ccg_ra_configure_rnf_ldid_to_exp_raid(ccg_ra, rnf_ldid, raid);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                MOD_NAME "Error! Failed to assign RAID for RN-F LDID: %u",
                rnf_ldid);
            return status;
        }
    }

    /*
     * Assign expanded RAID to RN-I LDIDs.
     *
     * Note: On an SMP link, caching agents and I/O coherent agents can be
     * assigned IDs from the same ID space. For example, an RN-I and RN-F
     * can have the same ID.
     */
    for (rni_ldid = 0; rni_ldid < shared_ctx->rni_count; rni_ldid++) {
        raid_idx = (offset_idx + rni_ldid);
        raid = shared_ctx->raid_table[raid_idx];

        status = ccg_ra_configure_rni_ldid_to_exp_raid(ccg_ra, rni_ldid, raid);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                MOD_NAME "Error! Failed to assign RAID for RN-I LDID: %u",
                rni_ldid);
            return status;
        }
    }

    /* Assign expanded RAID to RN-D LDIDs */
    for (rnd_ldid = 0; rnd_ldid < shared_ctx->rnd_count; rnd_ldid++) {
        raid_idx = (offset_idx + rnd_ldid);
        raid = shared_ctx->raid_table[raid_idx];

        status = ccg_ra_configure_rnd_ldid_to_exp_raid(ccg_ra, rnd_ldid, raid);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                MOD_NAME "Error! Failed to assign RAID for RN-D LDID: %u",
                rnd_ldid);
            return status;
        }
    }

    return FWK_SUCCESS;
}

/* Helper function to program the RAID to LDID LUT in CCG HA node */
static int program_ccg_ha_raid_to_ldid_lut(struct cmn_cyprus_ccg_ha_reg *ccg_ha)
{
    int status;
    uint8_t chip_id;
    uint8_t offset_idx;
    uint16_t ra_idx;
    uint16_t end_idx;
    uint16_t raid;
    uint16_t remote_ldid;
    const uint8_t chip_count = shared_ctx->config_table->chip_count;

    /*
     * The LDID values for remote RN‑Fs must be greater than those values that
     * are used by the local RN‑F nodes.
     */
    remote_ldid = shared_ctx->max_rn_count;

    /* Assign LDIDs for caching Request Agents in the remote chips */
    for (chip_id = 0; chip_id < chip_count; chip_id++) {
        if (chip_id == shared_ctx->chip_id) {
            /* Skip local chip */
            continue;
        }

        offset_idx = (chip_id * shared_ctx->max_rn_count);
        end_idx = (offset_idx + shared_ctx->max_rn_count);
        for (ra_idx = offset_idx; ra_idx < end_idx; ra_idx++) {
            raid = shared_ctx->raid_table[ra_idx];
            /* Configure LDID for the remote request agent */
            status = ccg_ha_configure_raid_to_ldid(ccg_ha, raid, remote_ldid);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(
                    MOD_NAME "Error! Failed to assign LDID:%u for RAID:%u",
                    remote_ldid,
                    raid);
                return status;
            }

            /* Mark the remote request agent as a caching agent */
            status = ccg_ha_set_raid_as_rnf(ccg_ha, raid);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(
                    MOD_NAME "Error! Failed to set RAID:%u as RN-F", raid);
                return status;
            }
            remote_ldid++;
        }
    }

    return FWK_SUCCESS;
}

/*
 * Helper function to assign unique LDID for each remote caching agent that can
 * send requests to HNs (HN‑F, HN‑I, HN‑D, and HN‑P) as HN‑Ss use the LDID of
 * remote caching agents for Snoop Filter (SF) tracking.
 */
static int program_hns_ldid_to_rn_nodeid(
    unsigned int ccg_ha_nodeid,
    uint8_t remote_chip_id)
{
    int status;
    unsigned int ldid;
    uint8_t rn_idx;
    uint8_t hns_idx;
    uint16_t rn_max;
    struct cmn_cyprus_hns_info *hns_info_table;
    struct cmn_cyprus_hns_reg *hns;

    rn_idx = 0;

    /*
     * LDID to node id values for the local RN-Fs are pre-populated on reset.So,
     * set the start LDID value for the remote RN-Fs ignoring the local RN-F
     * LDIDs.
     */
    if (remote_chip_id == 0) {
        ldid = shared_ctx->rnf_count;
    } else {
        ldid = (remote_chip_id * shared_ctx->rnf_count);
    }

    hns_info_table = shared_ctx->hns_info_table;

    /*
     * In hierarchical caching, the entire remote chip is tracked using an LDID
     * instead of individual remote RN-Fs.
     */
    rn_max =
        (shared_ctx->config->enable_lcn == true) ? 1 : shared_ctx->rnf_count;
    /*
     * Assign LDIDs to remote caching agents.
     *
     * Note: The remote chips are assumed to have the same number of caching
     * agents as the local chip.
     */
    while (rn_idx < rn_max) {
        for (hns_idx = 0; hns_idx < shared_ctx->hns_count; hns_idx++) {
            hns = hns_info_table[hns_idx].hns;

            if (is_hns_node_isolated(hns) == true) {
                /* Skip isolated HN-S nodes */
                continue;
            }

            /* Assign unique LDID to the remote caching agent */
            status = hns_configure_rn_node_id(hns, ldid, ccg_ha_nodeid);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(
                    MOD_NAME
                    "Error! Failed to set CCG HA Node id: %u at LDID: %u",
                    ccg_ha_nodeid,
                    ldid);
                return status;
            }

            /* Set the caching agent as remote */
            status = hns_set_rn_node_remote(hns, ldid);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(
                    MOD_NAME "Error! Failed to set LDID: %u as remote in HN-S",
                    ldid);
                return status;
            }

            /*
             * Set the source type for the remote caching agent. For all remote
             * caching agents, the source type must be programmed to 0b1100
             * (CHI‑E) as CCG HA is a proxy for all remote caching agents.
             */
            status = hns_set_rn_node_src_type(
                hns, ldid, CMN_CYPRUS_HNS_RN_SRC_TYPE_CHI_E);
            if (status != FWK_SUCCESS) {
                FWK_LOG_ERR(
                    MOD_NAME
                    "Error! Failed to set src type for LDID: %u in HN-S",
                    ldid);
                return status;
            }
        }
        rn_idx++;
        ldid++;
    }

    return FWK_SUCCESS;
}

/* Helper function to enable Symmetric Multiprocessor (SMP) mode */
static int enable_cml_smp_mode(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra,
    struct cmn_cyprus_ccg_ha_reg *ccg_ha)
{
    int status;

    /*
     * SMP connection is enabled by setting the lnk<X>_smp_mode_en bit in the
     * CCG RA and CCG HA protocol link control registers.
     */
    status = ccg_ra_set_cml_link_ctl(
        ccg_ra, CML_LINK_0, CCG_RA_LINK_CTL_SMP_EN, LINK_CTL_SMP_EN_VAL);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! Failed to set CCG RA SMP mode enable bit");
        return status;
    }

    status = ccg_ha_set_cml_link_ctl(
        ccg_ha, CML_LINK_0, CCG_HA_LINK_CTL_SMP_EN, LINK_CTL_SMP_EN_VAL);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! Failed to set CCG HA SMP mode enable bit");
        return status;
    }

    return status;
}

/*
 * Helper function to check the status of the Upper link layer direct connect
 * (ull to ull) mode state.
 */
static bool is_ccla_ull_in_run_state(void *ccla_reg_ptr)
{
    int status;
    uint8_t value;
    struct cmn_cyprus_ccla_reg *ccla_reg;

    ccla_reg = (struct cmn_cyprus_ccla_reg *)ccla_reg_ptr;

    /* Check if Rx ULL state is in run state */
    status = ccla_get_ull_status(ccla_reg, CCLA_ULL_STATUS_RX_ULL, &value);
    if ((status != FWK_SUCCESS) || (value != ULL_STATUS_ULL_RUN_STATE)) {
        return false;
    }

    /* Check if Tx ULL state is in run state */
    status = ccla_get_ull_status(ccla_reg, CCLA_ULL_STATUS_TX_ULL, &value);
    if ((status != FWK_SUCCESS) || (value != ULL_STATUS_ULL_RUN_STATE)) {
        return false;
    }

    return true;
}

static int enable_ccla_direct_connect_mode(struct cmn_cyprus_ccla_reg *ccla)
{
    int status;

    /*
     * CCLA Direct connect mode is enabled by the following steps:
     * 1) Set ull_to_ull_en bit on both sides in CCLA register.
     * 2) After ull_to_ull_en is set on both sides, set the send_vd_init bit on
     *    both sides in CCLA register.
     */
    status = ccla_set_ull_ctl(
        ccla, CCLA_ULL_CTL_ULL_TO_ULL_EN, ULL_CTL_ULL_TO_ULL_EN_VAL);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME
                    "Error! Failed to set ULL-to-ULL mode in CCLA ULL CTL");
        return status;
    }

    status = ccla_set_ull_ctl(
        ccla, CCLA_ULL_CTL_SEND_VD_INIT, ULL_CTL_SEND_VD_INIT_EN_VAL);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! Failed to send VD Init in CCLA ULL CTL");
        return status;
    }

    /* Poll for ULL status */
    status = shared_ctx->timer_api->wait(
        shared_ctx->timer_id,
        shared_ctx->config->cml_poll_timeout_us,
        is_ccla_ull_in_run_state,
        (void *)ccla);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! Polling CCLA ULL status failed");
        return status;
    }

    return status;
}

static int program_cml(const struct mod_cmn_cyprus_cml_config *cml_config)
{
    int status;
    struct cmn_cyprus_ccg_ha_reg *ccg_ha;
    struct ccg_ha_info *ccg_ha_info_table;
    struct cmn_cyprus_ccg_ra_reg *ccg_ra;
    struct ccg_ra_info *ccg_ra_info_table;
    struct cmn_cyprus_ccla_reg *ccla;
    struct ccla_info *ccla_info_table;

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

    /* Assign Home Agent ID (HAID) for the CCG HA node */
    ccg_ha_configure_haid(ccg_ha, cml_config->haid);

    /* Assign expanded RAID to local request agents */
    status = program_ccg_ra_ldid_to_raid_lut(ccg_ra);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Error! RAID programming failed");
        return status;
    }

    /*
     * Assign unique LDID for each remote caching agent that can send requests
     * to HNs (HN‑F, HN‑I, HN‑D, and HN‑P).
     */
    status = program_ccg_ha_raid_to_ldid_lut(ccg_ha);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME
                    "Error! CCG HA RAID-to-LDID LUT programming failed");
        return status;
    }

    /*
     * Configure CCG HA node id in place of remote caching agents in HN-S as
     * CCG HA is a proxy for all remote RN‑Fs. HN‑S use the LDIDs assigned
     * to the remote caching agents for Snoop Filter (SF) tracking.
     */
    status = program_hns_ldid_to_rn_nodeid(
        node_info_get_id(ccg_ha->CCG_HA_NODE_INFO), cml_config->remote_chip_id);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME
                    "Error! HN-S LDID-to-RN node id programming failed");
        return status;
    }

    /*
     * Enable Symmetric Multiprocessor (SMP) mode which allows for a shared,
     * common OS and memory to operate on multiple chips.
     */
    if (cml_config->enable_smp_mode == true) {
        status = enable_cml_smp_mode(ccg_ra, ccg_ha);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME "Error! SMP mode programming failed");
            return status;
        }
    }

    ccla_info_table = shared_ctx->ccla_info_table;
    ccla = ccla_info_table[cml_config->ccg_ldid].ccla;

    /*
     * Enable CCLA to CCLA Direct connect mode.
     *
     * This mode enables direct connection of the CXS interface from the
     * CCLA on one mesh to the CXS interface of the other.
     */
    if (cml_config->enable_direct_connect_mode == true) {
        status = enable_ccla_direct_connect_mode(ccla);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(MOD_NAME
                        "Error! Failed to enable CML direct connect mode");
            return status;
        }
    }

    return status;
}

static void setup_raid_table(void)
{
    uint16_t idx;
    uint16_t end_idx;
    uint16_t start_idx;
    uint8_t offset_pos;
    uint8_t chip_id;
    uint16_t raid_value;
    unsigned int offset_id;
    const uint8_t chip_count = shared_ctx->config_table->chip_count;

    /*
     * Calculate the number of bits required to represent chip id. The chip id
     * and the offset id are used to calculate the 6-bit base id of the expanded
     * RAID.
     */
    offset_pos = fwk_math_log2((unsigned int)chip_count);

    for (chip_id = 0; chip_id < chip_count; chip_id++) {
        start_idx = (chip_id * shared_ctx->max_rn_count);
        end_idx = start_idx + shared_ctx->max_rn_count;
        offset_id = start_idx;

        for (idx = start_idx; idx < end_idx; idx++) {
            /*
             * Use chip ID and offset ID for generating RAIDs. The 6-bit base ID
             * component of the RAID must be unique across chips as it is used
             * for identification and routing.
             *
             * Note: It is assumed that the mesh on the remote chip is
             * symmetrical having the same number of requesting agents. With
             * this assumption, the expanded RAID assignment will not overlap
             * with remote chip RAIDs ensuring the uniqueness.
             */
            raid_value = (chip_id | (offset_id << offset_pos));
            shared_ctx->raid_table[idx] = raid_value;

            offset_id++;
        }
    }
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

    /*
     * All the requesting agents are identified using a globally
     * unique 10-bit Requesting Agent ID (RAID). This ID is made up of a 6-bit
     * base ID and a 4-bit expanded ID.
     *
     * Calculate expanded RAID for all the requesting agents in the multichip
     * and store it in a table.
     */
    setup_raid_table();

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

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cmn700.h>
#include <cmn700_ccg.h>

#include <internal/cmn700_ctx.h>

#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_math.h>

#include <inttypes.h>
#include <stdint.h>

#define MOD_NAME "[CMN700_CCG] "

struct mod_cmn700_ccg_ctx {
    /* RAID value common to all function */
    uint8_t raid_value;

    /* Port Aggregation context flag */
    bool is_prog_for_port_agg;
};

static struct mod_cmn700_ccg_ctx cmn700_ccg_ctx;

/* Pointer to the current CCG configuration data */
static const struct mod_cmn700_ccg_config *config;

static unsigned int get_ldid(
    struct cmn700_device_ctx *ctx,
    bool program_for_port_aggregation)
{
    unsigned int ldid;

    ldid = (program_for_port_aggregation) ? config->port_aggregate_ldid :
                                            config->ldid;

    if (ldid >= ctx->ccg_node_count) {
        FWK_LOG_ERR(MOD_NAME "Unexpected ldid: %d", ldid);
        fwk_trap();
    }

    return ldid;
}

static bool ccg_link_wait_condition(void *data)
{
    bool ret;
    uint8_t linkid;
    uint64_t val1;
    uint64_t val2;
    unsigned int ccg_ldid;
    struct ccg_wait_condition_data *wait_data;
    struct cmn700_ccg_ra_reg *ccg_ra_reg;
    struct cmn700_ccg_ha_reg *ccg_ha_reg;
    struct cmn700_device_ctx *ctx;

    fwk_assert(data != NULL);

    wait_data = (struct ccg_wait_condition_data *)data;
    ctx = wait_data->ctx;

    ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
    ccg_ra_reg = ctx->ccg_ra_reg_table[ccg_ldid].ccg_ra_reg;
    ccg_ha_reg = ctx->ccg_ha_reg_table[ccg_ldid].ccg_ha_reg;
    linkid = wait_data->linkid;

    switch (wait_data->cond) {
    case CCG_LINK_CTRL_EN_BIT_SET:
        val1 = ccg_ra_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_CTRL;
        val2 = ccg_ha_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_CTRL;
        ret = ((val1 & CCG_LINK_CTRL_EN_MASK) > 0) &&
            ((val2 & CCG_LINK_CTRL_EN_MASK) > 0);
        break;

    case CCG_LINK_CTRL_UP_BIT_CLR:
        val1 = ccg_ra_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_CTRL;
        val2 = ccg_ha_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_CTRL;
        ret =
            (((val1 & CCG_LINK_CTRL_UP_MASK) == 0) &&
             ((val2 & CCG_LINK_CTRL_UP_MASK) == 0));
        break;

    case CCG_LINK_STATUS_DWN_BIT_SET:
        val1 = ccg_ra_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_STATUS;
        val2 = ccg_ha_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_STATUS;
        ret = ((val1 & CCG_LINK_STATUS_DOWN_MASK) > 0) &&
            ((val2 & CCG_LINK_STATUS_DOWN_MASK) > 0);
        break;

    case CCG_LINK_STATUS_DWN_BIT_CLR:
        val1 = ccg_ra_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_STATUS;
        val2 = ccg_ha_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_STATUS;
        ret =
            (((val1 & CCG_LINK_STATUS_DOWN_MASK) == 0) &&
             ((val2 & CCG_LINK_STATUS_DOWN_MASK) == 0));
        break;

    case CCG_LINK_STATUS_ACK_BIT_SET:
        val1 = ccg_ra_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_STATUS;
        val2 = ccg_ha_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_STATUS;
        ret = ((val1 & CCG_LINK_STATUS_ACK_MASK) > 0) &&
            ((val2 & CCG_LINK_STATUS_ACK_MASK) > 0);
        break;

    case CCG_LINK_STATUS_ACK_BIT_CLR:
        val1 = ccg_ra_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_STATUS;
        val2 = ccg_ha_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_STATUS;
        ret =
            (((val1 & CCG_LINK_STATUS_ACK_MASK) == 0) &&
             ((val2 & CCG_LINK_STATUS_ACK_MASK) == 0));
        break;

    case CCG_LINK_STATUS_HA_DVMDOMAIN_ACK_BIT_SET:
        val1 = ccg_ha_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_STATUS;
        ret = (((val1 & CCG_LINK_STATUS_DVMDOMAIN_ACK_MASK)) != 0);
        break;

    case CCG_LINK_STATUS_RA_DVMDOMAIN_ACK_BIT_SET:
        val1 = ccg_ra_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_STATUS;
        ret = (((val1 & CCG_LINK_STATUS_DVMDOMAIN_ACK_MASK)) != 0);
        break;

    default:
        fwk_unexpected();
        ret = false;
    }

    return ret;
}

static void program_ccg_ra_rnf_ldid_to_exp_raid_reg(
    struct cmn700_device_ctx *ctx,
    uint8_t ldid_value,
    uint8_t raid)
{
    uint32_t reg_offset = 0;
    uint32_t ldid_value_offset = 0;
    unsigned int ccg_ldid;
    struct cmn700_ccg_ra_reg *ccg_ra_reg;

    ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
    ccg_ra_reg = ctx->ccg_ra_reg_table[ccg_ldid].ccg_ra_reg;

    /* Each 64-bit RA RNF LDID-to-RAID register holds 4 LDIDs */
    reg_offset = ldid_value / 4;
    ldid_value_offset = ldid_value % 4;

    /* Adding raid_value into LDID-to-RAID register */
    ccg_ra_reg->CCG_RA_RNF_LDID_TO_EXP_RAID_REG[reg_offset] |=
        ((uint64_t)raid << (ldid_value_offset * NUM_BITS_RESERVED_FOR_RAID));

    /* Set corresponding valid bit */
    ccg_ra_reg->CCG_RA_RNF_LDID_TO_EXP_RAID_REG[reg_offset] |=
        (LDID_TO_EXP_RAID_VALID_MASK)
        << (ldid_value_offset * NUM_BITS_RESERVED_FOR_RAID);

    cmn700_ccg_ctx.raid_value++;
}

static void program_ccg_ra_rni_ldid_to_exp_raid_reg(
    struct cmn700_device_ctx *ctx,
    uint8_t ldid_value,
    uint8_t raid)
{
    uint32_t reg_offset = 0;
    uint32_t ldid_value_offset = 0;
    unsigned int ccg_ldid;
    struct cmn700_ccg_ra_reg *ccg_ra_reg;

    ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
    ccg_ra_reg = ctx->ccg_ra_reg_table[ccg_ldid].ccg_ra_reg;

    /* Each 64-bit RA RNI LDID-to-RAID register holds 4 LDIDs */
    reg_offset = ldid_value / 4;
    ldid_value_offset = ldid_value % 4;

    /* Adding raid_value into LDID-to-RAID register */
    ccg_ra_reg->CCG_RA_RNI_LDID_TO_EXP_RAID_REG[reg_offset] |=
        ((uint64_t)raid << (ldid_value_offset * NUM_BITS_RESERVED_FOR_RAID));

    /* Set corresponding valid bit */
    ccg_ra_reg->CCG_RA_RNI_LDID_TO_EXP_RAID_REG[reg_offset] |=
        (LDID_TO_EXP_RAID_VALID_MASK)
        << (ldid_value_offset * NUM_BITS_RESERVED_FOR_RAID);

    cmn700_ccg_ctx.raid_value++;
}

static void program_ccg_ra_rnd_ldid_to_exp_raid_reg(
    struct cmn700_device_ctx *ctx,
    uint8_t ldid_value,
    uint8_t raid)
{
    uint32_t reg_offset = 0;
    uint32_t ldid_value_offset = 0;
    unsigned int ccg_ldid;
    struct cmn700_ccg_ra_reg *ccg_ra_reg;

    ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
    ccg_ra_reg = ctx->ccg_ra_reg_table[ccg_ldid].ccg_ra_reg;

    /* Each 64-bit RA RND LDID-to-RAID register holds 4 LDIDs */
    reg_offset = ldid_value / 4;
    ldid_value_offset = ldid_value % 4;

    /* Adding raid_value into LDID-to-RAID register */
    ccg_ra_reg->CCG_RA_RND_LDID_TO_EXP_RAID_REG[reg_offset] |=
        ((uint64_t)raid << (ldid_value_offset * NUM_BITS_RESERVED_FOR_RAID));

    /* Set corresponding valid bit */
    ccg_ra_reg->CCG_RA_RND_LDID_TO_EXP_RAID_REG[reg_offset] |=
        (LDID_TO_EXP_RAID_VALID_MASK)
        << (ldid_value_offset * NUM_BITS_RESERVED_FOR_RAID);

    cmn700_ccg_ctx.raid_value++;
}

static void program_agentid_to_linkid_reg(
    struct cmn700_device_ctx *ctx,
    const struct mod_cmn700_ccg_config *config)
{
    uint8_t linkid = 0;
    uint32_t agentid;
    uint32_t reg_index = 0;
    uint32_t agentid_bit_offset = 0;
    unsigned int ccg_ldid;
    unsigned int remote_agentid_start;
    unsigned int remote_agentid_end;
    struct cmn700_ccg_ra_reg *ccg_ra_reg;
    struct cmn700_ccg_ha_reg *ccg_ha_reg;

    ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
    ccg_ra_reg = ctx->ccg_ra_reg_table[ccg_ldid].ccg_ra_reg;
    ccg_ha_reg = ctx->ccg_ha_reg_table[ccg_ldid].ccg_ha_reg;

    for (linkid = 0; linkid < CMN700_MAX_CCG_PROTOCOL_LINKS; linkid++) {
        remote_agentid_start =
            config->remote_agentid_to_linkid_map[linkid].remote_agentid_start;
        remote_agentid_end =
            config->remote_agentid_to_linkid_map[linkid].remote_agentid_end;

        /*
         * Skip configuring link if both start and end are 0, indicating
         * there's no link
         */
        if ((remote_agentid_start == 0) && (remote_agentid_end == 0)) {
            continue;
        }

        FWK_LOG_INFO(
            MOD_NAME "  Remote [AgentID %d - AgentID %d] Link %d",
            remote_agentid_start,
            remote_agentid_end,
            linkid);

        for (agentid = remote_agentid_start; agentid <= remote_agentid_end;
             agentid++) {
            /* Each register is 64 bits and holds 8 AgentID/LinkID mappings */
            reg_index = agentid / 8;
            agentid_bit_offset = agentid % 8;

            /* Writing AgentID-to-LinkID mappings */
            ccg_ra_reg->CCG_RA_AGENTID_TO_LINKID_REG[reg_index] |=
                ((uint64_t)linkid
                 << (agentid_bit_offset * NUM_BITS_RESERVED_FOR_LINKID));

            ccg_ha_reg->CCG_HA_AGENTID_TO_LINKID_REG[reg_index] |=
                ((uint64_t)linkid
                 << (agentid_bit_offset * NUM_BITS_RESERVED_FOR_LINKID));

            /* Setting corresponding valid bits */
            ccg_ra_reg->CCG_RA_AGENTID_TO_LINKID_VAL |=
                (UINT64_C(0x1) << agentid);
            ccg_ha_reg->CCG_HA_AGENTID_TO_LINKID_VAL |=
                (UINT64_C(0x1) << agentid);
        }
    }
}

static void program_ccg_ha_id(struct cmn700_device_ctx *ctx)
{
    struct cmn700_ccg_ha_reg *ccg_ha_reg;
    unsigned int ccg_haid;
    unsigned int ccg_ldid;

    ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
    ccg_ha_reg = ctx->ccg_ha_reg_table[ccg_ldid].ccg_ha_reg;
    ccg_haid = cmn700_ccg_ctx.is_prog_for_port_agg ?
        config->port_aggregate_haid :
        config->haid;
    ccg_ha_reg->CCG_HA_ID = ccg_haid;

    FWK_LOG_INFO(
        MOD_NAME "HAID for CCG %d with nodeid %d: HAID %d",
        ccg_ldid,
        get_node_id(ccg_ha_reg),
        ccg_haid);
}

static void program_ccg_ha_raid_to_ldid_lut(
    struct cmn700_device_ctx *ctx,
    uint8_t raid_id,
    uint8_t ldid_value)
{
    uint32_t reg_index = 0;
    uint32_t raid_bit_offset = 0;
    unsigned int ccg_ldid;
    struct cmn700_ccg_ha_reg *ccg_ha_reg;

    ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
    ccg_ha_reg = ctx->ccg_ha_reg_table[ccg_ldid].ccg_ha_reg;

    /* Each 64-bit RAID-to-LDID register holds 4 mappings, 16 bits each. */
    reg_index = raid_id / 4;
    raid_bit_offset = raid_id % 4;

    /* Write RAID-to-LDID mapping (with RNF bit set) */
    ccg_ha_reg->CCG_HA_RNF_EXP_RAID_TO_LDID_REG[reg_index] |=
        ((uint64_t)(
             ldid_value | CCG_HA_RAID_TO_LDID_RNF_MASK |
             (EXP_RAID_TO_LDID_VALID_MASK))
         << (raid_bit_offset * NUM_BITS_RESERVED_FOR_LDID));
}

static void program_hnf_ldid_to_chi_node_id_reg(
    struct cmn700_device_ctx *ctx,
    const struct mod_cmn700_ccg_config *config)
{
    uint8_t linkid;
    uint32_t agentid;
    uint32_t reg_index;
    uint32_t i;
    uint32_t nodeid_ra;
    unsigned int remote_agentid_start;
    unsigned int remote_agentid_end;
    struct cmn700_hnf_reg *hnf_reg;

    linkid = 0;
    reg_index = 0;
    i = 0;

    /* Assign the NodeID of CCHA as the RA's NodeID */
    nodeid_ra = ctx->ccg_ha_reg_table[config->ldid].node_id;

    if (cmn700_ccg_ctx.is_prog_for_port_agg) {
        /*
         * if programming for port aggregation support, reset the remote rnf
         * ldid value to the previous iteration in order to program.
         */
        ctx->remote_rnf_ldid_value -= ctx->rnf_count;
    }

    for (linkid = 0; linkid < CMN700_MAX_CCG_PROTOCOL_LINKS; linkid++) {
        remote_agentid_start =
            config->remote_agentid_to_linkid_map[linkid].remote_agentid_start;
        remote_agentid_end =
            config->remote_agentid_to_linkid_map[linkid].remote_agentid_end;

        /*
         * Skip configuring link if both start and end are 0, indicating
         * there's no link
         */
        if ((remote_agentid_start == 0) && (remote_agentid_end == 0)) {
            continue;
        }

        FWK_LOG_INFO(
            MOD_NAME "  Remote [AgentID %d - AgentID %d] Node Id: %" PRIu32,
            remote_agentid_start,
            remote_agentid_end,
            nodeid_ra);

        for (agentid = remote_agentid_start; agentid <= remote_agentid_end;
             agentid++) {
            reg_index = ctx->remote_rnf_ldid_value;

            for (i = 0; i < ctx->hnf_count; i++) {
                hnf_reg = (struct cmn700_hnf_reg *)ctx->hnf_node[i];
                /* Write CCHA NodeID, local/remote and valid bit */
                hnf_reg->HNF_RN_CLUSTER_PHYSID[reg_index][0] |= ((uint64_t)(
                    nodeid_ra |
                    (REMOTE_CCG_NODE << HNF_RN_PHYS_RN_LOCAL_REMOTE_SHIFT_VAL) |
                    (UINT64_C(0x1) << HNF_RN_PHYS_RN_ID_VALID_SHIFT_VAL)));
            }

            ctx->remote_rnf_ldid_value++;
        }
    }
}

static void enable_smp_mode(
    struct cmn700_device_ctx *ctx,
    const struct mod_cmn700_ccg_config *config)
{
    unsigned int ccg_ldid;
    struct cmn700_ccg_ra_reg *ccg_ra_reg;
    struct cmn700_ccg_ha_reg *ccg_ha_reg;

    ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
    ccg_ra_reg = ctx->ccg_ra_reg_table[ccg_ldid].ccg_ra_reg;
    ccg_ha_reg = ctx->ccg_ha_reg_table[ccg_ldid].ccg_ha_reg;

    ccg_ra_reg->LINK_REGS[0].CCG_CCPRTCL_LINK_CTRL |=
        (1 << CCG_RA_CCPRTCL_LINK_CTRL_SMP_MODE_EN_SHIFT_VAL);
    ccg_ha_reg->LINK_REGS[0].CCG_CCPRTCL_LINK_CTRL |=
        (1 << CCG_HA_CCPRTCL_LINK_CTRL_SMP_MODE_EN_SHIFT_VAL);

    FWK_LOG_INFO(MOD_NAME "SMP Mode Enabled");
}

/*
 * Helper function to check the status of the Upper link layer direct connect
 * (ull to ull) mode
 */
static bool ccla_ull_status_wait_condition(void *data)
{
    bool ccla_ull_status_rx_in_run;
    bool ccla_ull_status_tx_in_run;
    unsigned int ccg_ldid;
    struct cmn700_ccla_reg *ccla_reg;
    struct cmn700_device_ctx *ctx;

    ctx = (struct cmn700_device_ctx *)data;
    ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
    ccla_reg = ctx->ccla_reg_table[ccg_ldid].ccla_reg;

    ccla_ull_status_rx_in_run =
        ((ccla_reg->CCLA_ULL_STATUS & CCLA_ULL_STATUS_SEND_RX_ULL_STATE_MASK) >
         0);

    ccla_ull_status_tx_in_run =
        ((ccla_reg->CCLA_ULL_STATUS & CCLA_ULL_STATUS_SEND_TX_ULL_STATE_MASK) >
         0);

    return (ccla_ull_status_rx_in_run && ccla_ull_status_tx_in_run);
}

static int enable_ull_to_ull_mode(
    struct cmn700_device_ctx *ctx,
    const struct mod_cmn700_ccg_config *config)
{
    int status;
    unsigned int ccg_ldid;
    struct cmn700_ccla_reg *ccla_reg;

    ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
    ccla_reg = ctx->ccla_reg_table[ccg_ldid].ccla_reg;

    FWK_LOG_INFO(MOD_NAME "Enabling CCG ULL to ULL mode...");

    /* Enabling ULL-to-ULL mode */
    ccla_reg->CCLA_ULL_CTL = (1 << CCLA_ULL_CTL_ULL_TO_ULL_MODE_EN_SHIFT_VAL);

    /* Setting send_vd_init */
    ccla_reg->CCLA_ULL_CTL |= (1 << CCLA_ULL_CTL_SEND_VD_INIT_SHIFT_VAL);

    /* Wait until link enable bits are set */
    status = ctx->timer_api->wait(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        CCLA_ULL_STATUS_TIMEOUT,
        ccla_ull_status_wait_condition,
        ctx);

    if (status != FWK_SUCCESS) {
        if ((ccla_reg->CCLA_ULL_STATUS &
             CCLA_ULL_STATUS_SEND_RX_ULL_STATE_MASK) == 0) {
            FWK_LOG_ERR(MOD_NAME "Rx ULL is in Stop state");
        }

        if ((ccla_reg->CCLA_ULL_STATUS &
             CCLA_ULL_STATUS_SEND_TX_ULL_STATE_MASK) == 0) {
            FWK_LOG_ERR(MOD_NAME "Tx ULL is in Stop state");
        }

        FWK_LOG_ERR(MOD_NAME "Enabling CCG ULL to ULL mode... Failed");
        return status;
    }

    FWK_LOG_INFO(MOD_NAME "Enabling CCG ULL to ULL mode... Done");
    return status;
}

static void program_ccg_ra_sam_addr_region(
    struct cmn700_device_ctx *ctx,
    const struct mod_cmn700_ccg_config *config)
{
    uint64_t blocks;
    uint64_t size;
    unsigned int ccg_ldid;
    unsigned int i;
    unsigned int remote_haid;
    struct cmn700_ccg_ra_reg *ccg_ra_reg;

    ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
    ccg_ra_reg = ctx->ccg_ra_reg_table[ccg_ldid].ccg_ra_reg;

    FWK_LOG_INFO(
        MOD_NAME "Configuring RA SAM for CCRA NodeID %d",
        get_node_id(ccg_ra_reg));

    for (i = 0; i < CMN700_MAX_RA_SAM_ADDR_REGION; i++) {
        /* If the size is zero, skip that entry */
        if (config->ra_mmap_table[i].size == 0) {
            continue;
        }

        /* Size must be a multiple of SAM_GRANULARITY */
        fwk_assert((config->ra_mmap_table[i].size % (64 * FWK_KIB)) == 0);

        /* Size also must be a power of two */
        fwk_assert(
            (config->ra_mmap_table[i].size &
             (config->ra_mmap_table[i].size - 1)) == 0);

        /* Region base should be naturally aligned to the region size */
        fwk_assert(
            config->ra_mmap_table[i].base % config->ra_mmap_table[i].size == 0);

        if (cmn700_ccg_ctx.is_prog_for_port_agg) {
            remote_haid = config->port_aggregate_remote_haid[i];
        } else {
            remote_haid = config->ra_mmap_table[i].remote_haid;
        }

        FWK_LOG_INFO(
            MOD_NAME "  [0x%llx - 0x%llx] HAID %d",
            config->ra_mmap_table[i].base,
            config->ra_mmap_table[i].base + config->ra_mmap_table[i].size - 1,
            remote_haid);

        blocks = config->ra_mmap_table[i].size / (64 * FWK_KIB);
        size = fwk_math_log2(blocks);

        ccg_ra_reg->CCG_RA_SAM_ADDR_REGION_REG[i] = size |
            (config->ra_mmap_table[i].base) |
            ((uint64_t)remote_haid << SAM_ADDR_TARGET_HAID_SHIFT) |
            (SAM_ADDR_REG_VALID_MASK);
    }
}

static int enable_and_start_ccg_link_up_sequence(
    struct cmn700_device_ctx *ctx,
    uint8_t linkid)
{
    int status;
    unsigned int ccg_ldid;
    struct ccg_wait_condition_data wait_data;
    struct cmn700_ccg_ra_reg *ccg_ra_reg;
    struct cmn700_ccg_ha_reg *ccg_ha_reg;

    ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
    ccg_ra_reg = ctx->ccg_ra_reg_table[ccg_ldid].ccg_ra_reg;
    ccg_ha_reg = ctx->ccg_ha_reg_table[ccg_ldid].ccg_ha_reg;

    if (linkid > 2) {
        return FWK_E_PARAM;
    }

    wait_data.ctx = ctx;
    wait_data.linkid = linkid;

    FWK_LOG_INFO(MOD_NAME "Enabling CCG link %d...", linkid);
    /* Set link enable bit to enable the CCG link */
    ccg_ra_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_CTRL |=
        CCG_LINK_CTRL_EN_MASK;
    ccg_ha_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_CTRL |=
        CCG_LINK_CTRL_EN_MASK;

    /* Wait until link enable bits are set */
    wait_data.cond = CCG_LINK_CTRL_EN_BIT_SET;
    status = ctx->timer_api->wait(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        CCG_CCPRTCL_LINK_CTRL_TIMEOUT,
        ccg_link_wait_condition,
        &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Enabling CCG link %d... Failed", linkid);
        return status;
    }
    FWK_LOG_INFO(MOD_NAME "Enabling CCG link %d... Done", linkid);

    FWK_LOG_INFO(MOD_NAME "Verifying link down status...");
    /* Wait till link up bits are cleared in control register */
    wait_data.cond = CCG_LINK_CTRL_UP_BIT_CLR;
    status = ctx->timer_api->wait(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        CCG_CCPRTCL_LINK_CTRL_TIMEOUT,
        ccg_link_wait_condition,
        &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Verifying link down status... Failed");
        return status;
    }

    /* Wait till link down bits are set in status register */
    wait_data.cond = CCG_LINK_STATUS_DWN_BIT_SET;
    status = ctx->timer_api->wait(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        CCG_CCPRTCL_LINK_CTRL_TIMEOUT,
        ccg_link_wait_condition,
        &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Verifying link down status... Failed");
        return status;
    }

    /* Wait till link ACK bits are cleared in status register */
    wait_data.cond = CCG_LINK_STATUS_ACK_BIT_CLR;
    status = ctx->timer_api->wait(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        CCG_CCPRTCL_LINK_CTRL_TIMEOUT,
        ccg_link_wait_condition,
        &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Verifying link down status... Failed");
        return status;
    }
    FWK_LOG_INFO(MOD_NAME "Verifying link down status... Done");

    FWK_LOG_INFO(MOD_NAME "Bringing up link...");
    /* Bring up link using link request bit */
    ccg_ra_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_CTRL |=
        CCG_LINK_CTRL_REQ_MASK;
    ccg_ha_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_CTRL |=
        CCG_LINK_CTRL_REQ_MASK;

    /* Wait till link ACK bits are set in status register */
    wait_data.cond = CCG_LINK_STATUS_ACK_BIT_SET;
    status = ctx->timer_api->wait(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        CCG_CCPRTCL_LINK_CTRL_TIMEOUT,
        ccg_link_wait_condition,
        &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Bringing up link... Failed");
        return status;
    }

    /* Wait till link down bits are cleared in status register */
    wait_data.cond = CCG_LINK_STATUS_DWN_BIT_CLR;
    status = ctx->timer_api->wait(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        CCG_CCPRTCL_LINK_CTRL_TIMEOUT,
        ccg_link_wait_condition,
        &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(MOD_NAME "Bringing up link... Failed");
        return status;
    }
    FWK_LOG_INFO(MOD_NAME "Bringing up link... Done");

    return FWK_SUCCESS;
}

int ccg_setup(
    const unsigned int chip_id,
    struct cmn700_device_ctx *ctx,
    const struct mod_cmn700_ccg_config *ccg_config)
{
    int status;
    uint8_t rnf_ldid;
    uint8_t rni_ldid;
    uint8_t rnd_ldid;
    uint8_t agentid;
    uint8_t remote_agentid;
    uint8_t offset_id;
    uint8_t local_ra_cnt;
    unsigned int i;
    unsigned int unique_remote_rnf_ldid_value;

    FWK_LOG_INFO(MOD_NAME "Programming CCG gateway...");

    /* Assign the max count among the RNs as local_ra_cnt */
    if ((ctx->rnf_count > ctx->rnd_count) &&
        (ctx->rnf_count > ctx->rni_count)) {
        local_ra_cnt = ctx->rnf_count;
    } else if (ctx->rnd_count > ctx->rni_count) {
        local_ra_cnt = ctx->rnd_count;
    } else {
        local_ra_cnt = ctx->rni_count;
    }

    cmn700_ccg_ctx.is_prog_for_port_agg = false;

    do {
        /* Set the global config data */
        config = ccg_config;

        if (ccg_config->smp_mode) {
            enable_smp_mode(ctx, ccg_config);
        }

        /*
         * In order to assign unique AgentIDs across multiple chips, chip_id is
         * used as factor to offset the AgentID value
         */
        cmn700_ccg_ctx.raid_value = 0;
        offset_id = chip_id * local_ra_cnt;

        for (rnf_ldid = 0; rnf_ldid < ctx->rnf_count; rnf_ldid++) {
            agentid = cmn700_ccg_ctx.raid_value + offset_id;

            /* Program RAID values in CCRA LDID to RAID LUT */
            program_ccg_ra_rnf_ldid_to_exp_raid_reg(ctx, rnf_ldid, agentid);
        }

        /* Program agentid to linkid LUT for remote agents in CCRA/CCHA/CCLA */
        program_agentid_to_linkid_reg(ctx, ccg_config);

        /* Program HN-F ldid to CHI NodeID for remote RN-F agents */
        program_hnf_ldid_to_chi_node_id_reg(ctx, ccg_config);

        /*
         * unique_remote_rnf_ldid_value is used to keep track of the
         * ldid of the remote RNF agents
         */
        unique_remote_rnf_ldid_value = ctx->rnf_count;

        if (ccg_config->remote_rnf_count && (ctx->rnf_count == 0)) {
            FWK_LOG_ERR(
                MOD_NAME
                "Remote RN-F Count can't be %u when RN-F count is zero",
                ccg_config->remote_rnf_count);
            fwk_unexpected();
        }

        for (i = 0; i < ccg_config->remote_rnf_count; i++) {
            /*
             * The remote_agentid (RAID) should not include the current chip's
             * AgentIDs. If `block` is less than the current chip_id, then
             * include the AgentIDs starting from chip 0 till (not including)
             * current chip. If the `block` is equal or greater than the current
             * chip, then include the AgentIDs from next chip till the max chip.
             */
            if ((i / ctx->rnf_count) < chip_id) {
                remote_agentid = (i % ctx->rnf_count) +
                    (local_ra_cnt * (i / ctx->rnf_count));
            } else {
                remote_agentid = (i % ctx->rnf_count) + local_ra_cnt +
                    (local_ra_cnt * (i / ctx->rnf_count));
            }

            /* Program the CCHA raid to ldid LUT */
            program_ccg_ha_raid_to_ldid_lut(
                ctx, remote_agentid, unique_remote_rnf_ldid_value);

            unique_remote_rnf_ldid_value++;
        }

        /* Program the unique HAID for the CCHA block */
        program_ccg_ha_id(ctx);

        cmn700_ccg_ctx.raid_value = 0;
        offset_id = chip_id * local_ra_cnt;

        for (rnd_ldid = 0; rnd_ldid < ctx->rnd_count; rnd_ldid++) {
            /* Determine agentid of the remote agents */
            agentid = cmn700_ccg_ctx.raid_value + offset_id;

            /* Program RAID values in CCRA LDID to RAID LUT */
            program_ccg_ra_rnd_ldid_to_exp_raid_reg(ctx, rnd_ldid, agentid);
        }

        cmn700_ccg_ctx.raid_value = 0;
        offset_id = chip_id * local_ra_cnt;

        for (rni_ldid = 0; rni_ldid < ctx->rni_count; rni_ldid++) {
            /* Determine agentid of the remote agents */
            agentid = cmn700_ccg_ctx.raid_value + offset_id;

            /* Program RAID values in CCRA LDID to RAID LUT */
            program_ccg_ra_rni_ldid_to_exp_raid_reg(ctx, rni_ldid, agentid);
        }

        /*
         * Program the CCRA SAM with the address range and the corresponding
         * remote HAID
         */
        program_ccg_ra_sam_addr_region(ctx, ccg_config);

        if (ccg_config->ull_to_ull_mode) {
            status = enable_ull_to_ull_mode(ctx, ccg_config);
            if (status != FWK_SUCCESS) {
                return status;
            }
        }
        /* Program the Link Control registers present in CCRA/CCHA/CCLA */
        status = enable_and_start_ccg_link_up_sequence(ctx, 0);

        if (config->port_aggregate && !cmn700_ccg_ctx.is_prog_for_port_agg) {
            cmn700_ccg_ctx.is_prog_for_port_agg = true;
            FWK_LOG_INFO(MOD_NAME
                         "Programming CCG gateway for Port Aggregation...");
        } else {
            cmn700_ccg_ctx.is_prog_for_port_agg = false;
        }
    } while (config->port_aggregate && cmn700_ccg_ctx.is_prog_for_port_agg);

    return status;
}

int ccg_exchange_protocol_credit(
    struct cmn700_device_ctx *ctx,
    const struct mod_cmn700_ccg_config *ccg_config)
{
    int linkid;
    unsigned int ccg_ldid;
    struct cmn700_ccg_ra_reg *ccg_ra_reg;
    struct cmn700_ccg_ha_reg *ccg_ha_reg;

    cmn700_ccg_ctx.is_prog_for_port_agg = false;

    do {
        config = ccg_config;
        ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
        ccg_ra_reg = ctx->ccg_ra_reg_table[ccg_ldid].ccg_ra_reg;
        ccg_ha_reg = ctx->ccg_ha_reg_table[ccg_ldid].ccg_ha_reg;

        /* Current support enables Link 0 only */
        linkid = 0;

        FWK_LOG_INFO(
            MOD_NAME "Exchanging protocol credits for link %d...", linkid);
        /* Exchange protocol credits using link up bit */
        ccg_ra_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_CTRL |=
            CCG_LINK_CTRL_UP_MASK;
        ccg_ha_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_CTRL |=
            CCG_LINK_CTRL_UP_MASK;
        FWK_LOG_INFO(
            MOD_NAME "Exchanging protocol credits for link %d... Done", linkid);

        if (config->port_aggregate && !cmn700_ccg_ctx.is_prog_for_port_agg) {
            cmn700_ccg_ctx.is_prog_for_port_agg = true;
            FWK_LOG_INFO(MOD_NAME
                         "Exchange Protocol Credit for Port Aggregation...");
        } else {
            cmn700_ccg_ctx.is_prog_for_port_agg = false;
        }
    } while (config->port_aggregate && cmn700_ccg_ctx.is_prog_for_port_agg);

    return FWK_SUCCESS;
}

int ccg_enter_system_coherency(
    struct cmn700_device_ctx *ctx,
    const struct mod_cmn700_ccg_config *ccg_config)
{
    int status;
    int linkid;
    unsigned int ccg_ldid;
    struct ccg_wait_condition_data wait_data;
    struct cmn700_ccg_ha_reg *ccg_ha_reg;

    cmn700_ccg_ctx.is_prog_for_port_agg = false;

    do {
        config = ccg_config;
        ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
        ccg_ha_reg = ctx->ccg_ha_reg_table[ccg_ldid].ccg_ha_reg;

        /* Current support enables Link 0 only */
        linkid = 0;

        wait_data.ctx = ctx;
        wait_data.linkid = linkid;

        FWK_LOG_INFO(
            MOD_NAME "Entering system coherency for link %d...", linkid);
        /* Enter system coherency by setting DVMDOMAIN request bit */
        ccg_ha_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_CTRL |=
            CCG_LINK_CTRL_DVMDOMAIN_REQ_MASK;

        /* Wait till DVMDOMAIN ACK bit is set in status register */
        wait_data.cond = CCG_LINK_STATUS_HA_DVMDOMAIN_ACK_BIT_SET;
        status = ctx->timer_api->wait(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            CCG_CCPRTCL_LINK_DVMDOMAIN_TIMEOUT,
            ccg_link_wait_condition,
            &wait_data);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                MOD_NAME "Entering system coherency for link %d... Failed",
                linkid);
            return status;
        }
        FWK_LOG_INFO(
            MOD_NAME "Entering system coherency for link %d... Done", linkid);

        if (config->port_aggregate && !cmn700_ccg_ctx.is_prog_for_port_agg) {
            cmn700_ccg_ctx.is_prog_for_port_agg = true;
            FWK_LOG_INFO(MOD_NAME
                         "Enter system coherency for Port Aggregation...");
        } else {
            cmn700_ccg_ctx.is_prog_for_port_agg = false;
        }
    } while (config->port_aggregate && cmn700_ccg_ctx.is_prog_for_port_agg);

    return FWK_SUCCESS;
}

int ccg_enter_dvm_domain(
    struct cmn700_device_ctx *ctx,
    const struct mod_cmn700_ccg_config *ccg_config)
{
    int status;
    int linkid;
    unsigned int ccg_ldid;
    struct ccg_wait_condition_data wait_data;
    struct cmn700_ccg_ra_reg *ccg_ra_reg;

    cmn700_ccg_ctx.is_prog_for_port_agg = false;

    do {
        config = ccg_config;
        ccg_ldid = get_ldid(ctx, cmn700_ccg_ctx.is_prog_for_port_agg);
        ccg_ra_reg = ctx->ccg_ra_reg_table[ccg_ldid].ccg_ra_reg;

        /* Current support enables Link 0 only */
        linkid = 0;

        wait_data.ctx = ctx;
        wait_data.linkid = linkid;

        FWK_LOG_INFO(MOD_NAME "Entering DVM domain for link %d...", linkid);
        /* DVM domain entry by setting DVMDOMAIN request bit */
        ccg_ra_reg->LINK_REGS[linkid].CCG_CCPRTCL_LINK_CTRL |=
            CCG_LINK_CTRL_DVMDOMAIN_REQ_MASK;

        /* Wait till DVMDOMAIN ACK bit is set in status register */
        wait_data.cond = CCG_LINK_STATUS_RA_DVMDOMAIN_ACK_BIT_SET;
        status = ctx->timer_api->wait(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            CCG_CCPRTCL_LINK_DVMDOMAIN_TIMEOUT,
            ccg_link_wait_condition,
            &wait_data);
        if (status != FWK_SUCCESS) {
            FWK_LOG_ERR(
                MOD_NAME "Entering DVM domain for link %d... Failed", linkid);
            return status;
        }
        FWK_LOG_INFO(
            MOD_NAME "Entering DVM domain for link %d... Done", linkid);

        if (config->port_aggregate && !cmn700_ccg_ctx.is_prog_for_port_agg) {
            cmn700_ccg_ctx.is_prog_for_port_agg = true;
            FWK_LOG_INFO(MOD_NAME
                         "Entering DVM domain for Port Aggregation...");
        } else {
            cmn700_ccg_ctx.is_prog_for_port_agg = false;
        }
    } while (config->port_aggregate && cmn700_ccg_ctx.is_prog_for_port_agg);

    return FWK_SUCCESS;
}

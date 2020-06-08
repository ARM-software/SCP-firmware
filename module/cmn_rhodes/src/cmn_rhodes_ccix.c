/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cmn_rhodes.h>
#include <cmn_rhodes_ccix.h>

#include <internal/cmn_rhodes_ctx.h>

#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_math.h>
#include <fwk_log.h>

#define MOD_NAME "[CMN_RHODES_CCIX] "

/* RAID value common to all function */
static uint8_t raid_value;

/* Pointer to the current CCIX configuration data */
static const struct mod_cmn_rhodes_ccix_config *config;

static bool cxg_link_wait_condition(void *data)
{
    uint64_t val1;
    uint64_t val2;
    uint8_t linkid;
    struct cmn_rhodes_cxg_ra_reg *cxg_ra_reg;
    struct cmn_rhodes_cxg_ha_reg *cxg_ha_reg;

    fwk_assert(data != NULL);

    struct cxg_wait_condition_data *wait_data =
        (struct cxg_wait_condition_data *)data;
    struct cmn_rhodes_device_ctx *ctx = wait_data->ctx;
    linkid = wait_data->linkid;

    cxg_ra_reg = ctx->cxg_ra_reg_table[config->ldid].cxg_ra_reg;
    cxg_ha_reg = ctx->cxg_ha_reg_table[config->ldid].cxg_ha_reg;

    switch (wait_data->cond) {
    case CXG_LINK_CTRL_EN_BIT_SET:
        val1 = cxg_ra_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_CTRL;
        val2 = cxg_ha_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_CTRL;
        return (((val1 & CXG_LINK_CTRL_EN_MASK) &&
                  (val2 & CXG_LINK_CTRL_EN_MASK)) != 0);

    case CXG_LINK_CTRL_UP_BIT_CLR:
        val1 = cxg_ra_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_CTRL;
        val2 = cxg_ha_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_CTRL;
        return ((((val1 & CXG_LINK_CTRL_UP_MASK) == 0) &&
                  ((val2 & CXG_LINK_CTRL_UP_MASK) == 0)) != 0);

    case CXG_LINK_STATUS_DWN_BIT_SET:
        val1 = cxg_ra_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_STATUS;
        val2 = cxg_ha_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_STATUS;
        return (((val1 & CXG_LINK_STATUS_DOWN_MASK) &&
                  (val2 & CXG_LINK_STATUS_DOWN_MASK)) != 0);

    case CXG_LINK_STATUS_DWN_BIT_CLR:
        val1 = cxg_ra_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_STATUS;
        val2 = cxg_ha_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_STATUS;
        return ((((val1 & CXG_LINK_STATUS_DOWN_MASK) == 0) &&
                  ((val2 & CXG_LINK_STATUS_DOWN_MASK) == 0)) != 0);

    case CXG_LINK_STATUS_ACK_BIT_SET:
        val1 = cxg_ra_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_STATUS;
        val2 = cxg_ha_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_STATUS;
        return (((val1 & CXG_LINK_STATUS_ACK_MASK) &&
                (val2 & CXG_LINK_STATUS_ACK_MASK)) != 0);

    case CXG_LINK_STATUS_ACK_BIT_CLR:
        val1 = cxg_ra_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_STATUS;
        val2 = cxg_ha_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_STATUS;
        return ((((val1 & CXG_LINK_STATUS_ACK_MASK) == 0) &&
                  ((val2 & CXG_LINK_STATUS_ACK_MASK) == 0)) != 0);

    case CXG_LINK_STATUS_HA_DVMDOMAIN_ACK_BIT_SET:
        val1 = cxg_ha_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_STATUS;
        return (((val1 & CXG_LINK_STATUS_DVMDOMAIN_ACK_MASK)) != 0);

    case CXG_LINK_STATUS_RA_DVMDOMAIN_ACK_BIT_SET:
        val1 = cxg_ra_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_STATUS;
        return (((val1 & CXG_LINK_STATUS_DVMDOMAIN_ACK_MASK)) != 0);

    default:
        fwk_unexpected();
        return false;
    }
}

static void program_cxg_ra_rnf_ldid_to_raid_reg(
    struct cmn_rhodes_device_ctx *ctx, uint8_t ldid_value, uint8_t raid)
{
    uint32_t reg_offset = 0;
    uint32_t ldid_value_offset = 0;
    struct cmn_rhodes_cxg_ra_reg *cxg_ra_reg;

    cxg_ra_reg = ctx->cxg_ra_reg_table[config->ldid].cxg_ra_reg;

    /* Each 64-bit RA RNF LDID-to-RAID register holds 8 LDIDs */
    reg_offset = ldid_value / 8;
    ldid_value_offset = ldid_value % 8;

    /* Adding raid_value into LDID-to-RAID register */
    cxg_ra_reg->CXG_RA_RNF_LDID_TO_RAID_REG[reg_offset] |=
        ((uint64_t)raid << (ldid_value_offset * NUM_BITS_RESERVED_FOR_RAID));

    /* Set corresponding valid bit */
    cxg_ra_reg->CXG_RA_RNF_LDID_TO_RAID_VAL |= ((uint64_t)0x1 << ldid_value);

    /* Increment the global raid_value */
    raid_value++;
}

static void program_cxg_ra_rni_ldid_to_raid_reg(
    struct cmn_rhodes_device_ctx * ctx, uint8_t ldid_value, uint8_t raid)
{
    uint32_t reg_offset = 0;
    uint32_t ldid_value_offset = 0;
    struct cmn_rhodes_cxg_ra_reg *cxg_ra_reg;

    cxg_ra_reg = ctx->cxg_ra_reg_table[config->ldid].cxg_ra_reg;

    /* Each 64-bit RA RNI LDID-to-RAID register holds 8 LDIDs */
    reg_offset = ldid_value / 8;
    ldid_value_offset = ldid_value % 8;

    /* Adding raid_value into LDID-to-RAID register */
    cxg_ra_reg->CXG_RA_RNI_LDID_TO_RAID_REG[reg_offset] |=
        ((uint64_t)raid <<
            (ldid_value_offset * NUM_BITS_RESERVED_FOR_RAID));

    /* Set corresponding valid bit */
    cxg_ra_reg->CXG_RA_RNI_LDID_TO_RAID_VAL |= ((uint64_t)0x1 << ldid_value);

    /* Increment the global raid_value */
    raid_value++;
}

static void program_cxg_ra_rnd_ldid_to_raid_reg(
    struct cmn_rhodes_device_ctx *ctx, uint8_t ldid_value, uint8_t raid)
{
    uint32_t reg_offset = 0;
    uint32_t ldid_value_offset = 0;
    struct cmn_rhodes_cxg_ra_reg *cxg_ra_reg;

    cxg_ra_reg = ctx->cxg_ra_reg_table[config->ldid].cxg_ra_reg;

    /* Each 64-bit RA RND LDID-to-RAID register holds 8 LDIDs */
    reg_offset = ldid_value / 8;
    ldid_value_offset = ldid_value % 8;

    /* Adding raid_value into LDID-to-RAID register */
    cxg_ra_reg->CXG_RA_RND_LDID_TO_RAID_REG[reg_offset] |=
        ((uint64_t)raid <<
            (ldid_value_offset * NUM_BITS_RESERVED_FOR_RAID));

    /* Set corresponding valid bit */
    cxg_ra_reg->CXG_RA_RND_LDID_TO_RAID_VAL |= ((uint64_t)0x1 << ldid_value);

    /* Increment the global raid_value */
    raid_value++;
}

static void program_agentid_to_linkid_reg(
    struct cmn_rhodes_device_ctx *ctx,
    const struct mod_cmn_rhodes_ccix_config *config)
{
    uint32_t agentid;
    uint32_t reg_index = 0;
    uint8_t linkid = 0;
    uint32_t agentid_bit_offset = 0;
    struct cmn_rhodes_cxg_ra_reg *cxg_ra_reg;
    struct cmn_rhodes_cxg_ha_reg *cxg_ha_reg;
    struct cmn_rhodes_cxla_reg *cxla_reg;
    cxg_ra_reg = ctx->cxg_ra_reg_table[config->ldid].cxg_ra_reg;
    cxg_ha_reg = ctx->cxg_ha_reg_table[config->ldid].cxg_ha_reg;
    cxla_reg = ctx->cxla_reg_table[config->ldid].cxla_reg;

    unsigned int remote_agentid_start;
    unsigned int remote_agentid_end;

    for (linkid = 0; linkid < CMN_RHODES_MAX_CCIX_PROTOCOL_LINKS; linkid++) {
        remote_agentid_start =
            config->remote_agentid_to_linkid_map[linkid].remote_agentid_start;
        remote_agentid_end =
            config->remote_agentid_to_linkid_map[linkid].remote_agentid_end;

        /*
         * Skip configuring link if both start and end are 0, indicating
         * there's no link
         */
        if ((remote_agentid_start == 0) && (remote_agentid_end == 0))
            continue;

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
            cxg_ra_reg->CXG_RA_AGENTID_TO_LINKID_REG[reg_index] |=
                ((uint64_t)linkid <<
                 (agentid_bit_offset * NUM_BITS_RESERVED_FOR_LINKID));

            cxg_ha_reg->CXG_HA_AGENTID_TO_LINKID_REG[reg_index] |=
                ((uint64_t)linkid <<
                 (agentid_bit_offset * NUM_BITS_RESERVED_FOR_LINKID));

            cxla_reg->CXLA_AGENTID_TO_LINKID_REG[reg_index] |=
                ((uint64_t)linkid <<
                 (agentid_bit_offset * NUM_BITS_RESERVED_FOR_LINKID));

            /* Setting corresponding valid bits */
            cxg_ra_reg->CXG_RA_AGENTID_TO_LINKID_VAL |=
                ((uint64_t)0x1 << agentid);
            cxg_ha_reg->CXG_HA_AGENTID_TO_LINKID_VAL |=
                ((uint64_t)0x1 << agentid);
            cxla_reg->CXLA_AGENTID_TO_LINKID_VAL |= ((uint64_t)0x1 << agentid);
        }
    }
}

static void program_cxg_ha_id(struct cmn_rhodes_device_ctx *ctx, uint8_t haid)
{
    struct cmn_rhodes_cxg_ha_reg *cxg_ha_reg;

    cxg_ha_reg = ctx->cxg_ha_reg_table[config->ldid].cxg_ha_reg;

    FWK_LOG_INFO(
                      MOD_NAME "HAID for CCIX %d with nodeid %d: HAID %d",
                      config->ldid,
                      get_node_id(cxg_ha_reg),
                      haid);

    cxg_ha_reg->CXG_HA_ID = haid;
}

static void program_cxg_ha_raid_to_ldid_lut(struct cmn_rhodes_device_ctx *ctx,
    uint8_t raid_id, uint8_t ldid_value)
{
    uint32_t reg_index = 0;
    uint32_t raid_bit_offset = 0;
    struct cmn_rhodes_cxg_ha_reg *cxg_ha_reg;

    cxg_ha_reg = ctx->cxg_ha_reg_table[config->ldid].cxg_ha_reg;

    /* Each 64-bit RAID-to-LDID register holds 8 mappings, 8 bits each. */
    reg_index = raid_id / 8;
    raid_bit_offset = raid_id % 8;

    /* Write RAID-to-LDID mapping (with RNF bit set) */
    cxg_ha_reg->CXG_HA_RNF_RAID_TO_LDID_REG[reg_index] |=
        ((uint64_t)(ldid_value | CXG_HA_RAID_TO_LDID_RNF_MASK) <<
         (raid_bit_offset * NUM_BITS_RESERVED_FOR_LDID));

    /* Set corresponding valid bit */
    cxg_ha_reg->CXG_HA_RNF_RAID_TO_LDID_VAL |= ((uint64_t)0x1 << raid_id);
}

static void program_hnf_ldid_to_chi_node_id_reg(
    struct cmn_rhodes_device_ctx *ctx,
    const struct mod_cmn_rhodes_ccix_config *config)
{
    uint32_t agentid;
    uint32_t reg_index = 0;
    uint32_t reg_bit_offset = 0;
    uint32_t i = 0;
    struct cmn_rhodes_hnf_reg *hnf_reg = NULL;
    uint32_t nodeid_ra;
    uint8_t linkid = 0;

    unsigned int remote_agentid_start;
    unsigned int remote_agentid_end;

    /* Assign the NodeID of CXHA as the RA's NodeID */
    nodeid_ra = ctx->cxg_ha_reg_table[config->ldid].node_id;

    for (linkid = 0; linkid < CMN_RHODES_MAX_CCIX_PROTOCOL_LINKS; linkid++) {
        remote_agentid_start =
            config->remote_agentid_to_linkid_map[linkid].remote_agentid_start;
        remote_agentid_end =
            config->remote_agentid_to_linkid_map[linkid].remote_agentid_end;

        /*
         * Skip configuring link if both start and end are 0, indicating
         * there's no link
         */
        if ((remote_agentid_start == 0) && (remote_agentid_end == 0))
            continue;

        FWK_LOG_INFO(
                     MOD_NAME "  Remote [AgentID %d - AgentID %d] Link %d",
                     remote_agentid_start,
                     remote_agentid_end,
                     linkid);

        for (agentid = remote_agentid_start; agentid <= remote_agentid_end;
                agentid++) {
            /* Each 64-bit register holds 2 sets of config data, 32 bits each */
            reg_index = agentid / 2;
            reg_bit_offset = agentid % 2;

            for (i = 0; i < ctx->hnf_count; i++) {
                hnf_reg = (struct cmn_rhodes_hnf_reg *)ctx->hnf_node[i];
                /* Write CXHA NodeID, local/remote and valid bit */
                hnf_reg->RN_PHYS_ID[reg_index] |=
                    ((uint64_t)(nodeid_ra |
                                (REMOTE_CCIX_NODE <<
                                 HNF_RN_PHYS_RN_LOCAL_REMOTE_SHIFT_VAL) |
                                (UINT64_C(0x1) <<
                                 HNF_RN_PHYS_RN_ID_VALID_SHIFT_VAL)) <<
                     (reg_bit_offset * NUM_BITS_RESERVED_FOR_PHYS_ID));
            }
        }
    }
}

static int enable_smp_mode(struct cmn_rhodes_device_ctx *ctx,
    const struct mod_cmn_rhodes_ccix_config *config)
{
    struct cmn_rhodes_cxg_ra_reg *cxg_ra_reg;
    struct cmn_rhodes_cxg_ha_reg *cxg_ha_reg;
    struct cmn_rhodes_cxla_reg *cxla_reg;

    cxg_ra_reg = ctx->cxg_ra_reg_table[config->ldid].cxg_ra_reg;
    cxg_ha_reg = ctx->cxg_ha_reg_table[config->ldid].cxg_ha_reg;
    cxla_reg = ctx->cxla_reg_table[config->ldid].cxla_reg;

    cxg_ra_reg->CXG_RA_AUX_CTRL |= (1 << CXG_RA_AUX_CTRL_SMP_MODE_EN_SHIFT_VAL);
    cxg_ha_reg->CXG_HA_AUX_CTRL |= (1 << CXG_HA_AUX_CTRL_SMP_MODE_EN_SHIFT_VAL);
    cxla_reg->CXLA_AUX_CTRL |=
        (UINT64_C(0x1) << CXLA_AUX_CTRL_SMP_MODE_EN_SHIFT_VAL);

    FWK_LOG_INFO(MOD_NAME "SMP Mode Enabled");
    return FWK_SUCCESS;
}

static void program_cxg_ra_sam_addr_region(struct cmn_rhodes_device_ctx *ctx,
    const struct mod_cmn_rhodes_ccix_config *config)
{
    unsigned int i;
    uint64_t blocks;
    uint64_t size;
    struct cmn_rhodes_cxg_ra_reg *cxg_ra_reg;

    cxg_ra_reg = ctx->cxg_ra_reg_table[config->ldid].cxg_ra_reg;

    FWK_LOG_INFO(
                      MOD_NAME "Configuring RA SAM for CXRA NodeID %d",
                      get_node_id(cxg_ra_reg));

    for (i = 0; i < CMN_RHODES_MAX_RA_SAM_ADDR_REGION; i++) {
        /* If the size is zero, skip that entry */
        if (config->ra_mmap_table[i].size == 0)
            continue;

        /* Size must be a multiple of SAM_GRANULARITY */
        fwk_assert((config->ra_mmap_table[i].size % (64 * FWK_KIB)) == 0);

        /* Size also must be a power of two */
        fwk_assert((config->ra_mmap_table[i].size &
                    (config->ra_mmap_table[i].size - 1)) == 0);

        /* Region base should be naturally aligned to the region size */
        fwk_assert(config->ra_mmap_table[i].base %
                   config->ra_mmap_table[i].size == 0);

        FWK_LOG_INFO(
                     MOD_NAME "  [0x%llx - 0x%llx] HAID %d",
                     config->ra_mmap_table[i].base,
                     config->ra_mmap_table[i].base +
                     config->ra_mmap_table[i].size - 1,
                     config->ra_mmap_table[i].remote_haid);

        blocks = config->ra_mmap_table[i].size / (64 * FWK_KIB);
        size = fwk_math_log2(blocks);

        cxg_ra_reg->CXG_RA_SAM_ADDR_REGION_REG[i] =
            size | (config->ra_mmap_table[i].base) |
            ((uint64_t)config->ra_mmap_table[i].remote_haid <<
             SAM_ADDR_TARGET_HAID_SHIFT) |
            (SAM_ADDR_REG_VALID_MASK);

        i++;
    }
}

static int enable_and_start_ccix_link_up_sequence(
    struct cmn_rhodes_device_ctx *ctx, uint8_t linkid)
{
    int status;
    struct cxg_wait_condition_data wait_data;
    struct cmn_rhodes_cxg_ra_reg *cxg_ra_reg;
    struct cmn_rhodes_cxg_ha_reg *cxg_ha_reg;

    cxg_ra_reg = ctx->cxg_ra_reg_table[config->ldid].cxg_ra_reg;
    cxg_ha_reg = ctx->cxg_ha_reg_table[config->ldid].cxg_ha_reg;

    if (linkid > 2)
        return FWK_E_PARAM;

    wait_data.ctx = ctx;
    wait_data.linkid = linkid;

    FWK_LOG_INFO(
        MOD_NAME "Enabling CCIX link %d...", linkid);
    /* Set link enable bit to enable the CCIX link */
    cxg_ra_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_CTRL =
        CXG_LINK_CTRL_EN_MASK;
    cxg_ha_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_CTRL =
        CXG_LINK_CTRL_EN_MASK;

    /* Wait until link enable bits are set */
    wait_data.cond = CXG_LINK_CTRL_EN_BIT_SET;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                  CXG_PRTCL_LINK_CTRL_TIMEOUT,
                                  cxg_link_wait_condition,
                                  &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            MOD_NAME "Enabling CCIX link %d... Failed", linkid);
        return status;
    }
    FWK_LOG_INFO(
        MOD_NAME "Enabling CCIX link %d... Done", linkid);

    FWK_LOG_INFO(
        MOD_NAME "Verifying link down status...");
    /* Wait till link up bits are cleared in control register */
    wait_data.cond = CXG_LINK_CTRL_UP_BIT_CLR;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                  CXG_PRTCL_LINK_CTRL_TIMEOUT,
                                  cxg_link_wait_condition,
                                  &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            MOD_NAME "Verifying link down status... Failed");
        return status;
    }

    /* Wait till link down bits are set in status register */
    wait_data.cond = CXG_LINK_STATUS_DWN_BIT_SET;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                  CXG_PRTCL_LINK_CTRL_TIMEOUT,
                                  cxg_link_wait_condition,
                                  &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            MOD_NAME "Verifying link down status... Failed");
        return status;
    }

    /* Wait till link ACK bits are cleared in status register */
    wait_data.cond = CXG_LINK_STATUS_ACK_BIT_CLR;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                   CXG_PRTCL_LINK_CTRL_TIMEOUT,
                                   cxg_link_wait_condition,
                                   &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            MOD_NAME "Verifying link down status... Failed");
        return status;
    }
    FWK_LOG_INFO(
        MOD_NAME "Verifying link down status... Done");

    FWK_LOG_INFO(MOD_NAME "Bringing up link...");
    /* Bring up link using link request bit */
    cxg_ra_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_CTRL |=
        CXG_LINK_CTRL_REQ_MASK;
    cxg_ha_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_CTRL |=
        CXG_LINK_CTRL_REQ_MASK;

    /* Wait till link ACK bits are set in status register */
    wait_data.cond = CXG_LINK_STATUS_ACK_BIT_SET;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                   CXG_PRTCL_LINK_CTRL_TIMEOUT,
                                   cxg_link_wait_condition,
                                   &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(MOD_NAME "Bringing up link... Failed");
        return status;
    }

    /* Wait till link down bits are cleared in status register */
    wait_data.cond = CXG_LINK_STATUS_DWN_BIT_CLR;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                   CXG_PRTCL_LINK_CTRL_TIMEOUT,
                                   cxg_link_wait_condition,
                                   &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(MOD_NAME "Bringing up link... Failed");
        return status;
    }
    FWK_LOG_INFO(MOD_NAME "Bringing up link... Done");

    return FWK_SUCCESS;
}

int ccix_setup(const unsigned int chip_id, struct cmn_rhodes_device_ctx *ctx,
    const struct mod_cmn_rhodes_ccix_config *ccix_config)
{
    unsigned int i;
    unsigned int unique_remote_rnf_ldid_value;
    uint8_t rnf_ldid;
    uint8_t rni_ldid;
    uint8_t rnd_ldid;
    uint8_t agentid;
    uint8_t remote_agentid;
    uint8_t offset_id;
    uint8_t local_ra_cnt;
    int status;

    FWK_LOG_INFO(
        MOD_NAME "Programming CCIX gateway...");

    /* Assign the max count among the RNs as local_ra_cnt */
    if ((ctx->rnf_count > ctx->rnd_count) && (ctx->rnf_count > ctx->rni_count))
        local_ra_cnt = ctx->rnf_count;
    else if (ctx->rnd_count > ctx->rni_count)
        local_ra_cnt = ctx->rnd_count;
    else
        local_ra_cnt = ctx->rni_count;

    if (ccix_config->smp_mode == true) {
        status = enable_smp_mode(ctx, ccix_config);
        if (status != FWK_SUCCESS)
            return status;
    }


    /* Set the global config data */
    config = ccix_config;

    /*
     * In order to assign unique AgentIDs across multiple chips, chip_id is used
     * as factor to offset the AgentID value
     */
    raid_value = 0;
    offset_id = chip_id * local_ra_cnt;

    for (rnf_ldid = 0; rnf_ldid < ctx->rnf_count; rnf_ldid++) {
        agentid = raid_value + offset_id;

        /* Program RAID values in CXRA LDID to RAID LUT */
        program_cxg_ra_rnf_ldid_to_raid_reg(ctx, rnf_ldid, agentid);
    }

    /* Program agentid to linkid LUT for remote agents in CXRA/CXHA/CXLA */
    program_agentid_to_linkid_reg(ctx, ccix_config);

    /* Program HN-F ldid to CHI NodeID for remote RN-F agents */
    program_hnf_ldid_to_chi_node_id_reg(ctx, ccix_config);

    /*
     * unique_remote_rnf_ldid_value is used to keep track of the
     * ldid of the remote RNF agents
     */
    unique_remote_rnf_ldid_value = ctx->rnf_count;

    for (i = 0; i < ccix_config->remote_rnf_count; i++) {
         /*
          * The remote_agentid should not include the current chip's AgentIDs.
          * If `block` is less than the current chip_id, then include the
          * AgentIDs starting from chip 0 till (not including) current chip. If
          * the `block` is equal or greater than the current chip, then include
          * the AgentIDs from next chip till the max chip.
          */
         if ((i / ctx->rnf_count) < chip_id)
             remote_agentid = i;
         else
             remote_agentid = i + ctx->rnf_count;

        /* Program the CXHA raid to ldid LUT */
        program_cxg_ha_raid_to_ldid_lut(ctx, remote_agentid,
            unique_remote_rnf_ldid_value);

        unique_remote_rnf_ldid_value++;
    }

    /* Program the unique HAID for the CXHA block */
    program_cxg_ha_id(ctx, ccix_config->haid);

    raid_value = 0;
    offset_id = chip_id * local_ra_cnt;

    for (rnd_ldid = 0; rnd_ldid < ctx->rnd_count; rnd_ldid++) {
        /* Determine agentid of the remote agents */
        agentid = raid_value + offset_id;

        /* Program RAID values in CXRA LDID to RAID LUT */
        program_cxg_ra_rnd_ldid_to_raid_reg(ctx, rnd_ldid, agentid);
    }

    raid_value = 0;
    offset_id = chip_id * local_ra_cnt;

    for (rni_ldid = 0; rni_ldid < ctx->rni_count; rni_ldid++) {
        /* Determine agentid of the remote agents */
        agentid = raid_value + offset_id;

        /* Program RAID values in CXRA LDID to RAID LUT */
        program_cxg_ra_rni_ldid_to_raid_reg(ctx, rni_ldid, agentid);
    }

    /*
     * Program the CXRA SAM with the address range and the corresponding remote
     * HAID
     */
    program_cxg_ra_sam_addr_region(ctx, ccix_config);

    /* Program the Link Control registers present in CXRA/CXHA/CXLA */
    status = enable_and_start_ccix_link_up_sequence(ctx, 0);

    return status;
}

int ccix_exchange_protocol_credit(struct cmn_rhodes_device_ctx *ctx,
    const struct mod_cmn_rhodes_ccix_config *ccix_config)
{
    int linkid;
    struct cmn_rhodes_cxg_ra_reg *cxg_ra_reg;
    struct cmn_rhodes_cxg_ha_reg *cxg_ha_reg;

    config = ccix_config;
    cxg_ra_reg = ctx->cxg_ra_reg_table[config->ldid].cxg_ra_reg;
    cxg_ha_reg = ctx->cxg_ha_reg_table[config->ldid].cxg_ha_reg;

    /* TODO Add support to enable multiple links */
    linkid = 0;

    FWK_LOG_INFO(
        MOD_NAME "Exchanging protocol credits for link %d...", linkid);
    /* Exchange protocol credits using link up bit */
    cxg_ra_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_CTRL |=
        CXG_LINK_CTRL_UP_MASK;
    cxg_ha_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_CTRL |=
        CXG_LINK_CTRL_UP_MASK;
    FWK_LOG_INFO(
        MOD_NAME "Exchanging protocol credits for link %d... Done", linkid);

    return FWK_SUCCESS;
}

int ccix_enter_system_coherency(struct cmn_rhodes_device_ctx *ctx,
    const struct mod_cmn_rhodes_ccix_config *ccix_config)
{
    struct cxg_wait_condition_data wait_data;
    int status;
    int linkid;
    struct cmn_rhodes_cxg_ha_reg *cxg_ha_reg;

    config = ccix_config;
    cxg_ha_reg = ctx->cxg_ha_reg_table[config->ldid].cxg_ha_reg;

    /* TODO Add support to enable multiple links */
    linkid = 0;

    wait_data.ctx = ctx;
    wait_data.linkid = linkid;

    FWK_LOG_INFO(
        MOD_NAME "Entering system coherency for link %d...", linkid);
    /* Enter system coherency by setting DVMDOMAIN request bit */
    cxg_ha_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_CTRL |=
        CXG_LINK_CTRL_DVMDOMAIN_REQ_MASK;

    /* Wait till DVMDOMAIN ACK bit is set in status register */
    wait_data.cond = CXG_LINK_STATUS_HA_DVMDOMAIN_ACK_BIT_SET;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                  CXG_PRTCL_LINK_DVMDOMAIN_TIMEOUT,
                                  cxg_link_wait_condition,
                                  &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            MOD_NAME "Entering system coherency for link %d... Failed", linkid);
        return status;
    }
    FWK_LOG_INFO(
        MOD_NAME "Entering system coherency for link %d... Done", linkid);

    return FWK_SUCCESS;
}

int ccix_enter_dvm_domain(struct cmn_rhodes_device_ctx *ctx,
    const struct mod_cmn_rhodes_ccix_config *ccix_config)
{
    struct cxg_wait_condition_data wait_data;
    int status;
    int linkid;
    struct cmn_rhodes_cxg_ra_reg *cxg_ra_reg;

    config = ccix_config;
    cxg_ra_reg = ctx->cxg_ra_reg_table[config->ldid].cxg_ra_reg;

    /* TODO Add support to enable multiple links */
    linkid = 0;

    wait_data.ctx = ctx;
    wait_data.linkid = linkid;

    FWK_LOG_INFO(
        MOD_NAME "Entering DVM domain for link %d...", linkid);
    /* Enter system coherency by setting DVMDOMAIN request bit */
    cxg_ra_reg->LINK_REGS[linkid].CXG_PRTCL_LINK_CTRL |=
        CXG_LINK_CTRL_DVMDOMAIN_REQ_MASK;

    /* Wait till DVMDOMAIN ACK bit is set in status register */
    wait_data.cond = CXG_LINK_STATUS_RA_DVMDOMAIN_ACK_BIT_SET;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                  CXG_PRTCL_LINK_DVMDOMAIN_TIMEOUT,
                                  cxg_link_wait_condition,
                                  &wait_data);
    if (status != FWK_SUCCESS) {
        FWK_LOG_INFO(
            MOD_NAME "Entering DVM domain for link %d... Failed", linkid);
        return status;
    }
    FWK_LOG_INFO(
        MOD_NAME "Entering DVM domain for link %d... Done", linkid);

    return FWK_SUCCESS;
}

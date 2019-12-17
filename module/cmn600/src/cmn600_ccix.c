/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_macros.h>
#include <fwk_math.h>
#include <mod_log.h>
#include <internal/cmn600_ctx.h>
#include <internal/cmn600_ccix.h>


#define MOD_NAME "[CMN600] "

static bool cxg_link_wait_condition(void *data)
{
    uint64_t val1;
    uint64_t val2;
    uint8_t link_id;

    fwk_assert(data != NULL);

    struct cxg_wait_condition_data *wait_data =
        (struct cxg_wait_condition_data *)data;
    struct cmn600_ctx *ctx = wait_data->ctx;
    link_id = wait_data->link_id;

    switch (wait_data->cond) {
    case CXG_LINK_CTRL_EN_BIT_SET:
        val1 = ctx->cxg_ra_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_CTRL;
        val2 = ctx->cxg_ha_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_CTRL;
        return (((val1 & CXG_LINK_CTRL_EN_MASK) &&
                  (val2 & CXG_LINK_CTRL_EN_MASK)) != 0);

    case CXG_LINK_CTRL_UP_BIT_CLR:
        val1 = ctx->cxg_ra_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_CTRL;
        val2 = ctx->cxg_ha_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_CTRL;
        return ((((val1 & CXG_LINK_CTRL_UP_MASK) == 0) &&
                  ((val2 & CXG_LINK_CTRL_UP_MASK) == 0)) != 0);

    case CXG_LINK_STATUS_DWN_BIT_SET:
        val1 = ctx->cxg_ra_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_STATUS;
        val2 = ctx->cxg_ha_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_STATUS;
        return (((val1 & CXG_LINK_STATUS_DOWN_MASK) &&
                  (val2 & CXG_LINK_STATUS_DOWN_MASK)) != 0);

    case CXG_LINK_STATUS_DWN_BIT_CLR:
        val1 = ctx->cxg_ra_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_STATUS;
        val2 = ctx->cxg_ha_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_STATUS;
        return ((((val1 & CXG_LINK_STATUS_DOWN_MASK) == 0) &&
                  ((val2 & CXG_LINK_STATUS_DOWN_MASK) == 0)) != 0);

    case CXG_LINK_STATUS_ACK_BIT_SET:
        val1 = ctx->cxg_ra_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_STATUS;
        val2 = ctx->cxg_ha_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_STATUS;
        return (((val1 & CXG_LINK_STATUS_ACK_MASK) &&
                (val2 & CXG_LINK_STATUS_ACK_MASK)) != 0);

    case CXG_LINK_STATUS_ACK_BIT_CLR:
        val1 = ctx->cxg_ra_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_STATUS;
        val2 = ctx->cxg_ha_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_STATUS;
        return ((((val1 & CXG_LINK_STATUS_ACK_MASK) == 0) &&
                  ((val2 & CXG_LINK_STATUS_ACK_MASK) == 0)) != 0);

    case CXG_LINK_STATUS_HA_DVMDOMAIN_ACK_BIT_SET:
        val1 = ctx->cxg_ha_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_STATUS;
        return (((val1 & CXG_LINK_STATUS_DVMDOMAIN_ACK_MASK)) != 0);

    case CXG_LINK_STATUS_RA_DVMDOMAIN_ACK_BIT_SET:
        val1 = ctx->cxg_ra_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_STATUS;
        return (((val1 & CXG_LINK_STATUS_DVMDOMAIN_ACK_MASK)) != 0);

    default:
        fwk_assert(false);
        return false;
    }
}


static void program_cxg_ra_rnf_ldid_to_raid_reg(struct cmn600_ctx *ctx,
    uint8_t ldid_value, uint8_t raid)
{
    uint32_t reg_offset = 0;
    uint32_t ldid_value_offset = 0;

    /* Each 64-bit RA RNF LDID-to-RAID register holds 8 LDIDs */
    reg_offset = ldid_value / 8;
    ldid_value_offset = ldid_value % 8;

    /* Adding raid_value into LDID-to-RAID register */
    ctx->cxg_ra_reg->CXG_RA_RNF_LDID_TO_RAID_REG[reg_offset] |=
        ((uint64_t)raid <<
            (ldid_value_offset * NUM_BITS_RESERVED_FOR_RAID));

    /* Set corresponding valid bit */
    ctx->cxg_ra_reg->CXG_RA_RNF_LDID_TO_RAID_VAL |=
        ((uint64_t)0x1 << ldid_value);

    /* Increment raid_value */
    ctx->raid_value++;
}

static void program_cxg_ra_rni_ldid_to_raid_reg
    (struct cmn600_ctx * ctx, uint8_t ldid_value, uint8_t raid)
{
    uint32_t reg_offset = 0;
    uint32_t ldid_value_offset = 0;

    /* Each 64-bit RA RNI LDID-to-RAID register holds 8 LDIDs */
    reg_offset = ldid_value / 8;
    ldid_value_offset = ldid_value % 8;

    /* Adding raid_value into LDID-to-RAID register */
    ctx->cxg_ra_reg->CXG_RA_RNI_LDID_TO_RAID_REG[reg_offset] |=
        ((uint64_t)raid <<
            (ldid_value_offset * NUM_BITS_RESERVED_FOR_RAID));

    /* Set corresponding valid bit */
    ctx->cxg_ra_reg->CXG_RA_RNI_LDID_TO_RAID_VAL |=
        ((uint64_t)0x1 << ldid_value);

    /* Increment raid_value */
    ctx->raid_value++;
}

static void program_cxg_ra_rnd_ldid_to_raid_reg(struct cmn600_ctx *ctx,
    uint8_t ldid_value, uint8_t raid)
{
    uint32_t reg_offset = 0;
    uint32_t ldid_value_offset = 0;

    /* Each 64-bit RA RND LDID-to-RAID register holds 8 LDIDs */
    reg_offset = ldid_value / 8;
    ldid_value_offset = ldid_value % 8;

    /* Adding raid_value into LDID-to-RAID register */
    ctx->cxg_ra_reg->CXG_RA_RND_LDID_TO_RAID_REG[reg_offset] |=
        ((uint64_t)raid <<
            (ldid_value_offset * NUM_BITS_RESERVED_FOR_RAID));

    /* Set corresponding valid bit */
    ctx->cxg_ra_reg->CXG_RA_RND_LDID_TO_RAID_VAL |=
        ((uint64_t)0x1 << ldid_value);

    /* Increment raid_value */
    ctx->raid_value++;
}

static void program_agentid_to_linkid_reg(struct cmn600_ctx *ctx,
    uint8_t agent_id, uint8_t link_id)
{
    uint32_t reg_index = 0;
    uint32_t agentid_bit_offset = 0;

    /* Each register is 64 bits and holds 8 AgentID/LinkID mappings */
    reg_index = agent_id / 8;
    agentid_bit_offset = agent_id % 8;

    /* Writing AgentID-to-LinkID mappings */
    ctx->cxg_ra_reg->CXG_RA_AGENTID_TO_LINKID_REG[reg_index] |=
        ((uint64_t)link_id <<
            (agentid_bit_offset * NUM_BITS_RESERVED_FOR_LINK_ID));
    ctx->cxg_ha_reg->CXG_HA_AGENTID_TO_LINKID_REG[reg_index] |=
        ((uint64_t)link_id <<
            (agentid_bit_offset * NUM_BITS_RESERVED_FOR_LINK_ID));
    ctx->cxla_reg->CXLA_AGENTID_TO_LINKID_REG[reg_index] |=
        ((uint64_t)link_id <<
            (agentid_bit_offset * NUM_BITS_RESERVED_FOR_LINK_ID));

    /* Setting corresponding valid bits */
    ctx->cxg_ra_reg->CXG_RA_AGENTID_TO_LINKID_VAL |=
        ((uint64_t)0x1 << agent_id);
    ctx->cxg_ha_reg->CXG_HA_AGENTID_TO_LINKID_VAL |=
        ((uint64_t)0x1 << agent_id);
    ctx->cxla_reg->CXLA_AGENTID_TO_LINKID_VAL |= ((uint64_t)0x1 << agent_id);
}

static void program_cxg_ha_id(struct cmn600_ctx *ctx, uint8_t remote_ha_id)
{
    ctx->cxg_ha_id = ctx->chip_id;
    ctx->cxg_ha_node_id = 0x0;
    ctx->cxg_ha_id_remote = remote_ha_id;

    ctx->cxg_ha_reg->CXG_HA_ID = ctx->cxg_ha_id;
}

static void program_cxg_ha_raid_to_ldid_lut(struct cmn600_ctx *ctx,
    uint8_t raid_id, uint8_t ldid)
{
    uint32_t reg_index = 0;
    uint32_t raid_bit_offset = 0;

    /* Each 64-bit RAID-to-LDID register holds 8 mappings, 8 bits each. */
    reg_index = raid_id / 8;
    raid_bit_offset = raid_id % 8;

    /* Write RAID-to-LDID mapping (with RNF bit set) */
    ctx->cxg_ha_reg->CXG_HA_RNF_RAID_TO_LDID_REG[reg_index] |=
        ((uint64_t)(ldid | CXG_HA_RAID_TO_LDID_RNF_MASK) <<
            (raid_bit_offset * NUM_BITS_RESERVED_FOR_LDID));

    /* Set corresponding valid bit */
    ctx->cxg_ha_reg->CXG_HA_RNF_RAID_TO_LDID_VAL |= ((uint64_t)0x1 << raid_id);
}

static void program_hnf_ldid_to_chi_node_id_reg(struct cmn600_ctx *ctx,
    uint8_t ldid, uint32_t node_id, uint8_t local_remote)
{
    uint32_t reg_index = 0;
    uint32_t reg_bit_offset = 0;
    uint32_t i = 0;
    struct cmn600_hnf_reg *hnf_reg = NULL;

    /* Each 64-bit register holds 2 sets of config data, 32 bits each. */
    reg_index = ldid / 2;
    reg_bit_offset = ldid % 2;

    for (i = 0; i < ctx->hnf_count; i++) {
        hnf_reg = (struct cmn600_hnf_reg *)ctx->hnf_offset[i];
        /* Write NodeID, local/remote and valid bit */
        hnf_reg->RN_PHYS_ID[reg_index] |=
            ((uint64_t)(node_id | (local_remote <<
                                   HNF_RN_PHYS_RN_LOCAL_REMOTE_SHIFT_VAL) |
                                   ((uint64_t)0x1 <<
                                   HNF_RN_PHYS_RN_ID_VALID_SHIFT_VAL)) <<
                        (reg_bit_offset * NUM_BITS_RESERVED_FOR_PHYS_ID));
    }
}

static void program_cxg_ra_sam_addr_region(struct cmn600_ctx *ctx,
    struct mod_cmn600_ccix_remote_node_config *config)
{
    uint8_t i;
    uint64_t blocks;
    uint64_t sz;

    for (i = 0; i < config->remote_ha_mmap_count; i++) {
        /* Size must be a multiple of SAM_GRANULARITY */
        fwk_assert((config->remote_ha_mmap[i].size % (64 * 1024)) == 0);

        blocks = config->remote_ha_mmap[i].size / (64 * 1024);
        sz = fwk_math_log2(blocks);
        ctx->cxg_ra_reg->CXG_RA_SAM_ADDR_REGION_REG[i] =
            sz | (config->remote_ha_mmap[i].base) |
            ((uint64_t)ctx->cxg_ha_id_remote << SAM_ADDR_HOME_AGENT_ID_SHIFT) |
            (SAM_ADDR_REG_VALID_MASK);
    }
}

static int enable_and_start_ccix_link_up_sequence(struct cmn600_ctx *ctx,
    struct mod_cmn600_ccix_remote_node_config *config, uint8_t link_id)
{
    uint64_t val1;
    int status;
    struct cxg_wait_condition_data wait_data;

    if (link_id > 2)
        return FWK_E_PARAM;

    wait_data.ctx = ctx;
    wait_data.link_id = link_id;

    if (config->ccix_opt_tlp)
        ctx->cxla_reg->CXLA_CCIX_PROP_CONFIGURED |= PCIE_OPT_HDR_MASK;
    else
        ctx->cxla_reg->CXLA_CCIX_PROP_CONFIGURED &= ~PCIE_OPT_HDR_MASK;

    if (config->ccix_msg_pack_enable) {
        ctx->cxla_reg->CXLA_CCIX_PROP_CONFIGURED &=
            ~(1U << CXLA_CCIX_PROP_MSG_PACK_SHIFT_VAL);
    }

    ctx->cxla_reg->CXLA_CCIX_PROP_CONFIGURED &=
        ~CXLA_CCIX_PROP_MAX_PACK_SIZE_MASK;

    ctx->cxla_reg->CXLA_CCIX_PROP_CONFIGURED |=
        (CXLA_CCIX_PROP_MAX_PACK_SIZE_512 <<
        CXLA_CCIX_PROP_MAX_PACK_SIZE_SHIFT_VAL);

    ctx->cxla_reg->CXLA_LINKID_TO_PCIE_BUS_NUM =
        ((uint64_t)config->pcie_bus_num << (link_id * 16));

    /* Set up TC1 for PCIe so CCIx uses VC1 */
    val1 = ctx->cxla_reg->CXLA_PCIE_HDR_FIELDS &
           ~CXLA_PCIE_HDR_TRAFFIC_CLASS_MASK;

    ctx->cxla_reg->CXLA_PCIE_HDR_FIELDS =
        val1 | ((uint64_t)config->ccix_tc <<
        CXLA_PCIE_HDR_TRAFFIC_CLASS_SHIFT_VAL);

    val1 = ctx->cxla_reg->CXLA_PCIE_HDR_FIELDS &
           ~CXLA_PCIE_HDR_VENDOR_ID_MASK;

    ctx->cxla_reg->CXLA_PCIE_HDR_FIELDS =
        val1 | ((uint64_t)CCIX_VENDER_ID <<
        CXLA_PCIE_HDR_VENDOR_ID_SHIFT_VAL);

    ctx->log_api->log(MOD_LOG_GROUP_DEBUG,
        MOD_NAME "CXLA_PCIE_HDR_FIELDS: 0x%lx\n",
        ctx->cxla_reg->CXLA_PCIE_HDR_FIELDS);

    ctx->log_api->log(MOD_LOG_GROUP_DEBUG,
        MOD_NAME "Enabling CCIX link %d...", link_id);
    /* Set link enable bit to enable the CCIX link */
    ctx->cxg_ra_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_CTRL =
        CXG_LINK_CTRL_EN_MASK;
    ctx->cxg_ha_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_CTRL =
        CXG_LINK_CTRL_EN_MASK;

    /* Wait until link enable bits are set */
    wait_data.cond = CXG_LINK_CTRL_EN_BIT_SET;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                  CXG_PRTCL_LINK_CTRL_TIMEOUT,
                                  cxg_link_wait_condition,
                                  &wait_data);
    if (status != FWK_SUCCESS) {
        ctx->log_api->log(MOD_LOG_GROUP_INFO, "Failed\n");
        return status;
    }
    ctx->log_api->log(MOD_LOG_GROUP_DEBUG, "Done\n");

    ctx->log_api->log(MOD_LOG_GROUP_DEBUG,
        MOD_NAME "Verifying link down status...");
    /* Wait till link up bits are cleared in control register */
    wait_data.cond = CXG_LINK_CTRL_UP_BIT_CLR;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                  CXG_PRTCL_LINK_CTRL_TIMEOUT,
                                  cxg_link_wait_condition,
                                  &wait_data);
    if (status != FWK_SUCCESS) {
        ctx->log_api->log(MOD_LOG_GROUP_INFO, "Failed\n");
        return status;
    }

    /* Wait till link down bits are set in status register */
    wait_data.cond = CXG_LINK_STATUS_DWN_BIT_SET;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                  CXG_PRTCL_LINK_CTRL_TIMEOUT,
                                  cxg_link_wait_condition,
                                  &wait_data);
    if (status != FWK_SUCCESS) {
        ctx->log_api->log(MOD_LOG_GROUP_INFO, "Failed\n");
        return status;
    }

    /* Wait till link ACK bits are cleared in status register */
    wait_data.cond = CXG_LINK_STATUS_ACK_BIT_CLR;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                   CXG_PRTCL_LINK_CTRL_TIMEOUT,
                                   cxg_link_wait_condition,
                                   &wait_data);
    if (status != FWK_SUCCESS) {
        ctx->log_api->log(MOD_LOG_GROUP_INFO, "Failed\n");
        return status;
    }

    ctx->log_api->log(MOD_LOG_GROUP_DEBUG, "Done\n");

    ctx->log_api->log(MOD_LOG_GROUP_DEBUG, MOD_NAME "Bringing up link...");

    /* Bring up link using link request bit */
    ctx->cxg_ra_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_CTRL |=
        CXG_LINK_CTRL_REQ_MASK;
    ctx->cxg_ha_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_CTRL |=
        CXG_LINK_CTRL_REQ_MASK;

    /* Wait till link ACK bits are set in status register */
    wait_data.cond = CXG_LINK_STATUS_ACK_BIT_SET;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                   CXG_PRTCL_LINK_CTRL_TIMEOUT,
                                   cxg_link_wait_condition,
                                   &wait_data);
    if (status != FWK_SUCCESS) {
        ctx->log_api->log(MOD_LOG_GROUP_INFO, "Failed\n");
        return status;
    }


    /* Wait till link down bits are cleared in status register */
    wait_data.cond = CXG_LINK_STATUS_DWN_BIT_CLR;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                   CXG_PRTCL_LINK_CTRL_TIMEOUT,
                                   cxg_link_wait_condition,
                                   &wait_data);
    if (status != FWK_SUCCESS) {
        ctx->log_api->log(MOD_LOG_GROUP_INFO, "Failed\n");
        return status;
    }
    ctx->log_api->log(MOD_LOG_GROUP_DEBUG, "Done\n");
    return FWK_SUCCESS;
}

int ccix_setup(struct cmn600_ctx *ctx, void *remote_config)
{
    unsigned int i;
    uint8_t rnf_ldid;
    uint8_t rni_ldid;
    uint8_t rnd_ldid;
    uint8_t agent_id;
    uint8_t remote_agent_id;
    uint8_t offset_id;
    uint8_t rnf_cnt;
    uint8_t local_ra_cnt;
    int status;

    struct mod_cmn600_ccix_remote_node_config * ccix_remote_config =
        (struct mod_cmn600_ccix_remote_node_config *)remote_config;

    cmn600_setup_sam((struct cmn600_rnsam_reg *)((uint32_t)ctx->cxg_ra_reg));
    ctx->log_api->log(MOD_LOG_GROUP_DEBUG,
        MOD_NAME "Programming CCIX gateway...\n");

    /*
     *assert if outside the range as it next step value bein latched
     * to uint8_t
     */
    fwk_assert((ctx->external_rnsam_count > 1) &&
                (ctx->external_rnsam_count <= (0xFF + 1)));

    /* Number of local RN-F */
    rnf_cnt = ctx->external_rnsam_count - 1;

    /* Number of local RAs */
    local_ra_cnt = ctx->internal_rnsam_count + ctx->external_rnsam_count - 1;

    /* Set initial RAID value to 0. */
    ctx->raid_value = 0;

    if (ctx->chip_id == 0)
        offset_id = 0;
    else
        offset_id = local_ra_cnt;

    for (rnf_ldid = 0; rnf_ldid < rnf_cnt; rnf_ldid++) {
        agent_id = ctx->raid_value + offset_id;

        /* Program RAID values in CXRA LDID to RAID LUT */
        program_cxg_ra_rnf_ldid_to_raid_reg(ctx, rnf_ldid, agent_id);

        /*
         * Program agentid to linkid LUT for
         * remote agents in CXRA/CXHA/CXLA
         */
        program_agentid_to_linkid_reg(ctx, agent_id,
            ccix_remote_config->ccix_link_id);

        /*
         * The HN-F ldid to CHI node id valid bit for
         * local RN-F agents is already programmed
         */
        program_cxg_ha_id(ctx, ccix_remote_config->remote_ha_mmap[0].ha_id);
    }

    /*
     * Unique_ha_ldid_value is used to keep track of the
     * ldid of the remote RNF agents
     */
    ctx->unique_ha_ldid_value = rnf_cnt;

    if (ctx->chip_id == 0)
        offset_id = local_ra_cnt;
    else
        offset_id = 0;

    for (i = 0; i < ccix_remote_config->remote_ra_count; i++) {
        remote_agent_id = i + offset_id;

        /* Program the CXHA raid to ldid LUT */
        program_cxg_ha_raid_to_ldid_lut(ctx, remote_agent_id,
            ctx->unique_ha_ldid_value);
        /*
         * Program HN-F ldid to CHI node id for
         * remote RN-F agents
         */
        program_hnf_ldid_to_chi_node_id_reg(ctx, ctx->unique_ha_ldid_value,
            ctx->cxg_ha_node_id, REMOTE_CCIX_NODE);

        ctx->unique_ha_ldid_value++;
    }

    if (ctx->chip_id == 0)
        offset_id = 0;
    else
        offset_id = local_ra_cnt;

    for (i = 0; i < ctx->rnd_count; i++) {
        rnd_ldid = ctx->rnd_ldid[i];

        /* Determine agent_id of the remote agents */
        agent_id = ctx->raid_value + offset_id;

        /* Program raid values in CXRA LDID to RAID LUT */
        program_cxg_ra_rnd_ldid_to_raid_reg(ctx, rnd_ldid, agent_id);

        /* Program agentid to linkid LUT for remote agents */
        program_agentid_to_linkid_reg(ctx, agent_id,
            ccix_remote_config->ccix_link_id);
    }

    for (i = 0; i < ctx->rni_count; i++) {
        rni_ldid = ctx->rni_ldid[i];

        /* Determine agentid of the remote agents */
        agent_id = ctx->raid_value + offset_id;

        /* Program raid values in CXRA LDID to RAID LUT */
        program_cxg_ra_rni_ldid_to_raid_reg(ctx, rni_ldid, agent_id);

        /* Program agentid to linkid LUT for remote agents */
        program_agentid_to_linkid_reg(ctx, agent_id,
            ccix_remote_config->ccix_link_id);
    }

    program_cxg_ra_sam_addr_region(ctx, ccix_remote_config);
    status = enable_and_start_ccix_link_up_sequence(ctx, ccix_remote_config,
        ccix_remote_config->ccix_link_id);

    return status;
}

int ccix_exchange_protocol_credit(struct cmn600_ctx *ctx, uint8_t link_id)
{

    if (link_id > 2)
        return FWK_E_PARAM;

    ctx->log_api->log(MOD_LOG_GROUP_DEBUG,
        MOD_NAME "Exchanging protocol credits for link %d...", link_id);
    /* Exchange protocol credits using link up bit */
    ctx->cxg_ra_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_CTRL |=
        CXG_LINK_CTRL_UP_MASK;
    ctx->cxg_ha_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_CTRL |=
        CXG_LINK_CTRL_UP_MASK;
    ctx->log_api->log(MOD_LOG_GROUP_DEBUG, "Done\n");
    return FWK_SUCCESS;
}

int ccix_enter_system_coherency(struct cmn600_ctx *ctx, uint8_t link_id)
{
    struct cxg_wait_condition_data wait_data;
    int status;

    if (link_id > 2)
        return FWK_E_PARAM;

    wait_data.ctx = ctx;
    wait_data.link_id = link_id;

    ctx->log_api->log(MOD_LOG_GROUP_DEBUG,
        MOD_NAME "Entering system coherency for link %d...", link_id);
    /* Enter system coherency by setting DVMDOMAIN request bit */
    ctx->cxg_ha_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_CTRL |=
        CXG_LINK_CTRL_DVMDOMAIN_REQ_MASK;

    /* Wait till DVMDOMAIN ACK bit is set in status register */
    wait_data.cond = CXG_LINK_STATUS_HA_DVMDOMAIN_ACK_BIT_SET;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                  CXG_PRTCL_LINK_DVMDOMAIN_TIMEOUT,
                                  cxg_link_wait_condition,
                                  &wait_data);
    if (status != FWK_SUCCESS) {
        ctx->log_api->log(MOD_LOG_GROUP_INFO, "Failed\n");
        return status;
    }

    ctx->log_api->log(MOD_LOG_GROUP_DEBUG, "Done\n");
    return FWK_SUCCESS;
}

int ccix_enter_dvm_domain(struct cmn600_ctx *ctx, uint8_t link_id)
{
    struct cxg_wait_condition_data wait_data;
    int status;

    if (link_id > 2)
        return FWK_E_PARAM;

    wait_data.ctx = ctx;
    wait_data.link_id = link_id;

    ctx->log_api->log(MOD_LOG_GROUP_DEBUG,
        MOD_NAME "Entering DVM domain for link %d...", link_id);
    /* Enter system coherency by setting DVMDOMAIN request bit */
    ctx->cxg_ra_reg->LINK_REGS[link_id].CXG_PRTCL_LINK_CTRL |=
        CXG_LINK_CTRL_DVMDOMAIN_REQ_MASK;

    /* Wait till DVMDOMAIN ACK bit is set in status register */
    wait_data.cond = CXG_LINK_STATUS_RA_DVMDOMAIN_ACK_BIT_SET;
    status = ctx->timer_api->wait(FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
                                  CXG_PRTCL_LINK_DVMDOMAIN_TIMEOUT,
                                  cxg_link_wait_condition,
                                  &wait_data);
    if (status != FWK_SUCCESS) {
        ctx->log_api->log(MOD_LOG_GROUP_INFO, "Failed\n");
        return status;
    }

    ctx->log_api->log(MOD_LOG_GROUP_DEBUG, "Done\n");
    return FWK_SUCCESS;
}

void ccix_capabilities_get(struct cmn600_ctx *ctx)
{
    /* Populate maximum credit send capability */
    ctx->ccix_host_info.ccix_request_credits =
        (ctx->cxg_ra_reg->CXG_RA_UNIT_INFO & CXG_RA_REQUEST_TRACKER_DEPTH_MASK)
         >> CXG_RA_REQUEST_TRACKER_DEPTH_VAL;

    ctx->ccix_host_info.ccix_snoop_credits =
        (ctx->cxg_ha_reg->CXG_HA_UNIT_INFO & CXG_HA_SNOOP_TRACKER_DEPTH_MASK) >>
         CXG_HA_SNOOP_TRACKER_DEPTH_VAL;

    ctx->ccix_host_info.ccix_data_credits =
        (ctx->cxg_ha_reg->CXG_HA_UNIT_INFO & CXG_HA_WDB_DEPTH_MASK) >>
          CXG_HA_WDB_DEPTH_VAL;
}

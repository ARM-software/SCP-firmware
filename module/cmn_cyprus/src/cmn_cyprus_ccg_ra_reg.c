/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for accessing CCG RA registers.
 */

#include <internal/cmn_cyprus_ccg_ra_reg.h>
#include <internal/cmn_cyprus_common.h>
#include <internal/cmn_cyprus_reg.h>

#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>

/* RA SAM region configuration */
#define RA_SAM_REGION_VALID_POS (63)
#define RA_SAM_TARGET_HAID_POS  52
#define RA_SAM_TARGET_HAID_MASK UINT64_C(0x3FF)
#define RA_SAM_MIN_REGION_SIZE  (64 * FWK_KIB)

/* CCG Protocol Link Control register */
#define CCPRTCL_LINK_CTL_LINK_EN_POS        0
#define CCPRTCL_LINK_CTL_LINK_EN_MASK       UINT64_C(0x1)
#define CCPRTCL_LINK_CTL_LINK_REQ_POS       1
#define CCPRTCL_LINK_CTL_LINK_REQ_MASK      UINT64_C(0x1)
#define CCPRTCL_LINK_CTL_LINK_UP_POS        2
#define CCPRTCL_LINK_CTL_LINK_UP_MASK       UINT64_C(0x1)
#define CCPRTCL_LINK_CTL_DVMDOMAIN_REQ_POS  3
#define CCPRTCL_LINK_CTL_DVMDOMAIN_REQ_MASK UINT64_C(0x1)
#define CCPRTCL_LINK_CTL_SMP_MODE_EN_POS    16
#define CCPRTCL_LINK_CTL_SMP_MODE_EN_MASK   UINT64_C(0x1)

/* CCG Protocol Link Status register */
#define CCPRTCL_LINK_STATUS_LINK_ACK_POS       0
#define CCPRTCL_LINK_STATUS_LINK_ACK_MASK      UINT64_C(0x1)
#define CCPRTCL_LINK_STATUS_LINK_DOWN_POS      1
#define CCPRTCL_LINK_STATUS_LINK_DOWN_MASK     UINT64_C(0x1)
#define CCPRTCL_LINK_STATUS_DVMDOMAIN_ACK_POS  2
#define CCPRTCL_LINK_STATUS_DVMDOMAIN_ACK_MASK UINT64_C(0x1)

/* LDID to Expanded RAID */
#define NUM_BITS_RESERVED_FOR_RAID 16
#define EXP_RAID_VALID_BIT_OFFSET  15

/*
 * Number of LDID to Expanded RAID mappings per LDID to Expanded RAID register.
 */
#define LDID_TO_EXP_RAID_COUNT_PER_REG 4

/*
 * Helper Macro to get the register index of the LDID to Expanded RAID register.
 */
#define GET_LDID_TO_EXP_RAID_REG_IDX(ldid) \
    (ldid / LDID_TO_EXP_RAID_COUNT_PER_REG)

/*
 * Helper Macro to get the bit position of the LDID to Expanded RAID mapping.
 */
#define GET_LDID_TO_EXP_RAID_POS(ldid) \
    ((ldid % LDID_TO_EXP_RAID_COUNT_PER_REG) * NUM_BITS_RESERVED_FOR_RAID)

/*
 * Helper Macro to get the bit position of the valid bit for the LDID to
 * Expanded RAID mapping.
 */
#define GET_LDID_TO_EXP_RAID_VALID_POS(ldid) \
    (GET_LDID_TO_EXP_RAID_POS(ldid) + EXP_RAID_VALID_BIT_OFFSET);

/*
 * Configure the base address and the size of the remote memory region in the
 * CML RA SAM.
 */
inline int ccg_ra_configure_sam_addr_region(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t region_idx,
    uint64_t base,
    uint64_t size)
{
    uint64_t encoded_size;

    if (region_idx >= CCG_RA_SAM_ADDR_REG_COUNT) {
        return FWK_E_PARAM;
    }

    encoded_size = sam_encode_region_size(size, RA_SAM_MIN_REGION_SIZE);
    ccg_ra_reg->CCG_RA_SAM_ADDR_REGION_REG[region_idx] = encoded_size | base;

    return FWK_SUCCESS;
}

/* Configure the target HAID for the remote memory region in CML RA SAM */
inline int ccg_ra_configure_sam_addr_target_haid(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t region_idx,
    unsigned int target_haid)
{
    if (region_idx >= CCG_RA_SAM_ADDR_REG_COUNT) {
        return FWK_E_PARAM;
    }

    ccg_ra_reg->CCG_RA_SAM_ADDR_REGION_REG[region_idx] |=
        ((uint64_t)(target_haid & RA_SAM_TARGET_HAID_MASK)
         << RA_SAM_TARGET_HAID_POS);

    return FWK_SUCCESS;
}

/* Mark the remote memory region as valid for comparison in CML RA SAM */
inline int ccg_ra_set_sam_addr_region_valid(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t region_idx)
{
    if (region_idx >= CCG_RA_SAM_ADDR_REG_COUNT) {
        return FWK_E_PARAM;
    }

    ccg_ra_reg->CCG_RA_SAM_ADDR_REGION_REG[region_idx] |= UINT64_C(0x1)
        << RA_SAM_REGION_VALID_POS;

    return FWK_SUCCESS;
}

/* Mark the AgentID to LinkID mapping as valid */
inline int ccg_ra_set_agentid_to_linkid_valid(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    unsigned int agent_id)
{
    if (is_ccg_agent_id_valid(agent_id) != true) {
        return FWK_E_PARAM;
    }

    ccg_ra_reg->CCG_RA_AGENTID_TO_LINKID_VAL |= (UINT64_C(0x1) << agent_id);

    return FWK_SUCCESS;
}

/* Assign expanded RAID value for the RN-F LDID and set valid bit */
int ccg_ra_configure_rnf_ldid_to_exp_raid(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t rnf_ldid,
    uint16_t raid)
{
    uint8_t register_idx;
    uint8_t bit_pos;

    if (is_exp_raid_valid(raid) != true) {
        return FWK_E_PARAM;
    }

    register_idx = GET_LDID_TO_EXP_RAID_REG_IDX(rnf_ldid);
    if (register_idx >= CCG_RA_RNF_LDID_TO_EXP_RAID_REG_COUNT) {
        return FWK_E_RANGE;
    }

    bit_pos = GET_LDID_TO_EXP_RAID_POS(rnf_ldid);
    /* Configure raid_value in LDID-to-RAID register */
    ccg_ra_reg->CCG_RA_RNF_LDID_TO_EXP_RAID_REG[register_idx] |=
        ((uint64_t)raid << bit_pos);

    bit_pos = GET_LDID_TO_EXP_RAID_VALID_POS(rnf_ldid);
    /* Set corresponding valid bit */
    ccg_ra_reg->CCG_RA_RNF_LDID_TO_EXP_RAID_REG[register_idx] |= UINT64_C(0x1)
        << bit_pos;

    return FWK_SUCCESS;
}

/* Assign expanded RAID value for the RN-D LDID and set valid bit */
int ccg_ra_configure_rnd_ldid_to_exp_raid(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t rnd_ldid,
    uint16_t raid)
{
    uint8_t register_idx;
    uint8_t bit_pos;

    if (is_exp_raid_valid(raid) != true) {
        return FWK_E_PARAM;
    }

    register_idx = GET_LDID_TO_EXP_RAID_REG_IDX(rnd_ldid);
    if (register_idx >= CCG_RA_RND_LDID_TO_EXP_RAID_REG_COUNT) {
        return FWK_E_RANGE;
    }

    bit_pos = GET_LDID_TO_EXP_RAID_POS(rnd_ldid);
    /* Configure raid_value in LDID-to-RAID register */
    ccg_ra_reg->CCG_RA_RND_LDID_TO_EXP_RAID_REG[register_idx] |=
        ((uint64_t)raid << bit_pos);

    bit_pos = GET_LDID_TO_EXP_RAID_VALID_POS(rnd_ldid);
    /* Set corresponding valid bit */
    ccg_ra_reg->CCG_RA_RND_LDID_TO_EXP_RAID_REG[register_idx] |= UINT64_C(0x1)
        << bit_pos;

    return FWK_SUCCESS;
}

/* Assign expanded RAID value for the RN-I LDID and set valid bit */
int ccg_ra_configure_rni_ldid_to_exp_raid(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t rni_ldid,
    uint16_t raid)
{
    uint8_t register_idx;
    uint8_t bit_pos;

    if (is_exp_raid_valid(raid) != true) {
        return FWK_E_PARAM;
    }

    register_idx = GET_LDID_TO_EXP_RAID_REG_IDX(rni_ldid);
    if (register_idx >= CCG_RA_RNI_LDID_TO_EXP_RAID_REG_COUNT) {
        return FWK_E_RANGE;
    }

    bit_pos = GET_LDID_TO_EXP_RAID_POS(rni_ldid);
    /* Configure raid_value in LDID-to-RAID register */
    ccg_ra_reg->CCG_RA_RNI_LDID_TO_EXP_RAID_REG[register_idx] |=
        ((uint64_t)raid << bit_pos);

    bit_pos = GET_LDID_TO_EXP_RAID_VALID_POS(rni_ldid);
    /* Set corresponding valid bit */
    ccg_ra_reg->CCG_RA_RNI_LDID_TO_EXP_RAID_REG[register_idx] |= UINT64_C(0x1)
        << bit_pos;

    return FWK_SUCCESS;
}

/*
 * Set the given value in the bitfield in the CCG RA protocol link control
 * register.
 */
int ccg_ra_set_cml_link_ctl(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t link_id,
    enum CCG_RA_LINK_CTL bitfield,
    uint8_t value)
{
    int status;

    if (is_ccg_link_id_valid(link_id) != true) {
        return FWK_E_PARAM;
    }

    status = FWK_SUCCESS;

    switch (bitfield) {
    case CCG_RA_LINK_CTL_LINK_EN:
        /* Enable the link */
        ccg_ra_reg->LINK_REGS[link_id].CCPRTCL_LINK_CTL |=
            ((value & CCPRTCL_LINK_CTL_LINK_EN_MASK)
             << CCPRTCL_LINK_CTL_LINK_EN_POS);
        break;

    case CCG_RA_LINK_CTL_SMP_EN:
        /* Enable the SMP mode */
        ccg_ra_reg->LINK_REGS[link_id].CCPRTCL_LINK_CTL |=
            ((value & CCPRTCL_LINK_CTL_SMP_MODE_EN_MASK)
             << CCPRTCL_LINK_CTL_SMP_MODE_EN_POS);
        break;

    case CCG_RA_LINK_CTL_LINK_REQ:
        /* Request Link up */
        ccg_ra_reg->LINK_REGS[link_id].CCPRTCL_LINK_CTL |=
            ((value & CCPRTCL_LINK_CTL_LINK_REQ_MASK)
             << CCPRTCL_LINK_CTL_LINK_REQ_POS);
        break;

    case CCG_RA_LINK_CTL_LINK_UP:
        /* Set Link up status */
        ccg_ra_reg->LINK_REGS[link_id].CCPRTCL_LINK_CTL |=
            ((value & CCPRTCL_LINK_CTL_LINK_UP_MASK)
             << CCPRTCL_LINK_CTL_LINK_UP_POS);
        break;

    case CCG_RA_LINK_CTL_DVMDOMAIN_REQ:
        /* Set DVM Domain (SYSCOREQ) enable */
        ccg_ra_reg->LINK_REGS[link_id].CCPRTCL_LINK_CTL |=
            ((value & CCPRTCL_LINK_CTL_DVMDOMAIN_REQ_MASK)
             << CCPRTCL_LINK_CTL_DVMDOMAIN_REQ_POS);
        break;

    default:
        status = FWK_E_SUPPORT;
    };

    return status;
}

/*
 * Get the the requested bitfield value from the CCG RA protocol link control
 * register.
 */
int ccg_ra_get_cml_link_ctl(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t link_id,
    enum CCG_RA_LINK_CTL bitfield,
    uint8_t *value)
{
    int status;
    uint64_t reg_value;

    if ((value == NULL) || (is_ccg_link_id_valid(link_id) != true)) {
        return FWK_E_PARAM;
    }

    status = FWK_SUCCESS;
    reg_value = ccg_ra_reg->LINK_REGS[link_id].CCPRTCL_LINK_CTL;

    switch (bitfield) {
    case CCG_RA_LINK_CTL_LINK_EN:
        /* Get the Link enable bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_CTL_LINK_EN_POS) &
             CCPRTCL_LINK_CTL_LINK_EN_MASK);
        break;

    case CCG_RA_LINK_CTL_SMP_EN:
        /* Get the SMP mode enable bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_CTL_SMP_MODE_EN_POS) &
             CCPRTCL_LINK_CTL_SMP_MODE_EN_MASK);
        break;

    case CCG_RA_LINK_CTL_LINK_REQ:
        /* Get the Link Up/Down request bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_CTL_LINK_REQ_POS) &
             CCPRTCL_LINK_CTL_LINK_REQ_MASK);
        break;

    case CCG_RA_LINK_CTL_LINK_UP:
        /* Get the Link Up status bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_CTL_LINK_UP_POS) &
             CCPRTCL_LINK_CTL_LINK_UP_MASK);
        break;

    case CCG_RA_LINK_CTL_DVMDOMAIN_REQ:
        /* Get the DVM domain (SYSCOREQ) enable bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_CTL_DVMDOMAIN_REQ_POS) &
             CCPRTCL_LINK_CTL_DVMDOMAIN_REQ_MASK);
        break;

    default:
        status = FWK_E_SUPPORT;
    };

    return status;
}

/*
 * Get the the requested bitfield value from the CCG RA protocol link status
 * register.
 */
int ccg_ra_get_cml_link_status(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t link_id,
    enum CCG_RA_LINK_STATUS bitfield,
    uint8_t *value)
{
    int status;
    uint64_t reg_value;

    if ((value == NULL) || is_ccg_link_id_valid(link_id) != true) {
        return FWK_E_PARAM;
    }

    status = FWK_SUCCESS;
    reg_value = ccg_ra_reg->LINK_REGS[link_id].CCPRTCL_LINK_STATUS;

    switch (bitfield) {
    case CCG_RA_LINK_STATUS_ACK:
        /* Get the Link Up/Down acknowledge bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_STATUS_LINK_ACK_POS) &
             CCPRTCL_LINK_STATUS_LINK_ACK_MASK);
        break;

    case CCG_RA_LINK_STATUS_DOWN:
        /* Get the Link Down status bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_STATUS_LINK_DOWN_POS) &
             CCPRTCL_LINK_STATUS_LINK_DOWN_MASK);
        break;

    case CCG_RA_LINK_STATUS_DVMDOMAIN_ACK:
        /* Get the DVM domain status (SYSCOACK) bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_STATUS_DVMDOMAIN_ACK_POS) &
             CCPRTCL_LINK_STATUS_DVMDOMAIN_ACK_MASK);
        break;

    default:
        status = FWK_E_SUPPORT;
    };

    return status;
}

/* Assign expanded RAID value for the HN-S and set valid bit */
void ccg_ra_configure_hns_ldid_to_exp_raid(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint16_t raid)
{
    /* Configure RAID value */
    ccg_ra_reg->CCG_RA_HNS_LDID_TO_EXP_RAID_REG |= raid;

    /* Mark the LDID-to-RAID mapping as valid */
    ccg_ra_reg->CCG_RA_HNS_LDID_TO_EXP_RAID_REG |=
        (UINT64_C(0x1) << EXP_RAID_VALID_BIT_OFFSET);
}

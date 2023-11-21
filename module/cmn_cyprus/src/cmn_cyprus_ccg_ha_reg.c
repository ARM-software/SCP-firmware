/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for accessing CCG HA registers.
 */

#include <internal/cmn_cyprus_ccg_ha_reg.h>
#include <internal/cmn_cyprus_common.h>
#include <internal/cmn_cyprus_reg.h>

#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>

/* CCG HAID register */
#define HAID_MASK UINT64_C(0x3FF)

/* RN-F Expanded RAID to LDID register */
#define EXP_RAID_LDID_RNF_POS   14
#define EXP_RAID_LDID_VALID_POS 15
#define EXP_RAID_LDID_POS       16
#define EXP_RAID_LDID_MASK      UINT64_C(0x1FF)

/* CCG Protocol Link Control register */
#define CCPRTCL_LINK_CTL_LINK_EN_POS        0
#define CCPRTCL_LINK_CTL_LINK_EN_MASK       UINT64_C(0x1)
#define CCPRTCL_LINK_CTL_LINK_REQ_POS       1
#define CCPRTCL_LINK_CTL_LINK_REQ_MASK      UINT64_C(0x1)
#define CCPRTCL_LINK_CTL_LINK_UP_POS        2
#define CCPRTCL_LINK_CTL_LINK_UP_MASK       UINT64_C(0x1)
#define CCPRTCL_LINK_CTL_SNPDOMAIN_REQ_POS  3
#define CCPRTCL_LINK_CTL_SNPDOMAIN_REQ_MASK UINT64_C(0x1)
#define CCPRTCL_LINK_CTL_SMP_MODE_EN_POS    16
#define CCPRTCL_LINK_CTL_SMP_MODE_EN_MASK   UINT64_C(0x1)

/* CCG Protocol Link Status register */
#define CCPRTCL_LINK_STATUS_LINK_ACK_POS       0
#define CCPRTCL_LINK_STATUS_LINK_ACK_MASK      UINT64_C(0x1)
#define CCPRTCL_LINK_STATUS_LINK_DOWN_POS      1
#define CCPRTCL_LINK_STATUS_LINK_DOWN_MASK     UINT64_C(0x1)
#define CCPRTCL_LINK_STATUS_SNPDOMAIN_ACK_POS  2
#define CCPRTCL_LINK_STATUS_SNPDOMAIN_ACK_MASK UINT64_C(0x1)

/*
 * Number of Expanded RAID to LDID mappings per Expanded RAID to LDID register.
 */
#define EXP_RAID_TO_LDID_COUNT_PER_REG 4

/*
 * Helper Macro to get the register index of the Expanded RAID to LDID register.
 */
#define GET_RNF_EXP_RAID_TO_LDID_REG_IDX(raid) \
    (raid / EXP_RAID_TO_LDID_COUNT_PER_REG)

/*
 * Helper Macro to get the bit position of the Expanded RAID to LDID mapping.
 */
#define GET_RNF_EXP_RAID_TO_LDID_POS(raid) \
    ((raid % EXP_RAID_TO_LDID_COUNT_PER_REG) * EXP_RAID_LDID_POS)

/*
 * Helper Macro to get the bit position of the valid bit for the
 * Expanded RAID to LDID mapping.
 */
#define GET_RNF_EXP_RAID_TO_LDID_VALID_POS(raid) \
    (GET_RNF_EXP_RAID_TO_LDID_POS(raid) + EXP_RAID_LDID_VALID_POS)

/*
 * Helper Macro to get the bit position of the RN-F bit for the
 * Expanded RAID to LDID mapping.
 */
#define GET_RNF_EXP_RAID_TO_LDID_RNF_POS(raid) \
    (GET_RNF_EXP_RAID_TO_LDID_POS(raid) + EXP_RAID_LDID_RNF_POS)

/* Mark the AgentID to LinkID mapping as valid */
inline int ccg_ha_set_agentid_to_linkid_valid(
    struct cmn_cyprus_ccg_ha_reg *ccg_ha_reg,
    unsigned int agent_id)
{
    if (is_ccg_agent_id_valid(agent_id) != true) {
        return FWK_E_PARAM;
    }

    ccg_ha_reg->CCG_HA_AGENTID_TO_LINKID_VAL |= (UINT64_C(0x1) << agent_id);

    return FWK_SUCCESS;
}

/* Assign HAID to CCG HA node */
inline void ccg_ha_configure_haid(
    struct cmn_cyprus_ccg_ha_reg *ccg_ha_reg,
    unsigned int haid)
{
    ccg_ha_reg->CCG_HA_ID = (haid & HAID_MASK);
}

/* Assign LDID for the given Expanded RAID and set the valid bit */
int ccg_ha_configure_raid_to_ldid(
    struct cmn_cyprus_ccg_ha_reg *ccg_ha_reg,
    uint16_t raid,
    uint16_t ldid)
{
    uint8_t register_idx;
    uint8_t bit_pos;

    if (is_exp_raid_valid(raid) != true) {
        return FWK_E_PARAM;
    }

    register_idx = GET_RNF_EXP_RAID_TO_LDID_REG_IDX(raid);
    bit_pos = GET_RNF_EXP_RAID_TO_LDID_POS(raid);

    /* Configure the LDID for the given expanded RAID */
    ccg_ha_reg->CCG_HA_RNF_EXP_RAID_TO_LDID_REG[register_idx] |=
        (ldid & EXP_RAID_LDID_MASK) << bit_pos;

    bit_pos = GET_RNF_EXP_RAID_TO_LDID_VALID_POS(raid);
    /* Set valid bit */
    ccg_ha_reg->CCG_HA_RNF_EXP_RAID_TO_LDID_REG[register_idx] |= UINT64_C(0x1)
        << bit_pos;

    return FWK_SUCCESS;
}

/* Mark the given expanded RAID as RN-F */
int ccg_ha_set_raid_as_rnf(
    struct cmn_cyprus_ccg_ha_reg *ccg_ha_reg,
    uint16_t raid)
{
    uint8_t register_idx;
    uint8_t bit_pos;

    if (is_exp_raid_valid(raid) != true) {
        return FWK_E_PARAM;
    }

    register_idx = GET_RNF_EXP_RAID_TO_LDID_REG_IDX(raid);
    bit_pos = GET_RNF_EXP_RAID_TO_LDID_RNF_POS(raid);
    ccg_ha_reg->CCG_HA_RNF_EXP_RAID_TO_LDID_REG[register_idx] |= UINT64_C(0x1)
        << bit_pos;

    return FWK_SUCCESS;
}

/*
 * Set the given value in the bitfield in the CCG HA protocol link control
 * register.
 */
int ccg_ha_set_cml_link_ctl(
    struct cmn_cyprus_ccg_ha_reg *ccg_ha_reg,
    uint8_t link_id,
    enum CCG_HA_LINK_CTL bitfield,
    uint8_t value)
{
    int status;

    if (is_ccg_link_id_valid(link_id) != true) {
        return FWK_E_PARAM;
    }

    status = FWK_SUCCESS;

    switch (bitfield) {
    case CCG_HA_LINK_CTL_LINK_EN:
        /* Enable the link */
        ccg_ha_reg->LINK_REGS[link_id].CCPRTCL_LINK_CTL |=
            ((value & CCPRTCL_LINK_CTL_LINK_EN_MASK)
             << CCPRTCL_LINK_CTL_LINK_EN_POS);
        break;

    case CCG_HA_LINK_CTL_SMP_EN:
        /* Enable the SMP mode */
        ccg_ha_reg->LINK_REGS[link_id].CCPRTCL_LINK_CTL |=
            ((value & CCPRTCL_LINK_CTL_SMP_MODE_EN_MASK)
             << CCPRTCL_LINK_CTL_SMP_MODE_EN_POS);
        break;

    case CCG_HA_LINK_CTL_LINK_REQ:
        /* Request Link up */
        ccg_ha_reg->LINK_REGS[link_id].CCPRTCL_LINK_CTL |=
            ((value & CCPRTCL_LINK_CTL_LINK_REQ_MASK)
             << CCPRTCL_LINK_CTL_LINK_REQ_POS);
        break;

    case CCG_HA_LINK_CTL_LINK_UP:
        /* Set Link up status */
        ccg_ha_reg->LINK_REGS[link_id].CCPRTCL_LINK_CTL |=
            ((value & CCPRTCL_LINK_CTL_LINK_UP_MASK)
             << CCPRTCL_LINK_CTL_LINK_UP_POS);
        break;

    case CCG_HA_LINK_CTL_SNPDOMAIN_REQ:
        /* Set Snoop domain (SYSCOREQ) enable */
        ccg_ha_reg->LINK_REGS[link_id].CCPRTCL_LINK_CTL |=
            ((value & CCPRTCL_LINK_CTL_SNPDOMAIN_REQ_MASK)
             << CCPRTCL_LINK_CTL_SNPDOMAIN_REQ_POS);
        break;

    default:
        status = FWK_E_SUPPORT;
    };

    return status;
}

/*
 * Get the the requested bitfield value from the CCG HA protocol link control
 * register.
 */
int ccg_ha_get_cml_link_ctl(
    struct cmn_cyprus_ccg_ha_reg *ccg_ha_reg,
    uint8_t link_id,
    enum CCG_HA_LINK_CTL bitfield,
    uint8_t *value)
{
    int status;
    uint64_t reg_value;

    if ((value == NULL) || (is_ccg_link_id_valid(link_id) != true)) {
        return FWK_E_PARAM;
    }

    status = FWK_SUCCESS;
    reg_value = ccg_ha_reg->LINK_REGS[link_id].CCPRTCL_LINK_CTL;

    switch (bitfield) {
    case CCG_HA_LINK_CTL_LINK_EN:
        /* Get the Link enable bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_CTL_LINK_EN_POS) &
             CCPRTCL_LINK_CTL_LINK_EN_MASK);
        break;

    case CCG_HA_LINK_CTL_SMP_EN:
        /* Get the SMP mode enable bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_CTL_SMP_MODE_EN_POS) &
             CCPRTCL_LINK_CTL_SMP_MODE_EN_MASK);
        break;

    case CCG_HA_LINK_CTL_LINK_REQ:
        /* Get the Link Up/Down request bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_CTL_LINK_REQ_POS) &
             CCPRTCL_LINK_CTL_LINK_REQ_MASK);
        break;

    case CCG_HA_LINK_CTL_LINK_UP:
        /* Get the Link Up status bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_CTL_LINK_UP_POS) &
             CCPRTCL_LINK_CTL_LINK_UP_MASK);
        break;

    case CCG_HA_LINK_CTL_SNPDOMAIN_REQ:
        /* Get the Snoop domain (SYSCOREQ) enable bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_CTL_SNPDOMAIN_REQ_POS) &
             CCPRTCL_LINK_CTL_SNPDOMAIN_REQ_MASK);
        break;

    default:
        status = FWK_E_SUPPORT;
    };

    return status;
}

/*
 * Get the the requested bitfield value from the CCG HA protocol link status
 * register.
 */
int ccg_ha_get_cml_link_status(
    struct cmn_cyprus_ccg_ha_reg *ccg_ha_reg,
    uint8_t link_id,
    enum CCG_HA_LINK_STATUS bitfield,
    uint8_t *value)
{
    int status;
    uint64_t reg_value;

    if ((value == NULL) || is_ccg_link_id_valid(link_id) != true) {
        return FWK_E_PARAM;
    }

    status = FWK_SUCCESS;
    reg_value = ccg_ha_reg->LINK_REGS[link_id].CCPRTCL_LINK_STATUS;

    switch (bitfield) {
    case CCG_HA_LINK_STATUS_ACK:
        /* Get the Link Up/Down acknowledge bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_STATUS_LINK_ACK_POS) &
             CCPRTCL_LINK_STATUS_LINK_ACK_MASK);
        break;

    case CCG_HA_LINK_STATUS_DOWN:
        /* Get the Link Down status bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_STATUS_LINK_DOWN_POS) &
             CCPRTCL_LINK_STATUS_LINK_DOWN_MASK);
        break;

    case CCG_HA_LINK_STATUS_SNPDOMAIN_ACK:
        /* Get the Snoop domain status (SYSCOACK) bit */
        *value =
            ((reg_value >> CCPRTCL_LINK_STATUS_SNPDOMAIN_ACK_POS) &
             CCPRTCL_LINK_STATUS_SNPDOMAIN_ACK_MASK);
        break;

    default:
        status = FWK_E_SUPPORT;
    };

    return status;
}

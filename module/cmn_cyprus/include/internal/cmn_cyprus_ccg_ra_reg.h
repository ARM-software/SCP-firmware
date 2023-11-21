/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for accessing CCG Request Agent (RA)
 *     registers.
 */

#ifndef CMN_CYPRUS_CCG_RA_REG_INTERNAL_H
#define CMN_CYPRUS_CCG_RA_REG_INTERNAL_H

#include <internal/cmn_cyprus_reg.h>

#include <stdint.h>

/*
 * CCG RA Protocol Link control register bitfields.
 */
enum CCG_RA_LINK_CTL {
    /* Link Enable */
    CCG_RA_LINK_CTL_LINK_EN,

    /* SMP Mode Enable */
    CCG_RA_LINK_CTL_SMP_EN,

    /* Link up Request */
    CCG_RA_LINK_CTL_LINK_REQ,

    /* Link up status */
    CCG_RA_LINK_CTL_LINK_UP,

    /* DVM Domain (SYSCOREQ) Enable */
    CCG_RA_LINK_CTL_DVMDOMAIN_REQ,

    /* CCG RA protocol link control bitfield count */
    CCG_RA_LINK_CTL_COUNT,
};

/*
 * CCG RA link status bitfields.
 */
enum CCG_RA_LINK_STATUS {
    /* Link Up/Down Acknowledge status */
    CCG_RA_LINK_STATUS_ACK,

    /* Link down status */
    CCG_RA_LINK_STATUS_DOWN,

    /* DVM Domain status */
    CCG_RA_LINK_STATUS_DVMDOMAIN_ACK,

    /* CCG RA link status bitfield count */
    CCG_RA_LINK_STATUS_COUNT,
};

/*
 * Configure RA SAM region address range.
 *
 * \param ccg_ra_reg Pointer to the CCG RA node.
 *      \pre The CCG RA node pointer must be valid.
 * \param region_idx Region index.
 * \param base Region base address.
 * \param size Region size.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid region index.
 */
int ccg_ra_configure_sam_addr_region(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t region_idx,
    uint64_t base,
    uint64_t size);

/*
 * Configure RA SAM region target HAID.
 *
 * \param ccg_ra_reg Pointer to the CCG RA node.
 *      \pre The CCG RA node pointer must be valid.
 * \param region_idx Region index.
 * \param target_haid Target HAID.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid region index.
 */
int ccg_ra_configure_sam_addr_target_haid(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t region_idx,
    unsigned int target_haid);

/*
 * Set RA SAM region as valid.
 *
 * \param ccg_ra_reg Pointer to the CCG RA node.
 *      \pre The CCG RA node pointer must be valid.
 * \param region_idx Region index.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid region index.
 */
int ccg_ra_set_sam_addr_region_valid(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t region_idx);

/*
 * Set AgentID-to-LinkID mapping as valid.
 *
 * \param ccg_ra_reg Pointer to the CCG RA node.
 *      \pre The CCG RA node pointer must be valid.
 * \param agent_id Agent ID.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid agent id.
 */
int ccg_ra_set_agentid_to_linkid_valid(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    unsigned int agent_id);

/*
 * Configure LDID to expanded RAID for RN-F.
 *
 * \param ccg_ra_reg Pointer to the CCG RA node.
 *      \pre The CCG RA node pointer must be valid.
 * \param rnf_ldid RN-F LDID.
 * \param raid Expanded RAID.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid raid.
 * \retval ::FWK_E_RANGE Register index out of range.
 */
int ccg_ra_configure_rnf_ldid_to_exp_raid(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t rnf_ldid,
    uint16_t raid);

/*
 * Configure LDID to expanded RAID for RN-D.
 *
 * \param ccg_ra_reg Pointer to the CCG RA node.
 *      \pre The CCG RA node pointer must be valid.
 * \param rnd_ldid RN-D LDID.
 * \param raid Expanded RAID.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid raid.
 * \retval ::FWK_E_RANGE Register index out of range.
 */
int ccg_ra_configure_rnd_ldid_to_exp_raid(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t rnd_ldid,
    uint16_t raid);

/*
 * Configure LDID to expanded RAID for RN-I.
 *
 * \param ccg_ra_reg Pointer to the CCG RA node.
 *      \pre The CCG RA node pointer must be valid.
 * \param rni_ldid RN-I LDID.
 * \param raid Expanded RAID.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid raid.
 * \retval ::FWK_E_RANGE Register index out of range.
 */
int ccg_ra_configure_rni_ldid_to_exp_raid(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t rni_ldid,
    uint16_t raid);

/*
 * Set CCG protocol link control register bitfield to the given value.
 *
 * \param ccg_ra_reg Pointer to the CCG RA node.
 *      \pre The CCG RA node pointer must be valid.
 * \param link_id Link to be configured.
 *      \pre Only link 0 is supported.
 * \param bitfield Link control register bitfield.
 *      \pre Link control register bitfield must be valid.
 * \param value Value to be set.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid link id.
 * \retval ::FWK_E_SUPPORT Requested bitfield not supported.
 */
int ccg_ra_set_cml_link_ctl(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t link_id,
    enum CCG_RA_LINK_CTL bitfield,
    uint8_t value);

/*
 * Get CCG protocol link control register bitfield.
 *
 * \param ccg_ra_reg Pointer to the CCG RA node.
 *      \pre The CCG RA node pointer must be valid.
 * \param link_id Link id.
 *      \pre Only link 0 is supported.
 * \param bitfield Link control register bitfield.
 *      \pre Link control register bitfield must be valid.
 * \param[out] value Pointer to the variable that stores the value.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid value pointer or link id.
 * \retval ::FWK_E_SUPPORT Requested bitfield not supported.
 */
int ccg_ra_get_cml_link_ctl(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t link_id,
    enum CCG_RA_LINK_CTL bitfield,
    uint8_t *value);

/*
 * Get CCG protocol link status register bitfield.
 *
 * \param ccg_ra_reg Pointer to the CCG RA node.
 *      \pre The CCG RA node pointer must be valid.
 * \param link_id Link id.
 *      \pre Only link 0 is supported.
 * \param bitfield Link status register bitfield.
 *      \pre Link status register bitfield must be valid.
 * \param[out] value Pointer to the variable that stores the value.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid value pointer or link id.
 * \retval ::FWK_E_SUPPORT Requested bitfield not supported.
 */
int ccg_ra_get_cml_link_status(
    struct cmn_cyprus_ccg_ra_reg *ccg_ra_reg,
    uint8_t link_id,
    enum CCG_RA_LINK_STATUS bitfield,
    uint8_t *value);

#endif /* CMN_CYPRUS_CCG_RA_REG_INTERNAL_H */

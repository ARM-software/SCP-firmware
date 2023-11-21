/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for accessing CCG Home Agent (HA)
 *     registers.
 */

#ifndef CMN_CYPRUS_CCG_HA_REG_INTERNAL_H
#define CMN_CYPRUS_CCG_HA_REG_INTERNAL_H

#include <internal/cmn_cyprus_reg.h>

#include <stdint.h>

/*
 * CCG HA Protocol Link control register bitfield.
 */
enum CCG_HA_LINK_CTL {
    /* Link Enable */
    CCG_HA_LINK_CTL_LINK_EN,

    /* SMP Mode Enable */
    CCG_HA_LINK_CTL_SMP_EN,

    /* Link Up Request */
    CCG_HA_LINK_CTL_LINK_REQ,

    /* Link up status */
    CCG_HA_LINK_CTL_LINK_UP,

    /* Snoop Domain (SYSCOREQ) Enable */
    CCG_HA_LINK_CTL_SNPDOMAIN_REQ,

    /* CCG HA protocol link control bitfield count */
    CCG_HA_LINK_CTL_COUNT,
};

/*
 * CCG HA link status bitfields.
 */
enum CCG_HA_LINK_STATUS {
    /* Link Up/Down Acknowledge status */
    CCG_HA_LINK_STATUS_ACK,

    /* Link down status */
    CCG_HA_LINK_STATUS_DOWN,

    /* Snoop Domain status */
    CCG_HA_LINK_STATUS_SNPDOMAIN_ACK,

    /* CCG HA link status bitfield count */
    CCG_HA_LINK_STATUS_COUNT,
};

/*
 * Set the Agent ID to Link ID mapping as valid.
 *
 * \param ccg_ha_reg Pointer to the CCG HA node.
 *      \pre The CCG HA node pointer must be valid.
 * \param agent_id Logical agent ID.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid Logical agent ID.
 */
int ccg_ha_set_agentid_to_linkid_valid(
    struct cmn_cyprus_ccg_ha_reg *ccg_ha_reg,
    unsigned int agent_id);

/*
 * Configure Home Agent ID (HAID).
 *
 * \param ccg_ha_reg Pointer to the CCG HA node.
 *      \pre The CCG HA node pointer must be valid.
 * \param haid Home Agent ID.
 *
 * \return None.
 */
void ccg_ha_configure_haid(
    struct cmn_cyprus_ccg_ha_reg *ccg_ha_reg,
    unsigned int haid);

/*
 * Configure the LDID for the given RAID.
 *
 * \param ccg_ha_reg Pointer to the CCG HA node.
 *      \pre The CCG HA node pointer must be valid.
 * \param raid RAID value for which the LDID must be set.
 * \param ldid LDID value which must be set.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid RAID value.
 */
int ccg_ha_configure_raid_to_ldid(
    struct cmn_cyprus_ccg_ha_reg *ccg_ha_reg,
    uint16_t raid,
    uint16_t ldid);

/*
 * Set the given RAID as RN-F.
 *
 * \param ccg_ha_reg Pointer to the CCG HA node.
 *      \pre The CCG HA node pointer must be valid.
 * \param raid RAID value for which the RN-F bit must be set.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid RAID value.
 */
int ccg_ha_set_raid_as_rnf(
    struct cmn_cyprus_ccg_ha_reg *ccg_ha_reg,
    uint16_t raid);

/*
 * Set CCG protocol link control register bitfield to the given value.
 *
 * \param ccg_ha_reg Pointer to the CCG HA node.
 *      \pre The CCG HA node pointer must be valid.
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
int ccg_ha_set_cml_link_ctl(
    struct cmn_cyprus_ccg_ha_reg *ccg_ha_reg,
    uint8_t link_id,
    enum CCG_HA_LINK_CTL bitfield,
    uint8_t value);

/*
 * Get CCG protocol link control register bitfield.
 *
 * \param ccg_ha_reg Pointer to the CCG HA node.
 *      \pre The CCG HA node pointer must be valid.
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
int ccg_ha_get_cml_link_ctl(
    struct cmn_cyprus_ccg_ha_reg *ccg_ha_reg,
    uint8_t link_id,
    enum CCG_HA_LINK_CTL bitfield,
    uint8_t *value);

/*
 * Get CCG protocol link status register bitfield.
 *
 * \param ccg_ha_reg Pointer to the CCG HA node.
 *      \pre The CCG HA node pointer must be valid.
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
int ccg_ha_get_cml_link_status(
    struct cmn_cyprus_ccg_ha_reg *ccg_ha_reg,
    uint8_t link_id,
    enum CCG_HA_LINK_STATUS bitfield,
    uint8_t *value);

#endif /* CMN_CYPRUS_CCG_HA_REG_INTERNAL_H */

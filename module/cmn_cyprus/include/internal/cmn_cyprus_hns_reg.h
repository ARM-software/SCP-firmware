/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Definitions and utility functions for accessing HN-S register.
 */

#ifndef CMN_CYPRUS_HNS_REG_INTERNAL_H
#define CMN_CYPRUS_HNS_REG_INTERNAL_H

#include <internal/cmn_cyprus_reg.h>

#include <mod_cmn_cyprus.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * HN-S power mode policies.
 * 4'b0000: OFF
 * 4'b0010: MEM_RET - logic is off, enabled RAMs are in retention
 * 4'b1000: ON - logic is on and enabled RAMs are operating as normal
 * 4'b0111: FUNC_RET - logic is on, enabled RAMs are in retention
 */
#define MOD_CMN_CYPRUS_HNS_PWPR_POLICY_OFF      0
#define MOD_CMN_CYPRUS_HNS_PWPR_POLICY_MEM_RET  2
#define MOD_CMN_CYPRUS_HNS_PWPR_POLICY_FUNC_RET 7
#define MOD_CMN_CYPRUS_HNS_PWPR_POLICY_ON       8

/*!
 * HN-S operational power modes.
 */
enum mod_cmn_cyprus_hns_pwpr_op_mode {
    /*! 4'b0000: NOSFSLC - Snoop filter and SLC are disabled and powered off */
    MOD_CMN_CYPRUS_HNS_PWPR_OP_MODE_NOSFSLC,

    /*!
     * 4'b0001: SFONLY - Snoop filter only Mode.
     * SF is enabled but the whole SLC is powered off.
     */
    MOD_CMN_CYPRUS_HNS_PWPR_OP_MODE_SFONLY,

    /*!
     * 4'b0010: HAM - Half-Associativity Mode.
     * SF is enabled but the upper half of the SLC ways are disabled and
     * powered off.
     */
    MOD_CMN_CYPRUS_HNS_PWPR_OP_MODE_HAM,

    /*!
     * 4'b0011: FAM - Full Associativity Mode.
     * SF and the entire SLC are enabled.
     */
    MOD_CMN_CYPRUS_HNS_PWPR_OP_MODE_FAM,

    /*! HN-F OP mode count */
    MOD_CMN_CYPRUS_HNS_PWPR_OP_MODE_COUNT,
};

/*!
 * HN-F to SN-F memory striping mode top address bits.
 */
enum mod_cmn_cyprus_hns_sam_top_address_bit {
    /*! Top Address Bit 0 */
    MOD_CMN_CYPRUS_HNS_SAM_TOP_ADDRESS_BIT0 = 0,

    /*! Top Address Bit 1 */
    MOD_CMN_CYPRUS_HNS_SAM_TOP_ADDRESS_BIT1 = 1,

    /*! Top Address Bit 2 */
    MOD_CMN_CYPRUS_HNS_SAM_TOP_ADDRESS_BIT2 = 2,

    /*! Top Address Bit Count */
    MOD_CMN_CYPRUS_HNS_SAM_TOP_ADDRESS_BIT_COUNT,
};

/*
 * Enable HN-F to SN-F memory striping mode.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param sn_mode memory striping mode to be enabled.
 *      \note Only 3-SN mode is supported currently.
 *
 * \return None.
 */
void hns_enable_sn_mode(
    struct cmn_cyprus_hns_reg *hns,
    enum mod_cmn_cyprus_hnf_sam_hashed_mode sn_mode);

/*
 * Configure top address bit for HN-F to SN-F memory striping mode.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param top_address_bit Valid \ref mod_cmn_cyprus_hns_sam_top_address_bit
 * \param top_address_bit_value top address bit value to be configured.
 *
 * \return None.
 */
void hns_configure_top_address_bit(
    struct cmn_cyprus_hns_reg *hns,
    enum mod_cmn_cyprus_hns_sam_top_address_bit top_address_bit,
    unsigned int top_address_bit_value);

/*
 * Configure the target SN node id.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param sn_node_id Target SN node id.
 * \param sn_idx SN index at which the SN node id must be programmed.
 *      \pre sn_idx must be 0, 1 or 2 as only 3-SN mode is supported currently.
 *
 * \return None.
 */
void hns_configure_sn_node_id(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int sn_node_id,
    uint8_t sn_idx);

/*
 * Check if Range based address comparison mode is enabled for HNFSAM.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 *
 * \retval true Range based address comparison mode is enabled for HNFSAM.
 * \retval false Range based address comparison mode is disabled for HNFSAM.
 */
bool hns_is_range_comparison_mode_enabled(struct cmn_cyprus_hns_reg *hns);

/*
 * Configure the Non-Hashed region address range.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param hnsam_range_comp_en_mode HNFSAM range based addres comparison mode
 *      status.
 * \param base_addr Base address of the non-hashed region to be
 *      configured.
 * \param size Size of the non-hashed region to be configured.
 * \param non_hashed_region_idx Index of the non-hashed region to be configured.
 *
 * \return None.
 */
void hns_configure_non_hashed_region_addr_range(
    struct cmn_cyprus_hns_reg *hns,
    bool hnsam_range_comp_en_mode,
    uint64_t base_addr,
    uint64_t size,
    unsigned int non_hashed_region_idx);

/*
 * Configure the Non-Hashed region target SN node id.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param sn_node_id Target SN node id to be configured.
 * \param non_hashed_region_idx Index of the non-hashed region for which the
 *      target SN node id is to be configured.
 *
 * \return None.
 */
void hns_configure_non_hashed_region_sn_node_id(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int sn_node_id,
    unsigned int non_hashed_region_idx);

/*
 * Mark the Non-Hashed region as valid for address comparison.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param non_hashed_region_idx Index of the non-hashed region.
 *
 * \return None.
 */
void hns_set_non_hashed_region_valid(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int non_hashed_region_idx);

/*
 * Enable HN-S node dynamic transition mode.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 *
 * \return None.
 */
void hns_set_pwpr_dynamic_enable(struct cmn_cyprus_hns_reg *hns);

/*
 * Configure HN-S operational power mode.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param op_mode Operational power mode to be set.
 *
 * \return None.
 */
void hns_set_pwpr_op_mode(
    struct cmn_cyprus_hns_reg *hns,
    enum mod_cmn_cyprus_hns_pwpr_op_mode op_mode);

/*
 * Configure HN-S power mode policy.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param policy Power mode policy to be set.
 *
 * \return None.
 */
void hns_set_pwpr_policy(struct cmn_cyprus_hns_reg *hns, uint8_t policy);

#endif /* CMN_CYPRUS_HNS_REG_INTERNAL_H */

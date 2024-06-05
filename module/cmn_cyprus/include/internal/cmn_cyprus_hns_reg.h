/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
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
 * LCN SAM target node type.
 */
enum lcn_sam_node_type {
    /*! 3'b000: Target node type HN-F */
    LCN_SAM_NODE_TYPE_HN_F,

    /*! 3'b001: Target node type HN-I */
    LCN_SAM_NODE_TYPE_HN_I,

    /*! 3'b010: Target node type CXRA */
    LCN_SAM_NODE_TYPE_CXRA,

    /*! 3'b011: Target node type HN-P */
    LCN_SAM_NODE_TYPE_HN_P,

    /*! 3'b100: Target node type PCI-CXRA */
    LCN_SAM_NODE_TYPE_PCI_CXRA,

    /*! 3'b101: Target node type HN-S */
    LCN_SAM_NODE_TYPE_HN_S,

    /*! LCN SAM node type count */
    LCN_SAM_NODE_TYPE_COUNT,
};

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

/*!
 * CHI source type of the Request nodes.
 */
enum cmn_cyprus_hns_rn_src_type {
    /*! 5'b01010: 256 bit CHI-B RN-F */
    CMN_CYPRUS_HNS_RN_SRC_TYPE_CHI_B = 10,

    /*! 5'b01011: 256 bit CHI-C RN-F */
    CMN_CYPRUS_HNS_RN_SRC_TYPE_CHI_C = 11,

    /*! 5'b01100: 256 bit CHI-D RN-F */
    CMN_CYPRUS_HNS_RN_SRC_TYPE_CHI_D = 12,

    /*! 5'b01101: 256 bit CHI-E RN-F */
    CMN_CYPRUS_HNS_RN_SRC_TYPE_CHI_E = 13,

    /*! 5'b10000: 256 bit CHI-F RN-F */
    CMN_CYPRUS_HNS_RN_SRC_TYPE_CHI_F = 16,
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

/*
 * Configure RN node id at the LDID index in non-clustered mode.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param ldid LDID index of the request node.
 *      \pre LDID index value must be less than 128.
 * \param node_id Node ID to be configured at the LDID index.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_RANGE ldid out of range.
 */
int hns_configure_rn_node_id(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int ldid,
    unsigned int node_id);

/*
 * Configure the RN node source type at the given LDID.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param ldid LDID index of the request node.
 *      \pre LDID index value must be less than 128.
 * \param src_type Source type of the request node.
 *      \pre Source type must be valid.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_RANGE ldid out of range.
 */
int hns_set_rn_node_src_type(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int ldid,
    enum cmn_cyprus_hns_rn_src_type src_type);

/*
 * Configure the RN node id at the LDID as remote.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param ldid LDID index of the request node.
 *      \pre LDID index value must be less than 128.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_RANGE ldid out of range.
 */
int hns_set_rn_node_remote(struct cmn_cyprus_hns_reg *hns, unsigned int ldid);

/*
 * Enable CML Port Aggregation (CPA) for the remote Request Node at the given
 * LDID.
 *
 * \details HN-S tracks the request nodes (both local and remote) using LDIDs.
 *     This function is used to enable CPA mode for remote request node using
 *     it's LDID as input.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param ldid LDID index of the remote request node.
 *      \pre LDID index value must be less than 128.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_RANGE LDID value out of range.
 */
int hns_enable_rn_cpa(struct cmn_cyprus_hns_reg *hns, unsigned int ldid);

/*
 * Configure CPA group ID for the remote RN node id at the LDID to RN node id
 * look-up table.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param ldid LDID index of the remote request node.
 *      \pre LDID index value must be less than 128.
 * \param cpag_id CPA group ID.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_RANGE LDID value out of range.
 */
int hns_configure_rn_cpag_id(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int ldid,
    uint8_t cpag_id);

/*
 * Configure target CCG HA node id in the CPA group.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param cpag_tgt_idx Target index.
 * \param ccg_ha_node_id Target CCG HA node ID.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_RANGE Register index out of range.
 */
int hns_configure_rn_cpag_node_id(
    struct cmn_cyprus_hns_reg *hns,
    uint8_t cpag_tgt_idx,
    unsigned int ccg_ha_node_id);

/*
 * Configure address range for the given hashed region in LCN SAM.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param region_idx Hashed region index.
 * \param region Hashed region memory map.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int hns_configure_lcn_htg_region_range(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int region_idx,
    const struct mod_cmn_cyprus_mem_region_map *region);

/*
 * Configure target type for the given hashed region in LCN SAM.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param region_idx Hashed region index.
 * \param target_type Target node type for the given HTG.
 *
 * \return None.
 */
void hns_set_lcn_htg_region_target_type(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int region_idx,
    enum lcn_sam_node_type target_type);

/*
 * Configure hashed region target node count in LCN SAM.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param region_idx Hashed region index.
 * \param hn_count Number of target nodes in the given HTG.
 *
 * \return None.
 */
void hns_set_lcn_htg_region_hn_count(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int region_idx,
    unsigned int hn_count);

/*
 * Enable CAL mode for the given hashed region in LCN SAM.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param region_idx Hashed region index.
 *
 * \return None.
 */
void hns_enable_lcn_htg_cal_mode(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int region_idx);

/*
 * Enable CPA mode for the given hashed region in LCN SAM.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param region_idx hashed region index.
 *      \pre Hashed region index must be valid.
 * \param hns_count Number of target HN-S nodes in the HTG.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_RANGE Register index out of range or invalid bit mask.
 */
int hns_enable_lcn_htg_cpa_mode(
    struct cmn_cyprus_hns_reg *hns,
    unsigned int region_idx,
    unsigned int hns_count);

/*
 * Configure CPA group ID for the given hashed region in LCN SAM.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param region_idx HTG region index.
 *      \pre HTG region index must be valid.
 * \param hns_count Number of target HN-S nodes in the HTG.
 * \param cpag_id CPA group ID.
 *
 * \return None.
 */
void hns_set_lcn_htg_cpag_id(
    struct cmn_cyprus_hns_reg *hns,
    uint8_t region_idx,
    unsigned int hns_count,
    uint8_t cpag_id);

/*
 * Mark the HTG region as valid in LCN SAM.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param region_idx HTG region index.
 *
 * \return None.
 */
void hns_set_lcn_htg_region_valid(
    struct cmn_cyprus_hns_reg *hns,
    uint8_t region_idx);

/*
 * Mark the HTG secondary region as valid in LCN SAM.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 * \param region_idx HTG region index.
 *
 * \return None.
 */
void hns_set_lcn_htg_sec_region_valid(
    struct cmn_cyprus_hns_reg *hns,
    uint8_t region_idx);

/*
 * Setup the LCN SAM programming context.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 *
 * \details Reads static configuration from the given RNSAM node and saves it in
 * the context structure. This eliminates the need to read the configuration
 * every time when programming an LCN node.
 *
 * \note All RNSAM nodes in a given CMN mesh are expected to have the same
 * configuration during the boot time.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid rnsam register pointer.
 */
int setup_lcnsam_ctx(struct cmn_cyprus_rnsam_reg *rnsam);

/*
 * Setup the HN-S programming context.
 *
 * \param hns Pointer to the HN-S node.
 *      \pre The HN-S node pointer must be valid.
 *
 * \details Reads static configuration from the given HN-S node and saves it in
 * the context structure. This eliminates the need to read the configuration
 * every time when programming an HN-S node.
 *
 * \note All HN-S nodes in a given CMN mesh are expected to have the same
 * initial configuration during the boot time.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid hns register pointer.
 *
 */
int setup_hns_ctx(struct cmn_cyprus_hns_reg *hns);

/*
 * Check if the HN-S node is isolated.
 *
 * \details Only the HN-S node pointer is compared to check whether
 *      the HN-S node is isolated. This is due to the assumption that
 *      the node pointer for isolated HN-S nodes is set to 0 during the
 *      discovery.
 *
 * \param hns Pointer to the HN-S node.
 *
 * \retval true The HN-S node is isolated.
 * \retval false The HN-S node is not isolated.
 */
bool is_hns_node_isolated(const struct cmn_cyprus_hns_reg *hns);

#endif /* CMN_CYPRUS_HNS_REG_INTERNAL_H */

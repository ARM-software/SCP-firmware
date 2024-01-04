/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Utility functions for the programming RN-SAM.
 */

#ifndef CMN_CYPRUS_RNSAM_REG_INTERNAL_H
#define CMN_CYPRUS_RNSAM_REG_INTERNAL_H

#include <internal/cmn_cyprus_reg.h>

#include <mod_cmn_cyprus.h>

#include <stdint.h>

/* Number of non-hashed regions supported */
#define RNSAM_NON_HASH_MEM_REGION_COUNT \
    (RNSAM_NON_HASH_REG_COUNT + RNSAM_NON_HASH_REG_GRP2_COUNT)

/* Number of hashed regions supported */
#define RNSAM_HASH_MEM_REGION_COUNT \
    (RNSAM_HTG_REG_COUNT + RNSAM_HTG_REG_GRP2_COUNT)

/* Non-hashed region target Node ID mask */
#define RNSAM_NON_HASH_TGT_NODEID_ENTRY_MASK UINT64_C(0x0FFF)

/*!
 * RNSAM node type.
 */
enum sam_node_type {
    /*! Target type HN-F. Used for hashed regions. */
    SAM_NODE_TYPE_HN_F,

    /*! Target type HN-I. Used for I/O regions. */
    SAM_NODE_TYPE_HN_I,

    /*! Target type CXRA. Used for remote regions. */
    SAM_NODE_TYPE_CXRA,

    /*! SAM node type count */
    SAM_NODE_TYPE_COUNT
};

/*!
 * RNSAM CPAG port type.
 */
enum cpa_port_type {
    /*! CPA port type CXL */
    CPA_PORT_TYPE_CXL,

    /*! CPA port type SMP */
    CPA_PORT_TYPE_SMP,

    /*! CPA port type count */
    CPA_PORT_TYPE_COUNT,
};

/*
 * Stall RNSAM requests and enable default target ID selection.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 *
 * \return nothing.
 */
void rnsam_stall(struct cmn_cyprus_rnsam_reg *rnsam);

/*
 * Unstall RNSAM requests and disable default target ID selection.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 *
 * \return nothing.
 */
void rnsam_unstall(struct cmn_cyprus_rnsam_reg *rnsam);

/*
 * Configure non-hashed region address range and target type in RNSAM.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param region_idx Non-hashed region index.
 * \param base Non-hashed region base address.
 * \param size Non-hashed region size.
 * \param target_type Non-hashed region target node type
 *      \ref enum sam_node_type.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Range comparision mode is enabled but base address is
 * not aligned with the size.
 */
int rnsam_configure_non_hashed_region(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx,
    uint64_t base,
    uint64_t size,
    enum sam_node_type target_type);

/*
 * Configure non-hashed region target node in RNSAM.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param region_idx Non-hashed region index.
 * \param node_id Non-hashed region target node id.
 *
 * \return nothing.
 */
void rnsam_set_non_hashed_region_target(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint32_t region_idx,
    unsigned int node_id);

/*
 * Mark the non-hashed region as valid in RNSAM.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param region_idx Non-hashed region index.
 *
 * \return nothing.
 */
void rnsam_set_non_hash_region_valid(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx);

/*
 * Configure hashed region address range and target type in RNSAM.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param region_idx Hashed region index.
 * \param base Hashed region base address.
 * \param size Hashed region size.
 * \param target_type Hashed region target node type
 *      \ref enum sam_node_type.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Range comparision mode is enabled but base address is
 * not aligned with the size.
 */
int rnsam_configure_hashed_region(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx,
    uint64_t base,
    uint64_t size,
    enum sam_node_type target_type);

/*
 * Configure hashed region secondary address range and target type in RNSAM.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param region_idx Hashed region index.
 * \param base Hashed region base address.
 * \param size Hashed region size.
 * \param target_type Hashed region target node type
 *      \ref enum sam_node_type.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Range comparision mode is enabled but base address is
 * not aligned with the size.
 */
int rnsam_configure_sec_hashed_region(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx,
    uint64_t base,
    uint64_t size,
    enum sam_node_type target_type);

/*
 * Configure hashed region target node ID in RNSAM.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param hn_node_id Target node IDs for hashed target groups.
 * \param hn_node_id_idx Target node ID index.
 *
 * \return nothing.
 */
void rnsam_set_htg_target_hn_nodeid(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint16_t hn_node_id,
    uint32_t hn_node_id_idx);

/*
 * Configure hashed region target node count in RNSAM.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param scg_idx SCG/hashed region index.
 * \param target_count Number of target nodes in the given SCG/HTG.
 *
 * \return nothing.
 */
void rnsam_set_htg_target_hn_count(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t scg_idx,
    uint16_t target_count);

/*
 * Enable CAL mode for the given SCG/HTG region.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param scg_idx SCG/hashed region index.
 *
 * \return nothing.
 */
void rnsam_enable_htg_cal_mode(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t scg_idx);

/*
 * Mark the hashed region as valid in RNSAM.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param region_idx hashed region index.
 *
 * \return nothing.
 */
void rnsam_set_htg_region_valid(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx);

/*
 * Mark the secondary hashed region as valid in RNSAM.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param region_idx hashed region index.
 *
 * \return nothing.
 */
void rnsam_set_htg_secondary_region_valid(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx);

/*
 * Get the programmed base address of the given non-hashed region.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param region_idx Non-hashed region index.
 *
 * \return Base address of the given non-hashed region.
 */
uint64_t rnsam_get_non_hashed_region_base(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx);

/*
 * Get the programmed region size of the given non-hashed region.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param region_idx Non-hashed region index.
 *
 * \return Size of the given non-hashed region.
 */
uint64_t rnsam_get_non_hashed_region_size(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx);

/*
 * Get the programmed target node ID of the given non-hashed region.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param region_idx Non-hashed region index.
 *
 * \return Target node ID of the given non-hashed region.
 */
unsigned int rnsam_get_non_hashed_region_target_id(
    struct cmn_cyprus_rnsam_reg *rnsam,
    unsigned int region_idx);

/*
 * Configure number of clusters in hierarchical hashing mode.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param scg_idx SCG/hashed region index.
 * \param num_cluster_groups Number of HN-S clusters.
 *
 * \return nothing.
 */
void rnsam_set_hier_hash_cluster_groups(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t scg_idx,
    uint8_t num_cluster_groups);

/*
 * Configure number of HN-S nodes per cluster in hierarchical hashing mode.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param scg_idx SCG/hashed region index.
 * \param num_hns_per_cluster Number of HN-S nodes per cluster.
 *
 * \return nothing.
 */
void rnsam_set_hier_hash_num_hns_per_cluster(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t scg_idx,
    uint8_t num_hns_per_cluster);

/*
 * Configure address striping mode for second level of hierarchy in
 * hierarchical hashing mode.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param scg_idx SCG/hashed region index.
 * \param num_cluster_groups Number of HN-S clusters.
 *
 * \return nothing.
 */
void rnsam_set_hier_hash_addr_striping(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t scg_idx,
    uint8_t num_cluster_groups);

/*
 * Enable hierarchical hashing mode for the given SCG/HTG region.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param scg_idx SCG/hashed region index.
 *
 * \return nothing.
 */
void rnsam_enable_hier_hash_mode(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t scg_idx);

/*
 * Setup the RNSAM programming context.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 *
 * \details Reads static configuration from the givne RNSAM node and saves it in
 * the context structure. This eliminates the need to read the configuration
 * every time when programming an RNSAM node.
 *
 * \note All RNSAM nodes in a given CMN mesh are expected to have the same
 * configuration during the boot time.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid rnsam register pointer.
 *
 */
int setup_rnsam_ctx(struct cmn_cyprus_rnsam_reg *rnsam_reg);

/*
 * Enable CPA mode for the given non-hashed region.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param region_idx Non-hashed region index.
 *      \pre Non-hashed region index must be valid.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_RANGE Register index out of range.
 */
int rnsam_enable_non_hash_region_cpa_mode(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t region_idx);

/*
 * Configure CPA group ID for the given non-hashed region.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param region_idx Non-hashed region index.
 *      \pre Non-hashed region index must be valid.
 * \param cpag_id CPA group ID.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_RANGE Register index out of range.
 */
int rnsam_configure_non_hash_region_cpag_id(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t region_idx,
    uint8_t cpag_id);

/*
 * Configure the number of CCG nodes in CPA group.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param cpag_id CPA group ID.
 * \param ccg_count Number of CCG nodes.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_PARAM Invalid ccg count.
 * \retval ::FWK_E_RANGE Register index out of range.
 */
int rnsam_configure_cpag_ccg_count(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t cpag_id,
    uint8_t ccg_count);

/*
 * Set the CPA group port type.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param cpag_id CPA group ID.
 * \param cpa_port_type CPA port type.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_RANGE Register index out of range.
 */
int rnsam_set_cpag_port_type(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t cpag_id,
    enum cpa_port_type port_type);

/*
 * Configure the target CCG RA node ID for the given CPA group.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param cpag_tgt_idx Index of the target CCG node.
 * \param ccg_ra_node_id Target CCG RA node id.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_RANGE Register index out of range.
 */
int rnsam_configure_cpag_target_id(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t cpag_tgt_idx,
    unsigned int ccg_ra_node_id);

/*
 * Configure the base index for the given CPA group.
 *
 * \param rnsam Pointer to the RNSAM node.
 *      \pre The RNSAM node pointer must be valid.
 * \param cpag_id CPA group ID.
 * \param base_index Base index.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \retval ::FWK_E_RANGE Register index out of range.
 */
int rnsam_configure_cpag_base_index(
    struct cmn_cyprus_rnsam_reg *rnsam,
    uint8_t cpag_id,
    uint8_t base_index);

#endif /* CMN_CYPRUS_RNSAM_REG_INTERNAL_H */

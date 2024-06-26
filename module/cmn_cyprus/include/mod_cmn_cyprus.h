/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_CMN_CYPRUS_H
#define MOD_CMN_CYPRUS_H

#include <fwk_id.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupModuleCMN_CYPRUS CMN_CYPRUS
 *
 * \brief Arm Coherent Mesh Network (CMN) Cyprus driver module.
 *
 * \details This module adds support for the CMN Cyprus interconnect.
 * @{
 */

/*! Maximum RA SAM Address regions */
#define CMN_CYPRUS_MAX_RA_SAM_ADDR_REGION 8

/*!
 * \brief Coordinate (x, y, port number, device number) of a node in the mesh.
 */
struct cmn_cyprus_node_pos {
    /*! X position of the node in the mesh */
    unsigned int pos_x;

    /*! Y position of the node in the mesh */
    unsigned int pos_y;

    /*! Port number of the node in the MXP */
    uint8_t port_num;

    /*! Device number of the node in the MXP port */
    unsigned int device_num;
};

/*!
 * \brief HN-F SAM target ID selection mode.
 */
enum mod_cmn_cyprus_hnf_sam_mode {
    /*! Legacy CMN mode: Direct SN mapping */
    MOD_CMN_CYPRUS_HNF_SAM_MODE_DIRECT_MAPPING,

    /*! Range based hashed target groups */
    MOD_CMN_CYPRUS_HNF_SAM_MODE_RANGE_BASED_HASHING,

    /*! HN-SAM target ID selection mode count */
    MOD_CMN_CYPRUS_HNF_SAM_MODE_COUNT,
};

/*!
 * \brief HN-F SAM range based hashing modes.
 *
 * \details HN‑F SAM supports three non-power-of-two memory striping which are
 * 3‑SN mode, 5‑SN mode, and 6-SN mode. In these modes, the HN‑F stripes
 * addresses across three SN‑Fs, five SN‑Fs, or six SN‑Fs respectively.
 *
 * \note Only 3-SN mode is supported currently.
 */
enum mod_cmn_cyprus_hnf_sam_hashed_mode {
    /*! 3-SN mode: Addresses from a hashed region are striped across 3 SNs */
    MOD_CMN_CYPRUS_HNF_SAM_HASHED_MODE_3_SN,

    /*! HN-F to SN-F memory striping mode count */
    MOD_CMN_CYPRUS_HNF_SAM_HASHED_MODE_COUNT,
};

/*!
 * \brief HN-F SAM range-based hashed SN target selection configuration.
 */
struct mod_cmn_cyprus_hnf_sam_range_based_hashing_config {
    /*!
     * \brief HN-S to SN-F hashing mode.
     */
    enum mod_cmn_cyprus_hnf_sam_hashed_mode sn_mode;

    /*!
     * \brief Top PA address bit 0 to use for striping.
     *
     * \note top_address_bit0 should match with the value in HN-S to SN-F strip
     * setting.
     */
    unsigned int top_address_bit0;

    /*!
     * \brief Top PA address bit 1 to use for striping.
     *
     * \note top_address_bit1 should match with the value in HN-S to SN-F strip
     * setting.
     */
    unsigned int top_address_bit1;
};

/*!
 * \brief HN-F SAM configuration.
 */
struct mod_cmn_cyprus_hnf_sam_config {
    /*!
     * \brief Table of SN-Fs used as targets for the HN-S nodes.
     *
     * \details Each entry of this table corresponds to a HN-S node in the
     *      system. The HN-S node's LDID is used as indices in this table.
     */
    const unsigned int *snf_table;

    /*! Number of entries in the \ref snf_table */
    size_t snf_count;

    /*! HN-F SAM target ID selection mode */
    enum mod_cmn_cyprus_hnf_sam_mode hnf_sam_mode;

    /*!
     * \brief Range-based hashed SN target selection mode configuration data.
     *
     * \note Only valid when MOD_CMN_CYPRUS_HNF_SAM_MODE_RANGE_BASED_HASHING
     * mode is selected.
     */
    struct mod_cmn_cyprus_hnf_sam_range_based_hashing_config hashed_mode_config;
};

/*!
 * \brief Memory region configuration type.
 */
enum mod_cmn_cyprus_mem_region_type {
    /*!
     * Sub region of the system cache for non-hashed access (serviced by
     * dedicated SN-F nodes).
     */
    MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE_SUB,

    /*!
     * Input/Output region.
     * Non-hashed region serviced by dedicated HN-I and HN-D nodes.
     */
    MOD_CMN_CYPRUS_MEM_REGION_TYPE_IO,

    /*!
     * Region backed by the system cache (serviced by HN-S nodes in the
     * system)
     */
    MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE,

    /*! Remote non-hashed region (serviced by remote HN) */
    MOD_CMN_CYPRUS_MEM_REGION_TYPE_REMOTE_NON_HASHED,

    /*!
     * Remote hashed region (serviced by remote HN-S nodes)
     */
    MOD_CMN_CYPRUS_MEM_REGION_TYPE_REMOTE_HASHED,

    /*! Memory region configuration type count */
    MOD_CMN_CYPRUS_MEM_REGION_TYPE_COUNT,
};

/*!
 * \brief Memory region descriptor.
 */
struct mod_cmn_cyprus_mem_region_map {
    /*! Base address */
    uint64_t base;

    /*! Region size in bytes */
    uint64_t size;

    /*! Region configuration type */
    enum mod_cmn_cyprus_mem_region_type type;

    /*!
     * Secondary region base address.
     *
     * \note To be used only with \ref
     * mod_cmn_cyprus_mem_region_type.MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE
     * memory regions.
     */
    uint64_t sec_region_base;

    /*!
     * Secondary region size in bytes.
     *
     * \note To be used only with \ref
     * mod_cmn_cyprus_mem_region_type.MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE
     * memory regions.
     */
    uint64_t sec_region_size;

    /*!
     * \brief Target node id.
     *
     * \note Not used for ::MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE memory
     *      regions as it uses the pool of HN-S nodes available in the
     *      system.
     */
    unsigned int node_id;

    /*!
     * \brief HN-S start and end positions of a SCG/HTG
     *
     * \details Each SCG/HTG covers an address range and this address range can
     * be made to target a group of HN-Ss. These group of HN-Ss are typically
     * bound by an arbitrary rectangle/square in the mesh. To aid automatic
     * programming of the HN-Ss in SCG/HTG along with the discovery process,
     * each SCG/HTG takes hns_pos_start and hns_pos_end. HN-S nodes which are
     * bound by this range will be assigned to the respective SCG/HTG. This
     * eliminates the process of manually looking at the mesh and assigning the
     * HN-S node ids to a SCG/HTG.
     *
     *                                        hns_pos_end
     *                                             xx
     *                                            xx
     *                                           xx
     *                    ┌─────────────────────xx
     *                    │                     │
     *                    │                     │
     *                    │                     │
     *                    │                     │
     *                    │    nth- SCG/HTG     │
     *                    │                     │
     *                    │                     │
     *                    │                     │
     *                    │                     │
     *                   xx─────────────────────┘
     *                  xx
     *                 xx
     *                xx
     *         hns_pos_start
     */

    /*!
     * \brief HN-S's bottom left node position
     *
     * \details \ref hns_pos_start is the HN-S's bottom left node position in
     * the rectangle covering the HN-Ss for a SCG/HTG
     *
     * \note To be used only with \ref
     * mod_cmn_cyprus_mem_region_type.MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE
     * memory regions.
     */
    struct cmn_cyprus_node_pos hns_pos_start;

    /*!
     * \brief HN-S's top right node position
     *
     * \details \ref hns_pos_start is the HN-S's bottom left node position in
     * the rectangle covering the HN-Ss for a SCG/HTG
     *
     * \note To be used only with \ref
     * mod_cmn_cyprus_mem_region_type.MOD_CMN_CYPRUS_MEM_REGION_TYPE_SYSCACHE
     * memory regions.
     */
    struct cmn_cyprus_node_pos hns_pos_end;
};

/*!
 * \brief CMN Cyprus configuration data.
 */
struct isolated_hns_node_info {
    /*! Position of the isolated HN-S node */
    struct cmn_cyprus_node_pos hns_pos;

    /*! Base address of the isolated HN-S node */
    uintptr_t hns_base;

    /*! Base address of the isolated HN-S MPAM_S node */
    uintptr_t hns_mpam_s_base;

    /*! Base address of the isolated HN-S MPAM_NS node */
    uintptr_t hns_mpam_ns_base;
};

/*!
 * \brief RNSAM SCG hashing mode.
 */
enum mod_cmn_cyprus_rnsam_scg_hashing_mode {
    /*!
     * Default mode; Legacy CMN mode: Power of two hashing. Note: This
     * mode can be used only when the number of HN-F nodes in the SCG
     * is a power of two.
     */
    MOD_CMN_CYPRUS_RNSAM_SCG_POWER_OF_TWO_HASHING,

    /*! Hierarchical hashing */
    MOD_CMN_CYPRUS_RNSAM_SCG_HIERARCHICAL_HASHING,

    /*! RNSAM hashing mode count */
    MOD_CMN_CYPRUS_RNSAM_SCG_MODE_COUNT,
};

/*!
 * \brief Hierarchical hashing configuration
 */
struct mod_cmn_cyprus_scg_hier_hashing_cfg {
    /*!
     * \brief Number of HN-S clusters in the first level hierarchy.
     *
     * \note Number of clusters must be a power of two.
     * Supported cluster count: 1, 2, 4, 8, 16, 32.
     */
    uint8_t num_cluster_groups;
};

/*!
 * \brief SCG configuration.
 */
struct mod_cmn_cyprus_rnsam_scg_config {
    /*! Hashing mechanism used for SCG regions */
    enum mod_cmn_cyprus_rnsam_scg_hashing_mode scg_hashing_mode;

    /*!
     * Hierarchcial hashing configuration.
     * \note Used only when hierarchical hashing mode is selected.
     */
    struct mod_cmn_cyprus_scg_hier_hashing_cfg hier_hash_cfg;
};

/*!
 * \brief Remote memory region configuration.
 */
struct mod_cmn_cyprus_remote_region {
    /*! Remote memory region memory map */
    const struct mod_cmn_cyprus_mem_region_map region_mmap;

    /*! Target HAID(s) for RA SAM region */
    unsigned int *target_haid;
};

/*!
 * \brief CML Port Aggregation Group config data.
 */
struct mod_cmn_cyprus_cpag_config {
    /*! CPA group ID */
    uint8_t cpag_id;

    /*!
     * Number of CCG nodes in CPAG.
     * Supported values: 1, 2, 4, 8, 16, 32 and 3 (MOD-3 hash).
     *
     * Note: Only CPA groups with identical number of CCG nodes is supported.
     * So, all CPA groups in the local chip must have equal number of CCG nodes.
     */
    uint8_t ccg_count;
};

/*!
 * \brief Coherent Mesh Link configuration.
 *
 * \details Used to describe a remote chip connection.
 */
struct mod_cmn_cyprus_cml_config {
    /*! Logical Device ID(s) of the CCG node(s) */
    unsigned int *ccg_ldid;

    /* HAID(s) to be assigned to the CCG HA node(s) */
    unsigned int *haid;

    /*!
     * Remote chip id. Identifier of the remote chip that's connected
     * to the CML.
     */
    uint8_t remote_chip_id;

    /*!
     * Remote chip memory region table.
     * Used to configure RNSAM.
     */
    const struct mod_cmn_cyprus_remote_region
        remote_mmap_table[CMN_CYPRUS_MAX_RA_SAM_ADDR_REGION];

    /*! Symmetric Multiprocessor (SMP) mode */
    bool enable_smp_mode;

    /*!
     * CCLA to CCLA direct connect mode.
     *
     * Note: This mode is used during pre-silicon validation stage and not used
     * when external controller IP is present.
     */
    bool enable_direct_connect_mode;

    /*! Enable CML Port Aggregation mode */
    bool enable_cpa_mode;

    /*! CPA group config */
    struct mod_cmn_cyprus_cpag_config cpag_config;
};

/*!
 * \brief CMN Cyprus configuration data.
 */
struct mod_cmn_cyprus_config {
    /*! Base address of the configuration address space */
    uintptr_t periphbase;

    /*! Size along x-axis of the interconnect mesh */
    unsigned int mesh_size_x;

    /*! Size along y-axis of the interconnect mesh */
    unsigned int mesh_size_y;

    /*! Identifier of the clock that this device depends on */
    fwk_optional_id_t clock_id;

    /*! Table of memory region entries */
    const struct mod_cmn_cyprus_mem_region_map *mmap_table;

    /*! Number of entries in the \ref mmap_table */
    size_t mmap_count;

    /*! HN-F SAM configuration data */
    const struct mod_cmn_cyprus_hnf_sam_config hnf_sam_config;

    /*!
     * \brief HN-S with CAL support flag
     *
     * \details When set to true, enables HN-S with CAL support. This flag will
     * be used only if HN-S is found to be connected to CAL (When connected to
     * a CAL port, node id of HN-S will be a odd number).
     *
     * \note Only CAL2 mode is supported at the moment.
     */
    bool hns_cal_mode;

    /*! Table of isolated HN-S nodes */
    struct isolated_hns_node_info *isolated_hns_table;

    /*! Number of entries in the \ref isolated_hns_table */
    unsigned int isolated_hns_count;

    /*! RNSAM SCG configuration data */
    struct mod_cmn_cyprus_rnsam_scg_config rnsam_scg_config;

    /*! Address space offset for non-hashed regions of the chip */
    uint64_t chip_addr_space;

    /*! CML configuration table */
    const struct mod_cmn_cyprus_cml_config *cml_config_table;

    /*!
     * \brief Number of entries in the
     *      ::mod_cmn_cyprus_config::cml_config_table table.
     */
    const uint8_t cml_table_count;

    /*!
     * Time to wait during CML protocol link bringup sequence.
     * A valid non-zero value must be specified.
     */
    uint32_t cml_poll_timeout_us;

    /*! Enable Local Coherency Node feature */
    bool enable_lcn;
};

/*!
 * \brief CMN Cyprus configuration table.
 */
struct mod_cmn_cyprus_config_table {
    /*! Chip specific CMN configuration table */
    const struct mod_cmn_cyprus_config *chip_config_data;

    /*! Number of entries in the \ref chip_config_data */
    const uint8_t chip_count;

    /*! Timer identifier */
    fwk_id_t timer_id;
};

/*!
 * \brief Module API indices
 */
enum mod_cmn_cyprus_api_idx {
    MOD_CMN_CYPRUS_API_IDX_MAP_IO_REGION,
    MOD_CMN_CYPRUS_API_COUNT,
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_CMN_CYPRUS_H */

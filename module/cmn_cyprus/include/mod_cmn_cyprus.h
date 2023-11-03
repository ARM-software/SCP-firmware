/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_CMN_CYPRUS_H
#define MOD_CMN_CYPRUS_H

#include <fwk_id.h>

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

    /*! Target node id */
    unsigned int node_id;
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
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_CMN_CYPRUS_H */


/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_CMN600_H
#define MOD_CMN600_H

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupModuleCMN600 CMN600
 *
 * \brief Arm Coherent Mesh Network (CMN) 600 module
 *
 * \details This module adds support for the CMN600 interconnect
 * @{
 */

/*!
 * \brief Module API indices
 */
enum mod_cmn600_api_idx {
    /*! Index of the PPU_V1 power state observer API */
    MOD_CMN600_API_IDX_PPU_OBSERVER,

    /*! Number of APIs */
    MOD_CMN600_API_COUNT
};

/*!
 * \brief Memory region configuration type
 */
enum mod_cmn600_memory_region_type {
    /*! Input/Output region (serviced by dedicated HN-I and HN-D nodes) */
    MOD_CMN600_MEMORY_REGION_TYPE_IO,

    /*!
     * Region backed by the system cache (serviced by all HN-F nodes in the
     * system)
     */
    MOD_CMN600_MEMORY_REGION_TYPE_SYSCACHE,

    /*!
     * Sub region of the system cache for non-hashed access (serviced by
     * dedicated SN-F nodes).
     */
    MOD_CMN600_REGION_TYPE_SYSCACHE_SUB,
};

/*!
 * \brief Memory region map descriptor
 */
struct mod_cmn600_memory_region_map {
    /*! Base address */
    uint64_t base;

    /*! Region size in bytes */
    uint64_t size;

    /*! Region configuration type */
    enum mod_cmn600_memory_region_type type;

    /*!
     * \brief Target node identifier
     *
     * \note Not used for \ref
     *      mod_cmn600_memory_region_type.MOD_CMN600_MEMORY_REGION_TYPE_SYSCACHE
     *      memory regions as it uses the pool of HN-F nodes available in the
     *      system
     */
    unsigned int node_id;
};

/*!
 * \brief CMN600 configuration data
 */
struct mod_cmn600_config {
    /*! Peripheral base address. */
    uintptr_t base;

    /*! Size along x-axis of the interconnect mesh */
    unsigned int mesh_size_x;

    /*! Size along y-axis of the interconnect mesh */
    unsigned int mesh_size_y;

    /*! Default HN-D node identifier containing the global configuration */
    unsigned int hnd_node_id;

    /*!
     * \brief Table of SN-Fs used as targets for the HN-F nodes
     *
     * \details Each entry of this table corresponds to a HN-F node in the
     *      system. The HN-F's logical identifiers are used as indices in this
     *      table
     */
    const unsigned int *snf_table;

    /*! Number of entries in the \ref snf_table */
    size_t snf_count;

    /*! Table of region memory map entries */
    const struct mod_cmn600_memory_region_map *mmap_table;

    /*! Number of entries in the \ref mmap_table */
    size_t mmap_count;

    /*! Identifier of the clock that this device depends on */
    fwk_id_t clock_id;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_CMN600_H */

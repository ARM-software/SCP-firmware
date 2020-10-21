/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_CMN650_H
#define MOD_CMN650_H

#include <fwk_id.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupModuleCMN650 CMN650
 *
 * \brief Arm Coherent Mesh Network (CMN) 650 module
 *
 * \details This module adds support for the CMN650 interconnect
 * \{
 */

/*! Maximum CCIX Protocol Links supported by CCIX Gateway (CXG) */
#define CMN650_MAX_CCIX_PROTOCOL_LINKS 3

/*! Maximum RA SAM Address regions */
#define CMN650_MAX_RA_SAM_ADDR_REGION 8

/*!
 * \brief Module API indices
 */
enum mod_cmn650_api_idx {
    /*! Index of the PPU_V1 power state observer API */
    MOD_CMN650_API_IDX_PPU_OBSERVER,

    /*! Number of APIs */
    MOD_CMN650_API_COUNT
};

/*!
 * \brief Memory region configuration type
 */
enum mod_cmn650_mem_region_type {
    /*! Input/Output region (serviced by dedicated HN-I and HN-D nodes) */
    MOD_CMN650_MEM_REGION_TYPE_IO,

    /*!
     * Region backed by the system cache (serviced by all HN-F nodes in the
     * system)
     */
    MOD_CMN650_MEM_REGION_TYPE_SYSCACHE,

    /*!
     * Sub region of the system cache for non-hashed access (serviced by
     * dedicated SN-F nodes).
     */
    MOD_CMN650_REGION_TYPE_SYSCACHE_SUB,

    /*!
     * Region used for CCIX access (serviced by the CXRA nodes).
     */
    MOD_CMN650_REGION_TYPE_CCIX,
};

/*!
 * \brief Memory region map descriptor
 */
struct mod_cmn650_mem_region_map {
    /*! Base address */
    uint64_t base;

    /*! Region size in bytes */
    uint64_t size;

    /*! Region configuration type */
    enum mod_cmn650_mem_region_type type;

    /*!
     * \brief Target node identifier
     *
     * \note Not used for ::MOD_CMN650_REGION_TYPE_SYSCACHE_SUB memory
     *      regions as it uses the pool of HN-F nodes available in the system.
     */
    unsigned int node_id;
};

/*!
 * \brief Remote Agent to Link ID mapping
 *
 * \details Each CCIX Gateway block (CXG) can communicate up to three remote
 *      CCIX protocol links. Each remote agent, identified by their AgentID
 *      (RAID or HAID) will be behind one of these three links. This structure
 *      holds the start and end Agent IDs for each link. The remote AgentID to
 *      LinkID LUT registers (por_{cxg_ra,cxg_ha,
 *      cxla}_agentid_to_linkid_reg<X>) will be configured sequentially from
 *      ::mod_cmn650_agentid_to_linkid_map::remote_agentid_start and
 *      ::mod_cmn650_agentid_to_linkid_map::remote_agentid_end values. For
 *      all three links, corresponding to these remote Agent IDs, HN-F's
 *      RN_PHYS_ID registers will be programmed with the node id of the CXG
 *      Gateway block.
 *
 */
struct mod_cmn650_agentid_to_linkid_map {
    /*! Remote Agent ID start */
    unsigned int remote_agentid_start;

    /*! Remote Agent ID end */
    unsigned int remote_agentid_end;
};

/*!
 * \brief Remote Memory region map descriptor which will be used by CXRA SAM
 *      programming
 */
struct mod_cmn650_ra_mem_region_map {
    /*! Base address */
    uint64_t base;

    /*! Region size in bytes */
    uint64_t size;

    /*! Target HAID of remote CXG gateway for CXRA SAM Address region */
    unsigned int remote_haid;
};

/*!
 * \brief CCIX Gateway block descriptor
 *
 * \details Each CCIX Gateway block (CXG) can have up to eight remote memory map
 *      ::mod_cmn650_ra_mem_region_map descriptors and can have three links
 *      which can target range of remote agent ids. User is expected to assign
 *      an Home AgentID (HAID) ::mod_cmn650_ccix_config::haid for each
 *      logical ids of the CXG blocks. Overall structure of the descriptor is
 *      shown below:
 *
 *         +----------------------------------------------------------+
 *         | mod_cmn650_ccix_config<ldid>                             |
 *         |                                                          |
 *         |   HAID = haid                                            |
 *         +----------------------------------------------------------+
 *         | ra_mmap_table0                   | agentid_to_linkid_map0|
 *         |                                  |  remote_agent_id_start|
 *         | base..base+size --> remote_haid  |  .                    |
 *         |                                  |  .                    |
 *         +----------------------------------+  .                    |
 *         | ra_mmap_table1                   |  .                    |
 *         |                                  |  remote_agent_id_end  |
 *         | base..base+size --> remote_haid  |                       |
 *         |                                  +-----------------------+
 *         +----------------------------------+ agentid_to_linkid_map1|
 *         | .                                |  remote_agent_id_start|
 *         | .                                |  .                    |
 *         | .                                |  .                    |
 *         | .                                |  .                    |
 *         | .                                |  .                    |
 *         | .                                |  remote_agent_id_end  |
 *         +----------------------------------+                       |
 *         | ra_mmap_table6                   +-----------------------+
 *         |                                  | agentid_to_linkid_map2|
 *         | base..base+size --> remote_haid  |  remote_agent_id_start|
 *         |                                  |  .                    |
 *         +----------------------------------+  .                    |
 *         | ra_mmap_table7                   |  .                    |
 *         |                                  |  .                    |
 *         | base..base+size --> remote_haid  |  remote_agent_id_end  |
 *         |                                  |                       |
 *         +----------------------------------+-----------------------+
 */
struct mod_cmn650_ccix_config {
    /*! Logical ID of the CXG block to which this configuration applies */
    unsigned int ldid;

    /*! Unique HAID in a multi-chip system. This has to be assigned manually */
    unsigned int haid;

    /*! Number of remote RN Caching agents. */
    unsigned int remote_rnf_count;

    /*! Table of region memory map entries */
    const struct mod_cmn650_mem_region_map remote_mmap_table;

    /*! Table of remote region memory map entries */
    const struct mod_cmn650_ra_mem_region_map
        ra_mmap_table[CMN650_MAX_RA_SAM_ADDR_REGION];

    /*! Number of entries in the ::mod_cmn650_ccix_config::ra_mmap_table */
    size_t ra_mmap_count;

    /*! Table of remote agent ids start and end backed by the links */
    struct mod_cmn650_agentid_to_linkid_map
        remote_agentid_to_linkid_map[CMN650_MAX_CCIX_PROTOCOL_LINKS];

    /*! SMP Mode */
    bool smp_mode;

    /*! Port Aggregation Mode */
    bool port_aggregate;

    /*!
     * \brief Logical ID (LDID) of the CCIX Gateway to which port aggregation
     *      pair to be created
     */
    unsigned int port_aggregate_ldid;

    /*! HAID of the CCIX Gateway to which port aggregation pair to be created */
    unsigned int port_aggregate_haid;

    /*!
     * \brief Target HAID of remote CXG gateway for CXRA SAM Address region for
     *      the port aggregated CXG block
     */
    unsigned int port_aggregate_remote_haid[CMN650_MAX_RA_SAM_ADDR_REGION];
};

/*!
 * \brief CMN650 configuration data
 */
struct mod_cmn650_config {
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

    /*! Number of entries in the ::mod_cmn650_config::snf_table */
    size_t snf_count;

    /*! Table of region memory map entries */
    const struct mod_cmn650_mem_region_map *mmap_table;

    /*! Number of entries in the ::mod_cmn650_config::mmap_table */
    size_t mmap_count;

    /*! Table of CCIX configuration */
    const struct mod_cmn650_ccix_config *ccix_config_table;

    /*!
     * \brief Number of entries in the
     *      ::mod_cmn650_config::ccix_config_table table.
     */
    const size_t ccix_table_count;

    /*! Address space size of the chip */
    uint64_t chip_addr_space;

    /*! Identifier of the clock that this device depends on */
    fwk_id_t clock_id;

    /*!
     * \brief HN-F with CAL support flag
     * \details When set to true, enables HN-F with CAL support. This flag will
     *      be used only if HN-F is found to be connected to CAL (When connected
     *      to a CAL port, node id of HN-F will be a odd number).
     */
    bool hnf_cal_mode;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_CMN650_H */

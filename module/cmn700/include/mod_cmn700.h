/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_CMN700_H
#define MOD_CMN700_H

#include <fwk_id.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupModuleCMN700 CMN700
 *
 * \brief Arm Coherent Mesh Network (CMN) 700 module
 *
 * \details This module adds support for the CMN700 interconnect
 * @{
 */

/*! Maximum CCG Protocol Links supported */
#define CMN700_MAX_CCG_PROTOCOL_LINKS 3

/*! Maximum RA SAM Address regions */
#define CMN700_MAX_RA_SAM_ADDR_REGION 8

/*!
 * \brief Memory region configuration type
 */
enum mod_cmn700_mem_region_type {
    /*! Input/Output region (serviced by dedicated HN-I and HN-D nodes) */
    MOD_CMN700_MEM_REGION_TYPE_IO,

    /*!
     * Region backed by the system cache (serviced by all HN-F nodes in the
     * system)
     */
    MOD_CMN700_MEM_REGION_TYPE_SYSCACHE,

    /*!
     * Sub region of the system cache for non-hashed access (serviced by
     * dedicated SN-F nodes).
     */
    MOD_CMN700_REGION_TYPE_SYSCACHE_SUB,

    /*!
     * Region used for CCG access (serviced by the CCRA nodes).
     */
    MOD_CMN700_REGION_TYPE_CCG,
};

/*!
 * \brief Coordinate (x, y, port no) of a node in the mesh
 */
struct node_pos {
    /*! x position of the node in the mesh */
    unsigned int pos_x;

    /*! y position of the node in the mesh */
    unsigned int pos_y;

    /*! port position of the node in the xp */
    unsigned int port_num;
};

/*!
 * \brief Memory region map descriptor
 */
struct mod_cmn700_mem_region_map {
    /*! Base address */
    uint64_t base;

    /*! Region size in bytes */
    uint64_t size;

    /*! Region configuration type */
    enum mod_cmn700_mem_region_type type;

    /*!
     * \brief Target node identifier
     *
     * \note Not used for \ref
     * mod_cmn700_mem_region_type.MOD_CMN700_REGION_TYPE_SYSCACHE_SUB
     * memory regions as it uses the pool of HN-F nodes available in the
     * system
     */
    unsigned int node_id;

    /*!
     * \brief HN-F start and end positions of a SCG/HTG
     *
     * \details Each SCG/HTG covers an address range and this address range can
     * be made to target a group of HN-Fs. These group of HN-Fs are typically
     * bound by an arbitrary rectangle/square in the mesh. To aid automatic
     * programming of the HN-Fs in SCG/HTG along with the discovery process,
     * each SCG/HTG takes hnf_pos_start and hnf_pos_end. HN-F nodes which are
     * bounded by this range will be assigned to the respective SCG/HTG. This
     * eliminates the process of manually looking at the mesh and assigning the
     * HN-F node ids to a SCG/HTG.
     *
     *                                        hnf_pos_end
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
     *         hnf_pos_start
     */

    /*!
     * \brief HN-F's bottom left node position
     *
     * \details \ref hnf_pos_start is the HN-F's bottom left node position in
     * the rectangle covering the HN-Fs for a SCG/HTG
     *
     * \note To be used only with \ref
     * mod_cmn700_mem_region_type.MOD_CMN700_MEM_REGION_TYPE_SYSCACHE memory
     * regions.
     */
    struct node_pos hnf_pos_start;

    /*!
     * \brief HN-F's top right node position
     *
     * \details \ref hnf_pos_start is the HN-F's bottom left node position in
     * the rectangle covering the HN-Fs for a SCG/HTG
     *
     * \note To be used only with \ref
     * mod_cmn700_mem_region_type.MOD_CMN700_MEM_REGION_TYPE_SYSCACHE memory
     * regions.
     */
    struct node_pos hnf_pos_end;
};

/*!
 * \brief HN-F to SN-F memory striping modes
 */
enum mod_cmn700_hnf_to_snf_mem_strip_mode {
    MOD_CMN700_1_SN_MODE,
    MOD_CMN700_3_SN_MODE,
    MOD_CMN700_6_SN_MODE,
    MOD_CMN700_5_SN_MODE,
};

/*!
 * \brief Hierarchical hashing configuration
 */
struct mod_cmn700_hierarchical_hashing {
    /*!
     * \brief Number of HN-Fs per cluster.
     *
     * \note The value should not account for \ref
     * mod_cmn700_config.hnf_cal_mode
     */
    unsigned int hnf_cluster_count;

    /*!
     * \brief HN-F to SN-F hashing mode.
     */
    enum mod_cmn700_hnf_to_snf_mem_strip_mode sn_mode;

    /*!
     * \brief Top PA address bit 0 to use for striping
     *
     * \note top_address_bit0 should match with the value in HN-F to SN-F strip
     * setting
     */
    unsigned int top_address_bit0;

    /*!
     * \brief Top PA address bit 1 to use for striping
     *
     * \note top_address_bit1 should match with the value in HN-F to SN-F strip
     * setting
     */
    unsigned int top_address_bit1;

    /*!
     * \brief Top PA address bit 2 to use for striping
     *
     * \note top_address_bit2 should match with the value in HN-F to SN-F strip
     * setting
     */
    unsigned int top_address_bit2;
};

/*!
 * \brief Remote Agent to Link ID mapping
 *
 * \details Each CCG nodes can communicate up to three remote CCG protocol
 *      links. Each remote agent, identified by their AgentID (RAID or HAID)
 *      will be behind one of these three links. This structure holds the start
 *      and end Agent IDs for each link. The remote AgentID to LinkID LUT
 *      registers (por_{ccg_ra,ccg_ha, ccla}_agentid_to_linkid_reg<X>) will be
 *      configured sequentially from
 *      ::mod_cmn700_agentid_to_linkid_map::remote_agentid_start and
 *      ::mod_cmn700_agentid_to_linkid_map::remote_agentid_end values. For
 *      all three links, corresponding to these remote Agent IDs, HN-F's
 *      RN_PHYS_ID registers will be programmed with the node id of the CCG
 *      block.
 */
struct mod_cmn700_agentid_to_linkid_map {
    /*! Remote Agent ID start */
    unsigned int remote_agentid_start;

    /*! Remote Agent ID end */
    unsigned int remote_agentid_end;
};

/*!
 * \brief Remote Memory region map descriptor which will be used by CCRA SAM
 *      programming
 */
struct mod_cmn700_ra_mem_region_map {
    /*! Base address */
    uint64_t base;

    /*! Region size in bytes */
    uint64_t size;

    /*! Target HAID of remote CCG for CCRA SAM Address region */
    unsigned int remote_haid;
};

/*!
 * \brief CCG block descriptor
 *
 * \details Each CCG block can have up to eight remote memory map
 *      ::mod_cmn700_ra_mem_region_map descriptors and can have three links
 *      which can target range of remote agent ids. User is expected to assign
 *      an Home AgentID (HAID) ::mod_cmn700_ccg_config::haid for each
 *      logical ids of the CCG blocks. Overall structure of the descriptor is
 *      shown below:
 *
 *         +----------------------------------------------------------+
 *         | mod_cmn700_ccg_config<ldid>                              |
 *         |                                                          |
 *         |   HAID = haid                                            |
 *         +----------------------------------+-----------------------+
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
struct mod_cmn700_ccg_config {
    /*! Logical ID of the CCG block to which this configuration applies */
    unsigned int ldid;

    /*! Unique HAID in a multi-chip system. This has to be assigned manually */
    unsigned int haid;

    /*! Number of remote RN Caching agents. */
    unsigned int remote_rnf_count;

    /*! Table of region memory map entries */
    const struct mod_cmn700_mem_region_map remote_mmap_table;

    /*! Table of remote region memory map entries */
    const struct mod_cmn700_ra_mem_region_map
        ra_mmap_table[CMN700_MAX_RA_SAM_ADDR_REGION];

    /*! Number of entries in the ::mod_cmn700_ccg_config::ra_mmap_table */
    size_t ra_mmap_count;

    /*! Table of remote agent ids start and end backed by the links */
    struct mod_cmn700_agentid_to_linkid_map
        remote_agentid_to_linkid_map[CMN700_MAX_CCG_PROTOCOL_LINKS];

    /*! SMP Mode */
    bool smp_mode;

    /*!
     * \brief CCLA to CCLA direct connect mode
     *
     * \details CCG enables direct connection of CXS interface from the CCLA on
     * one CMN‑700 to the CXS interface of the other. When such connection is
     * present, set this option to true in order to enable upper link layer to
     * upper link layer connection between CCLAs.
     */
    bool ull_to_ull_mode;

    /*! Port Aggregation Mode */
    bool port_aggregate;

    /*!
     * \brief Logical ID (LDID) of the CCG to which port aggregation pair to be
     *      created
     */
    unsigned int port_aggregate_ldid;

    /*! HAID of the CCG to which port aggregation pair to be created */
    unsigned int port_aggregate_haid;

    /*!
     * \brief Target HAID of remote CCG for CCRA SAM Address region for
     *      the port aggregated CCG block
     */
    unsigned int port_aggregate_remote_haid[CMN700_MAX_RA_SAM_ADDR_REGION];
};

/*!
 * \brief CMN700 configuration data
 */
struct mod_cmn700_config {
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

    /*! Hierarchical hashing support */
    bool hierarchical_hashing_enable;

    /*! Hierarchical hashing configuration */
    struct mod_cmn700_hierarchical_hashing hierarchical_hashing_config;

    /*! Table of region memory map entries */
    const struct mod_cmn700_mem_region_map *mmap_table;

    /*! Number of entries in the \ref mmap_table */
    size_t mmap_count;

    /*! Table of CCG configuration */
    const struct mod_cmn700_ccg_config *ccg_config_table;

    /*!
     * \brief Number of entries in the
     *      ::mod_cmn700_config::ccg_config_table table.
     */
    const size_t ccg_table_count;

    /*! Address space size of the chip */
    uint64_t chip_addr_space;

    /*! Identifier of the clock that this device depends on */
    fwk_id_t clock_id;

    /*!
     * \brief HN-F with CAL support flag
     * \details When set to true, enables HN-F with CAL support. This flag will
     * be used only if HN-F is found to be connected to CAL (When connected to
     * a CAL port, node id of HN-F will be a odd number).
     */
    bool hnf_cal_mode;
};

/*!
 * \brief Module API indices
 */
enum mod_cmn700_api_idx {
    MOD_CMN700_API_IDX_MAP_IO_REGION,
    MOD_CMN700_API_COUNT,
};

/*!
 * \brief Module interface to manage mappings in RN-SAM
 */
struct mod_cmn700_memmap_rnsam_api {
    /*!
     * \brief Program or update the given IO memory carveout in the RN-SAM of
     *        all the nodes
     * \param base Base address of the carveout to be mapped
     * \param size Size of the carveout
     * \param node_id Target node id to which carveout will be mapped
     *
     * \return FWK_SUCCESS on successfully mapping the region
     * \return FWK_E_DATA if mapping region is invalid
     */
    int (*map_io_region)(uint64_t base, size_t size, uint32_t node_id);
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_CMN700_H */

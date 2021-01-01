/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_STATISTICS_H
#define MOD_STATISTICS_H

#include <fwk_attributes.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \ingroup GroupModules
 * \defgroup GroupStatistics Statistics for Performance and Power domains
 *      of operating level changes
 * \{
 */

/*!
 * \defgroup GroupStatisticsTypes Types
 * \{
 */

/*!
 * \brief Statistics memory region information.
 */
struct FWK_PACKED mod_stats_config_info {
    /*! AP physical address where statistics are located */
    uint64_t ap_stats_addr;

    /*! SCP physical address where statistics are located */
    uintptr_t scp_stats_addr;

    /*! Size in bytes of the shared memory region for statistics */
    uint32_t stats_region_size;

    /*! Alarm used for period updates */
    fwk_id_t alarm_id;
};

/*!
 * \brief Domain statistics mapping structure.
 */
struct mod_stats_map {
    /*! Array which contains values with levels that each domain has.
     * We do not rely on domain_section->level_count in SCP address or
     * offset calculation, since it can be modified by OSPM */
    int *se_level_count;

    /*! Array which contains values with current level for each domain.
     * We do not rely on domain_section->curr_level in SCP address or
     * offset calculation, since it can be modified by OSPM */
    uint32_t *se_curr_level;

    /*! An array of pointers to the domain statistics table. Every domain
     * has its own table when statistics collection is set for it. */
    struct mod_stats_domain_stats_data *se_stats[];
};

/*!
 * \brief Domain statistics management structure.
 */
struct mod_stats_context {
    /*! Number of all stats entities of this type in the system (e.g. power
     * domains, performance domains, etc) */
    int se_total_num;

    /*! Number of stats entities of this type for which the statistics are
     * collected */
    int se_used_num;

    /*! Index used for mapping incremented after adding a new domain. */
    int last_stats_id;

    /*! Array to map system entity id to index used in statistics memory
     * allocator. It's needed because statistics are not enabled for all
     * entities */
    int *se_index_map;

    /*! Pointer to performance or power domain statistics mapping. Every
     * domain has its own table when statistics collection is set for it. */
    struct mod_stats_map *se_stats_map;
};

/*!
 * \brief Statistics status and mode enum.
 */
enum mod_stats_mode {
    STATS_NOT_INITIALIZED, /*!< Module not initialized */
    STATS_SETUP, /*!< Module in setup phase, not ready to use */
    STATS_INITIALIZED, /*!< Module ready to use */
    STATS_NOT_SUPPORTED, /*!< Statistics are not supported */
    STATS_INTERNAL_ERROR, /*!< Module reached an error */
    STATS_STOPPED, /*!< Statistics tracking stopped */
};

/*!
 * \brief Statistics context.
 */
struct mod_stats_info {
    /*! type_signature - private copy which should be the same as in shared
     * memory description header describing type of statistics region. */
    uint32_t type_signature;

    /*! Pointer to the shared memory header location */
    struct mod_stats_desc_header *desc_header;

    /*! Size of the shared memory header */
    unsigned int desc_header_size;

    /*! Size of unused shared memory region provided by the platform, which
     * might be evaluated and adjusted to actual need */
    unsigned int used_mem_size;

    /*! Offset inside the shared memory region to the header */
    uintptr_t desc_header_offset;

    /*! Pointer to the structure maintaining internal data, allocations
     * and sections of the entity (power domain, performance domain)
     * statistics */
    struct mod_stats_context *context;

    /*! Mode of operation for the statistics module */
    enum mod_stats_mode mode;
};

/*!
 * \brief Performance or power statistics memory region as defined in SCMIv2
 *      specification
 */
struct mod_stats_desc_header {
    /*! Signature - 0x50455246 (‘PERF’) or 0x504F5752 ('POWR'). */
    uint32_t signature;

    /*! The revision value aligned with the SCMI specification. */
    uint16_t revision;

    /*! The attributes aligned with the SCMI specification. */
    uint16_t attributes;

    /*! Total number of all performance or power domains in the system, which
     * also corresponds to the size of 'domain_offset' array size. */
    uint16_t domain_count;

    /*! Empty space just for memory alignment as per SCMI specification. */
    uint16_t reserved[3];

    /*! For each domain this array provides 4B offset from start addr of the
     * statistics memory region to the particular performance or power domain
     * statistics data section. If the value is 0 then statistics are not
     * collected for this domain. */
    uint32_t domain_offset[];
};

/*!
 * \brief Performance or power level statistics data
 *
 * \note In case of power domain. Here the reference to level means power state
 */
struct FWK_PACKED mod_stats_level_stats {
    /*! The performance or power level ID. */
    uint32_t level_id;

    /*! Reserved field as per SCMI specification. */
    uint32_t reserved;

    /*! Maintains the number of how many time this level has been used. */
    uint64_t usage_count;

    /*! Cumulative time of how long this level has been used. Value is in
     * microseconds. */
    uint64_t total_residency_us;
};

/*!
 * \brief Performance or power domain statistics data
 */
struct FWK_PACKED mod_stats_domain_stats_data {
    /*! Holds value of total number of performance or power levels available
     * in the domain. */
    uint16_t level_count;

    /*! Holds current Operating Performance Point or power level ID. */
    uint16_t curr_level_id;

    /*! An offset to the extended statistics area if they are present. */
    uint32_t extended_stats_offset;

    /*! Holds time stamp when the last performance or power level has been
     * changed. Value is in microseconds.*/
    uint64_t ts_last_change_us;

    /*! Beginning of the statistics region with information for each
     * performance or power level. */
    struct mod_stats_level_stats level[];
};

/*!
 * \}
 */


/*!
 * \defgroup GroupStatsApis APIs
 * \{
 */

/*!
 * \brief Stats API.
 */
struct mod_stats_api {
    /*!
     * \brief Initialize statistics for a given module.
     *
     * \param module_id Element identifier of the module.
     * \param domain_count Total number of domains in this module.
     * \param used_domains Total number of domains that have statistics.
     */
    int (*init_stats)(
        fwk_id_t module_id,
        int domain_count,
        int used_domains);

    /*!
     * \brief Start the statistics for the given module.
     *
     * \param module_id Element identifier of the module.
     */
    int (*start_stats)(fwk_id_t module_id);

    /*!
     * \brief Add new domain to the statistics of the parent module.
     *
     * \param module_id Element identifier of the module.
     * \param domain_id Element identifier of the domain.
     * \param level_count Total number of levels in this domain.
     */
    int (*add_domain)(
        fwk_id_t module_id,
        fwk_id_t domain_id,
        int level_count);

    /*!
     * \brief Update the domain statistics with new level ID set.
     *
     * \param module_id Element identifier of the module.
     * \param domain_id Element identifier of the domain.
     * \param level_id Current operating level ID.
     */
    int (*update_domain)(
        fwk_id_t module_id,
        fwk_id_t domain_id,
        uint32_t level_id);

    /*!
     * \brief Get low and high addresses of statistics in AP address space
     *          with length of the memory region
     *
     * \param module_id Element identifier of the module.
     * \param [out] addr_low lower 32 bits of the statistics address
     * \param [out] addr_high higher 32 bits of the statistics address
     * \param [out] length of the statistics memory region
     */
    int (*get_statistics_desc)(
        fwk_id_t module_id,
        uint32_t *addr_low,
        uint32_t *addr_high,
        uint32_t *len);
};
/*!
 * \}
 */
/*!
 * \defgroup GroupStatisticsIds Identifiers
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_stats_api_idx {
    MOD_STATS_API_IDX_STATS, /*!< API index for mod_stats_api_id_stats() */
    MOD_STATS_API_IDX_COUNT /*!< Number of defined APIs */
};

/*! Module API identifier */
static const fwk_id_t mod_stats_api_id_stats =
    FWK_ID_API_INIT(FWK_MODULE_IDX_STATISTICS, MOD_STATS_API_IDX_STATS);

/*!
 * \}
 */

/*!
 * \}
 */

#endif

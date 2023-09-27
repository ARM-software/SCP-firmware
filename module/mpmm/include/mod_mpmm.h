/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MPMM_H
#define MOD_MPMM_H

#include <fwk_id.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules
 *  \defgroup GroupMPMM Max Power Mitigation Mechanism (MPMM)
 * \{
 */

/*! Maximum number of thresholds allowed. */
#define MPMM_MAX_THRESHOLD_COUNT 8

/*! How many bits for each threshold in the PCT. */
#define MPMM_THRESHOLD_MAP_NUM_OF_BITS 4

/*! Maximum number of supported cores per domain. */
#define MPMM_MAX_NUM_CORES_IN_DOMAIN 8

/*!
 * \brief MPMM threshold to maximum performance mapping.
 */
struct mod_mpmm_threshold_perf {
    /*!
     * \brief Cores bitmap representing thresholds states.
     *
     * \details The threshold bitmap is a description of the threshold status of
     *      each online core. Each core is assigned 4-bits and a maximum of 8
     *      cores per domain is allowed. The bitmap must be arranged in
     *      numerically descending order. The lowest threshold is the least
     *      significant 4-bits. The next lowest threshold should be placed in
     *      the next lowest 4-bits. There is no relation between the core index
     *      number and the position of the threshold in this bitmap.
     *
     *      Example for a system with four cores:
     *
     *      4-cores online                  3-cores online
     *      +------+------+                 +------+------+
     *      | Core | thrd |                 | Core | thrd |
     *      +------+------+                 +------+------+
     *      |   0  |   2  |                 |   0  |   2  |
     *      |   1  |   3  |                 |   1  |   3  |
     *      |   2  |   0  |                 |   -  |   -  |
     *      |   3  |   2  |                 |   3  |   2  |
     *      +------+------+                 +------+------+
     *      bitmap = 0x3220                 bitmap = 0x0322
     *      (Note the numerically descending order arrangement.)
     */
    uint32_t threshold_bitmap;

    /*!
     * \brief Maximum allowed performance level for the thresholds states.
     *
     * \details The performance limit required when the cores are in this
     *      particular threshold_bitmap state.
     */
    uint32_t perf_limit;
};

/*!
 * \brief Perf Constraint Lookup Table (PCT) entry.
 *
 * \details The table entries should be provided in numerically descending
 *          order with respect to the number of online cores. The first
 *          entry must represent the maximum number of cores.
 */
struct mod_mpmm_pct_table {
    /*! Number of cores online. */
    uint32_t cores_online;

    /*!
     * \brief Default maximum performance level for the number of cores online
     *      given by cores_online.
     *
     * \details This performance limit will be selected if no suitable value is
     *      found in the threshold_perf table.
     */
    uint32_t default_perf_limit;

    /*! Number of performance levels entries in the threshold_perf table. */
    uint32_t num_perf_limits;

    /*!
     * \brief Array of threshold_map and performance level pairs.
     *
     * \details The threshold_map entries are provided in numerically descending
     *      order at every performance level.
     */
    struct mod_mpmm_threshold_perf threshold_perf[MPMM_MAX_NUM_CORES_IN_DOMAIN];
};

/*!
 * \brief MPMM sub-element configuration.
 *
 * \details The configuration data of each core.
 */
struct mod_mpmm_core_config {
    /*! Identifier of the power domain associated with each core. */
    fwk_id_t pd_id;

    /*! Base address of the MPMM registers */
    uintptr_t mpmm_reg_base;

    /*! Core initial power state when the platfrom starts is ON. */
    bool core_starts_online;

    /*! Identifier of the base AMU Auxiliry counter. */
    fwk_id_t base_aux_counter_id;
};

/*!
 * \brief MPMM domain configuration.
 *
 */
struct mod_mpmm_domain_config {
    /*! List of core configurations. */
    struct mod_mpmm_core_config const *core_config;

    /*! Perf Constraint Table (PCT) for each domain. */
    struct mod_mpmm_pct_table *pct;

    /*! Size of the Perf Constraint Table in bytes. */
    size_t pct_size;

    /*! Base throttling Count */
    uint64_t btc;

    /*! Number of threshold counters */
    uint32_t num_threshold_counters;

    /*! Identifier of the performance domain associated with mpmm domain. */
    fwk_id_t perf_id;
};

/*!
 * \}
 */

#endif /* MOD_MPMM_H */

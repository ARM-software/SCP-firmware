/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_TRAFFIC_COP_H
#define MOD_TRAFFIC_COP_H

#include <fwk_id.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules
 *  \defgroup GroupTrafficCop Traffic Cop
 * \{
 */

/*!
 * \brief Perf Constraint Lookup Table (PCT) entry.
 *
 * \details The table entries should be provided in numerically descending
 *          order with respect to the number of online cores. The first
 *          entry must represent the maximum number of cores.
 */
struct mod_tcop_pct_table {
    /*! Number of cores online. */
    uint32_t cores_online;

    /*! Maximum allowed performance level for this number of online cores. */
    uint32_t perf_limit;
};

/*!
 * \brief Traffic Cop sub-element configuration.
 *
 * \details The configuration data of each core.
 */
struct mod_tcop_core_config {
    /*! Identifier of the power domain associated with each core. */
    fwk_id_t pd_id;

    /*! Core initial power state when the platfrom starts is ON. */
    bool core_starts_online;
};

/*!
 * \brief Traffic Cop domain configuration.
 *
 */
struct mod_tcop_domain_config {
    /*! Identifier of the performance domain associated with tcop domain. */
    fwk_id_t perf_id;

    /*! Perf Constraint Table (PCT) for each domain. */
    struct mod_tcop_pct_table *pct;

    /*! Size of the Perf Constraint Table in bytes. */
    size_t pct_size;

    /*! List of core configurations. */
    struct mod_tcop_core_config const *core_config;
};

/*!
 * \}
 */

#endif /* MOD_TRAFFC_COP_H */

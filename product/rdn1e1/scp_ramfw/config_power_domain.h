/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_POWER_DOMAIN_H
#define CONFIG_POWER_DOMAIN_H

/*
 * Power domain indices for the statically defined domains used for:
 * - Indexing the domains in the rdn1e1_power_domain_static_element_table
 * - Indexing the SYSTOP children in the power domain tree
 *
 * When calculating a power domain element index, use the formula:
 * core_count + pd_static_dev_idx
 */
enum pd_static_dev_idx {
    PD_STATIC_DEV_IDX_CLUSTER0,
    PD_STATIC_DEV_IDX_CLUSTER1,
    PD_STATIC_DEV_IDX_DBGTOP,
    PD_STATIC_DEV_IDX_SYSTOP,
    PD_STATIC_DEV_IDX_COUNT
};

#endif /* CONFIG_POWER_DOMAIN_H */

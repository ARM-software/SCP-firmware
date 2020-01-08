/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_POWER_DOMAIN_H
#define CONFIG_POWER_DOMAIN_H

/*
 * Power domain indices for the statically defined domains used for:
 * - Indexing the domains in the rddaniel_power_domain_static_element_table
 * - Indexing the SYSTOP children in the power domain tree
 *
 * When calculating a power domain element index, use the formula:
 * core_count + pd_static_dev_idx
 */
enum pd_static_dev_idx {
    PD_STATIC_DEV_IDX_CLUSTER0,
    PD_STATIC_DEV_IDX_CLUSTER1,
    PD_STATIC_DEV_IDX_CLUSTER2,
    PD_STATIC_DEV_IDX_CLUSTER3,
    PD_STATIC_DEV_IDX_CLUSTER4,
    PD_STATIC_DEV_IDX_CLUSTER5,
    PD_STATIC_DEV_IDX_CLUSTER6,
    PD_STATIC_DEV_IDX_CLUSTER7,
    PD_STATIC_DEV_IDX_CLUSTER8,
    PD_STATIC_DEV_IDX_CLUSTER9,
    PD_STATIC_DEV_IDX_CLUSTER10,
    PD_STATIC_DEV_IDX_CLUSTER11,
    PD_STATIC_DEV_IDX_CLUSTER12,
    PD_STATIC_DEV_IDX_CLUSTER13,
    PD_STATIC_DEV_IDX_CLUSTER14,
    PD_STATIC_DEV_IDX_CLUSTER15,
    PD_STATIC_DEV_IDX_SYSTOP,
    PD_STATIC_DEV_IDX_COUNT
};

#endif /* CONFIG_POWER_DOMAIN_H */

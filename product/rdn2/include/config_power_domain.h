/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_POWER_DOMAIN_H
#define CONFIG_POWER_DOMAIN_H

#include <stdint.h>

/*
 * Power domain indices for the statically defined domains used for:
 * - Indexing the domains in the platform_power_domain_static_element_table
 * - Indexing the SYSTOP children in the power domain tree
 *
 * When calculating a power domain element index, use the formula:
 * core_count + cluster_count + pd_static_dev_idx
 */
enum pd_static_dev_idx {
    PD_STATIC_DEV_IDX_SYSTOP,
    PD_STATIC_DEV_IDX_NONE = UINT32_MAX
};

#endif /* CONFIG_POWER_DOMAIN_H */

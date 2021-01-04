/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TC0_CORE_H
#define TC0_CORE_H

#include <fwk_assert.h>

#define TC0_CORE_PER_CLUSTER_MAX 8

#define CORES_PER_CLUSTER  8
#define NUMBER_OF_CLUSTERS 1

static inline unsigned int tc0_core_get_cluster_count(void)
{
    return NUMBER_OF_CLUSTERS;
}

static inline unsigned int tc0_core_get_core_per_cluster_count(
    unsigned int cluster)
{
    fwk_assert(cluster < tc0_core_get_cluster_count());

    return CORES_PER_CLUSTER;
}

static inline unsigned int tc0_core_get_core_count(void)
{
    return tc0_core_get_core_per_cluster_count(0) *
        tc0_core_get_cluster_count();
}

#endif /* TC0_CORE_H */

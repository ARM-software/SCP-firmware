/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TC2_CORE_H
#define TC2_CORE_H

#include <fwk_assert.h>

#define CORES_PER_CLUSTER  8
#define NUMBER_OF_CLUSTERS 1

static inline unsigned int tc2_core_get_cluster_count(void)
{
    return NUMBER_OF_CLUSTERS;
}

static inline unsigned int tc2_core_get_core_per_cluster_count(
    unsigned int cluster_idx)
{
    fwk_assert(cluster_idx < tc2_core_get_cluster_count());

    return CORES_PER_CLUSTER;
}

static inline unsigned int tc2_core_get_core_count(void)
{
    return tc2_core_get_core_per_cluster_count(0) *
        tc2_core_get_cluster_count();
}

#endif /* TC2_CORE_H */

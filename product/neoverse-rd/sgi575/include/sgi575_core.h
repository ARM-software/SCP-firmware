/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SGI575_CORE_H
#define SGI575_CORE_H

#include <fwk_assert.h>

/* SGI575 only has one configuration, hence the constant values */

#define SGI575_CLUSTERS_MAX          2
#define SGI575_CORES_PER_CLUSTER_MAX 4

static inline unsigned int sgi575_core_get_cluster_count(void)
{
    return SGI575_CLUSTERS_MAX;
}

static inline unsigned int sgi575_core_get_core_per_cluster_count(
    unsigned int cluster)
{
    fwk_assert(cluster < sgi575_core_get_cluster_count());

    return SGI575_CORES_PER_CLUSTER_MAX;
}

static inline unsigned int sgi575_core_get_core_count(void)
{
    return SGI575_CLUSTERS_MAX * SGI575_CORES_PER_CLUSTER_MAX;
}

#endif /* SGI575_CORE_H */

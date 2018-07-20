/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <sgi575_core.h>

/* SGI575 only has one configuration, hence the constant values */

#define CORES_PER_CLUSTER 4
#define NUMBER_OF_CLUSTERS 2

unsigned int sgi575_core_get_core_per_cluster_count(unsigned int cluster)
{
    assert(cluster < sgi575_core_get_cluster_count());

    return CORES_PER_CLUSTER;
}

unsigned int sgi575_core_get_core_count(void)
{
    return NUMBER_OF_CLUSTERS * CORES_PER_CLUSTER;
}

unsigned int sgi575_core_get_cluster_count(void)
{
    return NUMBER_OF_CLUSTERS;
}

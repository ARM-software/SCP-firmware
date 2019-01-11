/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CLARK_CORE_H
#define CLARK_CORE_H

#include <fwk_assert.h>

#define CLARK_CORE_PER_CLUSTER_MAX 8

/* Clark only has one configuration, hence the constant values */
#define CORES_PER_CLUSTER   4
#define NUMBER_OF_CLUSTERS  2

static inline unsigned int clark_core_get_cluster_count(void)
{
    return NUMBER_OF_CLUSTERS;
}

static inline unsigned int clark_core_get_core_per_cluster_count(
    unsigned int cluster)
{
    fwk_assert(cluster < clark_core_get_cluster_count());

    return CORES_PER_CLUSTER;
}

static inline unsigned int clark_core_get_core_count(void)
{
    return NUMBER_OF_CLUSTERS * CORES_PER_CLUSTER;
}

#endif /* CLARK_CORE_H */

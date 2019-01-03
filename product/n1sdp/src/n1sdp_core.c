/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <n1sdp_core.h>

unsigned int n1sdp_core_get_core_per_cluster_count(unsigned int cluster)
{
    assert(cluster < n1sdp_core_get_cluster_count());

    return CORES_PER_CLUSTER;
}

unsigned int n1sdp_core_get_core_count(void)
{
    return (NUMBER_OF_CLUSTERS * CORES_PER_CLUSTER);
}

unsigned int n1sdp_core_get_cluster_count(void)
{
    return NUMBER_OF_CLUSTERS;
}

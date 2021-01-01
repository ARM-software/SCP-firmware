/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "morello_core.h"
#include "morello_scc_reg.h"
#include "morello_scp_pik.h"

#include <fwk_assert.h>

unsigned int morello_core_get_core_per_cluster_count(unsigned int cluster)
{
    assert(cluster < morello_core_get_cluster_count());

    return CORES_PER_CLUSTER;
}

unsigned int morello_core_get_core_count(void)
{
    return (NUMBER_OF_CLUSTERS * CORES_PER_CLUSTER);
}

unsigned int morello_core_get_cluster_count(void)
{
    return NUMBER_OF_CLUSTERS;
}

bool morello_is_multichip_enabled(void)
{
    return (
        (SCC->PLATFORM_CTRL & SCC_PLATFORM_CTRL_MULTI_CHIP_MODE_MASK) >>
        SCC_PLATFORM_CTRL_MULTI_CHIP_MODE_POS);
}

uint8_t morello_get_chipid(void)
{
    return ((uint8_t)(
        (SCC->PLATFORM_CTRL & SCC_PLATFORM_CTRL_CHIPID_MASK) >>
        SCC_PLATFORM_CTRL_CHIPID_POS));
}

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <synquacer_core.h>

uint32_t synquacer_core_get_core_count(void)
{
    return SYNQUACER_CSS_CPUS_MAX;
}

uint32_t synquacer_core_get_cluster_count(void)
{
    return SYNQUACER_CSS_CPUS_CLUSTER_MAX;
}

uint32_t synquacer_core_get_core_per_cluster_count(void)
{
    return SYNQUACER_CSS_CPUS_PER_CLUSTER_MAX;
}

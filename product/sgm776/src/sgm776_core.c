/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sgm776_core.h"
#include "sgm776_pik.h"
#include "sgm776_pik_cpu.h"

unsigned int sgm776_core_get_count(void)
{
    return (PIK_CLUS0->PCL_CONFIG & PIK_CPU_PCL_CONFIG_NO_OF_PPU) - 1;
}

unsigned int sgm776_cluster_get_count(void)
{
    return SGM776_CLUSTER_COUNT;
}

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sgm775_core.h"
#include "sgm775_pik.h"
#include "sgm775_pik_cpu.h"

unsigned int sgm775_core_get_count(void)
{
    return (PIK_CLUS0->PIK_CONFIG & PIK_CPU_V8_2_PIK_CONFIG_NO_OF_PPU) - 1;
}

unsigned int sgm775_cluster_get_count(void)
{
    return SGM775_CLUSTER_COUNT;
}

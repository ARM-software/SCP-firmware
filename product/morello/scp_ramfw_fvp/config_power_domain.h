/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_POWER_DOMAIN_H
#define CONFIG_POWER_DOMAIN_H

#include <stdint.h>

/*
 * Power domain indices in single chip use case
 */
enum pd_single_chip_idx {
    PD_SINGLE_CHIP_IDX_CLUS0CORE0,
    PD_SINGLE_CHIP_IDX_CLUS0CORE1,
    PD_SINGLE_CHIP_IDX_CLUS1CORE0,
    PD_SINGLE_CHIP_IDX_CLUS1CORE1,
    PD_SINGLE_CHIP_IDX_CLUSTER0,
    PD_SINGLE_CHIP_IDX_CLUSTER1,
    PD_SINGLE_CHIP_IDX_DBGTOP0,
    PD_SINGLE_CHIP_IDX_SYSTOP0,
    PD_SINGLE_CHIP_IDX_COUNT,
    PD_SINGLE_CHIP_IDX_NONE = UINT32_MAX
};

#endif /* CONFIG_POWER_DOMAIN_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_POWER_DOMAIN_H
#define CONFIG_POWER_DOMAIN_H

/*
 * Total supported chips in multichip use case
 */
#define CHIP_COUNT 2

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
};

/*
 * Power domain indices in multi chip use case (with only master SYSTOP)
 * TBD: Use chip specific SYSTOPs (level 2) and logical SYSTOP (level 3)
 *      once level 3 handling is fixed in PD driver. Then PD tree view
 *      in master SCP will look like:
 *      enum pd_multi_chip_idx {
 *          PD_MULTI_CHIP_IDX_CLUS0CORE0,
 *          PD_MULTI_CHIP_IDX_CLUS0CORE1,
 *          PD_MULTI_CHIP_IDX_CLUS1CORE0,
 *          PD_MULTI_CHIP_IDX_CLUS1CORE1,
 *          PD_MULTI_CHIP_IDX_CLUS2CORE0,
 *          PD_MULTI_CHIP_IDX_CLUS2CORE1,
 *          PD_MULTI_CHIP_IDX_CLUS3CORE0,
 *          PD_MULTI_CHIP_IDX_CLUS3CORE1,
 *          PD_MULTI_CHIP_IDX_CLUSTER0,
 *          PD_MULTI_CHIP_IDX_CLUSTER1,
 *          PD_MULTI_CHIP_IDX_DBGTOP0,
 *          PD_MULTI_CHIP_IDX_CLUSTER2,
 *          PD_MULTI_CHIP_IDX_CLUSTER3,
 *          PD_MULTI_CHIP_IDX_DBGTOP1,
 *          PD_MULTI_CHIP_IDX_SYSTOP0,
 *          PD_MULTI_CHIP_IDX_SYSTOP1,
 *          PD_MULTI_CHIP_IDX_SYSTOP_LOGICAL,
 *          PD_MULTI_CHIP_IDX_COUNT,
 *      };
 */
enum pd_multi_chip_idx {
    PD_MULTI_CHIP_IDX_CLUS0CORE0,
    PD_MULTI_CHIP_IDX_CLUS0CORE1,
    PD_MULTI_CHIP_IDX_CLUS1CORE0,
    PD_MULTI_CHIP_IDX_CLUS1CORE1,
    PD_MULTI_CHIP_IDX_CLUS2CORE0,
    PD_MULTI_CHIP_IDX_CLUS2CORE1,
    PD_MULTI_CHIP_IDX_CLUS3CORE0,
    PD_MULTI_CHIP_IDX_CLUS3CORE1,
    PD_MULTI_CHIP_IDX_CLUSTER0,
    PD_MULTI_CHIP_IDX_CLUSTER1,
    PD_MULTI_CHIP_IDX_CLUSTER2,
    PD_MULTI_CHIP_IDX_CLUSTER3,
    PD_MULTI_CHIP_IDX_DBGTOP0,
    PD_MULTI_CHIP_IDX_SYSTOP0,
    PD_MULTI_CHIP_IDX_COUNT,
};

#endif /* CONFIG_POWER_DOMAIN_H */

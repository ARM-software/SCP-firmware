/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_POWER_DOMAIN_H
#define CONFIG_POWER_DOMAIN_H

/*
 * Power domain element indexes defined in increasing order of power domain
 * level.
 */

enum power_domain_idx {
    /* Level 0 */
    POWER_DOMAIN_IDX_BIG_CPU0,
    POWER_DOMAIN_IDX_BIG_CPU1,
    POWER_DOMAIN_IDX_LITTLE_CPU0,
    POWER_DOMAIN_IDX_LITTLE_CPU1,
    POWER_DOMAIN_IDX_LITTLE_CPU2,
    POWER_DOMAIN_IDX_LITTLE_CPU3,

    /* Level 1 */
    POWER_DOMAIN_IDX_BIG_SSTOP,
    POWER_DOMAIN_IDX_LITTLE_SSTOP,
    POWER_DOMAIN_IDX_DBGSYS,
    POWER_DOMAIN_IDX_GPUTOP,

    /* Level 2 */
    POWER_DOMAIN_IDX_SYSTOP,

    /* Number of defined elements */
    POWER_DOMAIN_IDX_COUNT,
};

#endif /* CONFIG_POWER_DOMAIN_H */

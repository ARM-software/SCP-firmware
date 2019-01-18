/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     CLARK System Support
 */

#ifndef CLARK_POWER_DOMAIN_H
#define CLARK_POWER_DOMAIN_H

#include <mod_power_domain.h>

/*! Additional CLARK power domain states */
enum clark_power_domain_states {
    CLARK_POWER_DOMAIN_STATE_FUNC_RET = MOD_PD_STATE_COUNT,
    CLARK_POWER_DOMAIN_STATE_FULL_RET,
    CLARK_POWER_DOMAIN_STATE_MEM_RET,
    CLARK_POWER_DOMAIN_STATE_COUNT
};

/*! Power domain state masks */
enum clark_power_domain_state_masks {
    CLARK_POWER_DOMAIN_STATE_FUNC_RET_MASK =
        (1 << CLARK_POWER_DOMAIN_STATE_FUNC_RET),
    CLARK_POWER_DOMAIN_STATE_FULL_RET_MASK =
        (1 << CLARK_POWER_DOMAIN_STATE_FULL_RET),
    CLARK_POWER_DOMAIN_STATE_MEM_RET_MASK =
        (1 << CLARK_POWER_DOMAIN_STATE_MEM_RET),
};

/*! Mask for the cluster valid power states */
#define CLARK_CLUSTER_VALID_STATE_MASK ( \
                                MOD_PD_STATE_OFF_MASK | \
                                MOD_PD_STATE_ON_MASK | \
                                MOD_PD_STATE_SLEEP_MASK | \
                                CLARK_POWER_DOMAIN_STATE_MEM_RET_MASK | \
                                CLARK_POWER_DOMAIN_STATE_FUNC_RET_MASK \
                                )

/*! Mask for the core valid power states */
#define CLARK_CORE_VALID_STATE_MASK ( \
                                MOD_PD_STATE_OFF_MASK | \
                                MOD_PD_STATE_ON_MASK | \
                                MOD_PD_STATE_SLEEP_MASK | \
                                CLARK_POWER_DOMAIN_STATE_FULL_RET_MASK \
                                )

#endif /* CLARK_POWER_DOMAIN_H */

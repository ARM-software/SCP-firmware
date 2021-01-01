/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     MORELLO power domain header file.
 */

#ifndef MORELLO_POWER_DOMAIN_H
#define MORELLO_POWER_DOMAIN_H

#include <mod_power_domain.h>

/*! Additional MORELLO power domain states */
enum morello_power_domain_states {
    MORELLO_POWER_DOMAIN_STATE_FUNC_RET = MOD_PD_STATE_COUNT,
    MORELLO_POWER_DOMAIN_STATE_FULL_RET,
    MORELLO_POWER_DOMAIN_STATE_MEM_RET,
    MORELLO_POWER_DOMAIN_STATE_COUNT
};

/*! Power domain state masks */
enum morello_power_domain_state_masks {
    MORELLO_POWER_DOMAIN_STATE_FUNC_RET_MASK =
        (1 << MORELLO_POWER_DOMAIN_STATE_FUNC_RET),
    MORELLO_POWER_DOMAIN_STATE_FULL_RET_MASK =
        (1 << MORELLO_POWER_DOMAIN_STATE_FULL_RET),
    MORELLO_POWER_DOMAIN_STATE_MEM_RET_MASK =
        (1 << MORELLO_POWER_DOMAIN_STATE_MEM_RET),
};

/*! Mask for the cluster valid power states */
#define MORELLO_CLUSTER_VALID_STATE_MASK \
    (MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK | MOD_PD_STATE_SLEEP_MASK | \
     MORELLO_POWER_DOMAIN_STATE_MEM_RET_MASK | \
     MORELLO_POWER_DOMAIN_STATE_FUNC_RET_MASK)

/*! Mask for the core valid power states */
#define MORELLO_CORE_VALID_STATE_MASK \
    (MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK | MOD_PD_STATE_SLEEP_MASK | \
     MORELLO_POWER_DOMAIN_STATE_FULL_RET_MASK)

#endif /* MORELLO_POWER_DOMAIN_H */

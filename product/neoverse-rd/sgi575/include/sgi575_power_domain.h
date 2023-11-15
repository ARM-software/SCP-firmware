/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SGI575 System Support
 */

#ifndef SGI575_POWER_DOMAIN_H
#define SGI575_POWER_DOMAIN_H

#include <mod_power_domain.h>

/*! Additional SGI575 power domain states */
enum sgi575_power_domain_states {
    SGI575_POWER_DOMAIN_STATE_FUNC_RET = MOD_PD_STATE_COUNT,
    SGI575_POWER_DOMAIN_STATE_FULL_RET,
    SGI575_POWER_DOMAIN_STATE_MEM_RET,
    SGI575_POWER_DOMAIN_STATE_COUNT
};

/*! Power domain state masks */
enum sgi575_power_domain_state_masks {
    SGI575_POWER_DOMAIN_STATE_FUNC_RET_MASK =
        (1 << SGI575_POWER_DOMAIN_STATE_FUNC_RET),
    SGI575_POWER_DOMAIN_STATE_FULL_RET_MASK =
        (1 << SGI575_POWER_DOMAIN_STATE_FULL_RET),
    SGI575_POWER_DOMAIN_STATE_MEM_RET_MASK =
        (1 << SGI575_POWER_DOMAIN_STATE_MEM_RET),
};

/*! Mask for the cluster valid power states */
#define SGI575_CLUSTER_VALID_STATE_MASK \
    (MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK | MOD_PD_STATE_SLEEP_MASK | \
     SGI575_POWER_DOMAIN_STATE_MEM_RET_MASK | \
     SGI575_POWER_DOMAIN_STATE_FUNC_RET_MASK)

/*! Mask for the core valid power states */
#define SGI575_CORE_VALID_STATE_MASK \
    (MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK | MOD_PD_STATE_SLEEP_MASK | \
     SGI575_POWER_DOMAIN_STATE_FULL_RET_MASK)

#endif /* SGI575_POWER_DOMAIN_H */

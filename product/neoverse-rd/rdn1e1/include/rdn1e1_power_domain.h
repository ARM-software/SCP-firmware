/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     RDN1E1 System Support
 */

#ifndef RDN1E1_POWER_DOMAIN_H
#define RDN1E1_POWER_DOMAIN_H

#include <mod_power_domain.h>

/*! Additional RDN1E1 power domain states */
enum rdn1e1_power_domain_states {
    RDN1E1_POWER_DOMAIN_STATE_FUNC_RET = MOD_PD_STATE_COUNT,
    RDN1E1_POWER_DOMAIN_STATE_FULL_RET,
    RDN1E1_POWER_DOMAIN_STATE_MEM_RET,
    RDN1E1_POWER_DOMAIN_STATE_COUNT
};

/*! Power domain state masks */
enum rdn1e1_power_domain_state_masks {
    RDN1E1_POWER_DOMAIN_STATE_FUNC_RET_MASK =
        (1 << RDN1E1_POWER_DOMAIN_STATE_FUNC_RET),
    RDN1E1_POWER_DOMAIN_STATE_FULL_RET_MASK =
        (1 << RDN1E1_POWER_DOMAIN_STATE_FULL_RET),
    RDN1E1_POWER_DOMAIN_STATE_MEM_RET_MASK =
        (1 << RDN1E1_POWER_DOMAIN_STATE_MEM_RET),
};

/*! Mask for the cluster valid power states */
#define RDN1E1_CLUSTER_VALID_STATE_MASK \
    (MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK | MOD_PD_STATE_SLEEP_MASK | \
     RDN1E1_POWER_DOMAIN_STATE_MEM_RET_MASK | \
     RDN1E1_POWER_DOMAIN_STATE_FUNC_RET_MASK)

/*! Mask for the core valid power states */
#define RDN1E1_CORE_VALID_STATE_MASK \
    (MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK | MOD_PD_STATE_SLEEP_MASK | \
     RDN1E1_POWER_DOMAIN_STATE_FULL_RET_MASK)

#endif /* RDN1E1_POWER_DOMAIN_H */

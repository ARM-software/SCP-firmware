/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     N1SDP power domain header file.
 */

#ifndef N1SDP_POWER_DOMAIN_H
#define N1SDP_POWER_DOMAIN_H

#include <mod_power_domain.h>

/*! Additional N1SDP power domain states */
enum n1sdp_power_domain_states {
    N1SDP_POWER_DOMAIN_STATE_FUNC_RET = MOD_PD_STATE_COUNT,
    N1SDP_POWER_DOMAIN_STATE_FULL_RET,
    N1SDP_POWER_DOMAIN_STATE_MEM_RET,
    N1SDP_POWER_DOMAIN_STATE_COUNT
};

/*! Power domain state masks */
enum n1sdp_power_domain_state_masks {
    N1SDP_POWER_DOMAIN_STATE_FUNC_RET_MASK =
        (1 << N1SDP_POWER_DOMAIN_STATE_FUNC_RET),
    N1SDP_POWER_DOMAIN_STATE_FULL_RET_MASK =
        (1 << N1SDP_POWER_DOMAIN_STATE_FULL_RET),
    N1SDP_POWER_DOMAIN_STATE_MEM_RET_MASK =
        (1 << N1SDP_POWER_DOMAIN_STATE_MEM_RET),
};

/*! Mask for the cluster valid power states */
#define N1SDP_CLUSTER_VALID_STATE_MASK ( \
                                MOD_PD_STATE_OFF_MASK | \
                                MOD_PD_STATE_ON_MASK | \
                                MOD_PD_STATE_SLEEP_MASK | \
                                N1SDP_POWER_DOMAIN_STATE_MEM_RET_MASK | \
                                N1SDP_POWER_DOMAIN_STATE_FUNC_RET_MASK \
                                )

/*! Mask for the core valid power states */
#define N1SDP_CORE_VALID_STATE_MASK ( \
                                MOD_PD_STATE_OFF_MASK | \
                                MOD_PD_STATE_ON_MASK | \
                                MOD_PD_STATE_SLEEP_MASK | \
                                N1SDP_POWER_DOMAIN_STATE_FULL_RET_MASK \
                                )

#endif /* N1SDP_POWER_DOMAIN_H */

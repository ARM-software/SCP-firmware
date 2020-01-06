/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno power domain header file.
 */

#ifndef JUNO_POWER_DOMAIN_H
#define JUNO_POWER_DOMAIN_H

#include <mod_power_domain.h>

/*!
 * \brief Identifiers of the additional Juno power domain states.
 */
enum juno_power_domain_states {
    JUNO_POWER_DOMAIN_MEM_RET = MOD_PD_STATE_COUNT,
    JUNO_POWER_DOMAIN_CSS_SLEEP0,
    JUNO_POWER_DOMAIN_CSS_SLEEP0_EMU,
    JUNO_POWER_DOMAIN_STATE_COUNT
};

/*!
 * \brief Masks for the additional Juno power domain states.
 */
enum juno_power_domain_state_mask {
    JUNO_POWER_DOMAIN_MEM_RET_MASK = (1 << JUNO_POWER_DOMAIN_MEM_RET),
    JUNO_POWER_DOMAIN_CSS_SLEEP0_MASK = (1 << JUNO_POWER_DOMAIN_CSS_SLEEP0),
    JUNO_POWER_DOMAIN_CSS_SLEEP0_EMU_MASK =
        (1 << JUNO_POWER_DOMAIN_CSS_SLEEP0_EMU),
};

/*! Mask for the core valid power states */
#define JUNO_CORE_VALID_STATE_MASK (MOD_PD_STATE_OFF_MASK | \
                                    MOD_PD_STATE_ON_MASK | \
                                    MOD_PD_STATE_SLEEP_MASK)

/*! Mask for the cluster valid power states */
#define JUNO_CLUSTER_VALID_STATE_MASK (MOD_PD_STATE_OFF_MASK | \
                                       MOD_PD_STATE_ON_MASK | \
                                       JUNO_POWER_DOMAIN_MEM_RET)

#endif /* JUNO_POWER_DOMAIN_H */

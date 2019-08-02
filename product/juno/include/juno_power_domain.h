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

/*! Mask for the core valid power states */
#define JUNO_CORE_VALID_STATE_MASK (MOD_PD_STATE_OFF_MASK | \
                                    MOD_PD_STATE_ON_MASK | \
                                    MOD_PD_STATE_SLEEP_MASK)

/*! Mask for the cluster valid power states */
#define JUNO_CLUSTER_VALID_STATE_MASK (MOD_PD_STATE_OFF_MASK | \
                                       MOD_PD_STATE_ON_MASK)

#endif /* JUNO_POWER_DOMAIN_H */

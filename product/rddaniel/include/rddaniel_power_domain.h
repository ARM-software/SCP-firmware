/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RDDANIEL_POWER_DOMAIN_H
#define RDDANIEL_POWER_DOMAIN_H

#include <mod_power_domain.h>

/*! Mask for the cluster valid power states */
#define RDDANIEL_CLUSTER_VALID_STATE_MASK ( \
                                MOD_PD_STATE_OFF_MASK | \
                                MOD_PD_STATE_ON_MASK \
                                )

/*! Mask for the core valid power states */
#define RDDANIEL_CORE_VALID_STATE_MASK ( \
                                MOD_PD_STATE_OFF_MASK | \
                                MOD_PD_STATE_ON_MASK  \
                                )

#endif /* RDDANIEL_POWER_DOMAIN_H */

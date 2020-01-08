/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RDDANIELXLR_POWER_DOMAIN_H
#define RDDANIELXLR_POWER_DOMAIN_H

#include <mod_power_domain.h>

/*! Mask for the cluster valid power states */
#define RDDANIELXLR_CLUSTER_VALID_STATE_MASK ( \
                                MOD_PD_STATE_OFF_MASK | \
                                MOD_PD_STATE_ON_MASK \
                                )

/*! Mask for the core valid power states */
#define RDDANIELXLR_CORE_VALID_STATE_MASK ( \
                                MOD_PD_STATE_OFF_MASK | \
                                MOD_PD_STATE_ON_MASK  \
                                )

#endif /* RDDANIELXLR_POWER_DOMAIN_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TC2_POWER_DOMAIN_H
#define TC2_POWER_DOMAIN_H

#include <mod_power_domain.h>

/*! Mask for the cluster valid power states */
#define TC2_CLUSTER_VALID_STATE_MASK \
    (MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK)

/*! Mask for the core valid power states */
#define TC2_CORE_VALID_STATE_MASK (MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK)

#endif /* TC2_POWER_DOMAIN_H */

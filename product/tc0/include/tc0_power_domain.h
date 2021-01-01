/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef TC0_POWER_DOMAIN_H
#define TC0_POWER_DOMAIN_H

#include <mod_power_domain.h>

/*! Mask for the cluster valid power states */
#define TC0_CLUSTER_VALID_STATE_MASK \
    (MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK)

/*! Mask for the core valid power states */
#define TC0_CORE_VALID_STATE_MASK (MOD_PD_STATE_OFF_MASK | MOD_PD_STATE_ON_MASK)

#endif /* TC0_POWER_DOMAIN_H */

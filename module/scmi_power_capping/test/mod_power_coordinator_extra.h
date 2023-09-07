/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_POWER_COORIDNATOR_EXTRA_H_
#    define MOD_POWER_COORDINATOR_EXTRA_H_

#    include "mod_power_coordinator.h"

int get_coordinator_period(fwk_id_t domain_id, uint32_t *period);
int set_coordinator_period(fwk_id_t domain_id, uint32_t period);
#endif /* MOD_POWER_COORDINATOR_EXTRA_H_ */

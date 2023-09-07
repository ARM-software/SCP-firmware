/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_POWER_METER_EXTRA_H_
#define MOD_POWER_METER_EXTRA_H_

#include "mod_power_meter.h"

int get_power(fwk_id_t domain_id, uint32_t *power);
#endif /* MOD_POWER_METER_EXTRA_H_ */

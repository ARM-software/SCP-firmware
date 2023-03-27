/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SCMI_EXTRA_H_
#define MOD_SCMI_EXTRA_H_

#include "fwk_id.h"

int get_agent_count(unsigned int *agent_count);
int get_agent_id(fwk_id_t service_id, unsigned int *agent_id);
int respond(fwk_id_t service_id, const void *payload, size_t size);

#endif /* MOD_SCMI_EXTRA_H_ */

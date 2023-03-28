/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RESOURCE_PERMS_EXTRA_H_
#define MOD_RESOURCE_PERMS_EXTRA_H_

#include <mod_resource_perms.h>

enum mod_res_perms_permissions agent_has_protocol_permission(
    uint32_t agent_id,
    uint32_t protocol_id);

enum mod_res_perms_permissions agent_has_resource_permission(
    uint32_t agent_id,
    uint32_t protocol_id,
    uint32_t message_id,
    uint32_t resource_id);

#endif /* MOD_RESOURCE_PERMS_EXTRA_H_ */

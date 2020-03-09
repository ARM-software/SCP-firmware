/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_scmi_system_power.h>
#include <mod_system_power.h>

#include <fwk_module.h>

#include <stddef.h>

struct fwk_module_config config_scmi_system_power = {
    .get_element_table = NULL,
    .data = &((struct mod_scmi_system_power_config) {
        .system_view = MOD_SCMI_SYSTEM_VIEW_FULL,
        .system_suspend_state = MOD_SYSTEM_POWER_POWER_STATE_SLEEP0
    }),
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <juno_alarm_idx.h>

#include <mod_scmi_system_power.h>
#include <mod_system_power.h>

#include <fwk_module.h>

struct fwk_module_config config_scmi_system_power = {
    .data =
        &(struct mod_scmi_system_power_config){
            .system_view = MOD_SCMI_SYSTEM_VIEW_FULL,
            .system_suspend_state =
                (unsigned int)MOD_SYSTEM_POWER_POWER_STATE_SLEEP0,
            .alarm_id = FWK_ID_SUB_ELEMENT_INIT(
                FWK_MODULE_IDX_TIMER,
                JUNO_ALARM_ELEMENT_IDX,
                JUNO_SYSTEM_POWER_ALARM_IDX),
            .graceful_timeout = 1000, /* ms */
        },
};

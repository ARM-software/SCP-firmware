/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Power unit test configuration.
 */

#include <scmi_system_power.h>

#include <mod_power_domain.h>
#include <mod_scmi.h>
#include <mod_scmi_system_power.h>

enum scp_sys_pow_nums {
    MOD_SCMI_SYS_POWER_IDX_0,
    MOD_SCMI_SYS_POWER_COUNT,
};

struct mod_scmi_system_power_config fake_config = {
    .alarm_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_TIMER, 0, 0),
    .graceful_timeout = 1000,
    .system_suspend_state = (unsigned int)MOD_PD_STATE_COUNT,
    .system_view = MOD_SCMI_SYSTEM_VIEW_OSPM,
};

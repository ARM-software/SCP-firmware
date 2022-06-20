/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Alarm indices */
enum morello_alarm_idx {
    MORELLO_PCIE_ALARM_IDX,
    MORELLO_SENSOR_ALARM_IDX,
    MORELLO_PSU_XR77128_CLUS0_ALARM_IDX,
    MORELLO_PSU_XR77128_CLUS1_ALARM_IDX,
#ifdef BUILD_HAS_DEBUGGER
    MORELLO_DEBUGGER_CLI_ALARM_IDX,
#endif
    MORELLO_ALARM_IDX_COUNT,
};

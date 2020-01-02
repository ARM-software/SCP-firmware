/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Timer sub-element (alarm) indices.
 */

#ifndef JUNO_ALARM_IDX_H
#define JUNO_ALARM_IDX_H

/* Alarm indices for XRP7724 */
enum juno_xrp7724_alarm_idx {
    JUNO_XRP7724_ALARM_IDX,
    JUNO_XRP7724_ALARM_IDX_COUNT,
};

/* Alarm indices for DVFS */
enum juno_dvfs_alarm_idx {
    JUNO_DVFS_ALARM_VLITTLE_IDX = JUNO_XRP7724_ALARM_IDX_COUNT,
    JUNO_DVFS_ALARM_BIG_IDX,
    JUNO_DVFS_ALARM_GPU_IDX,
    JUNO_DVFS_ALARM_IDX_CNT,
};

/* Total count of alarms */
#define JUNO_ALARM_IDX_COUNT    JUNO_DVFS_ALARM_IDX_CNT

#endif /* JUNO_ALARM_IDX_H */

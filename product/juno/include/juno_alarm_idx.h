/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
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

/* Total count of alarms */
#define JUNO_ALARM_IDX_COUNT    JUNO_XRP7724_ALARM_IDX_COUNT

#endif /* JUNO_ALARM_IDX_H */

/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RCAR_ALARM_IDX_H
#define RCAR_ALARM_IDX_H

#define RCAR_ALARM_ELEMENT_IDX 0

/* Alarm indices for DVFS */
enum rcar_dvfs_alarm_idx {
    RCAR_DVFS_ALARM_LITTLE_IDX = RCAR_ALARM_ELEMENT_IDX,
    RCAR_DVFS_ALARM_BIG_IDX,
    RCAR_DVFS_ALARM_IDX_CNT,
};

enum rcar_misc_alarm_idx {
    RCAR_PPU_ALARM_IDX = RCAR_DVFS_ALARM_IDX_CNT,
    RCAR_THERMAL_ALARM_IDX,
    RCAR_SYSTEM_POWER_ALARM_IDX,
    RCAR_ALARM_IDX_COUNT
};

#endif /* RCAR_ALARM_IDX_H */

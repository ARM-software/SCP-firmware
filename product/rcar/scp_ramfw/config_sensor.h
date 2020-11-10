/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2021, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_SENSOR_H
#define CONFIG_SENSOR_H

enum REG_SENSOR_DEVICES {
    REG_SENSOR_DEV_SOC_TEMP1,
    REG_SENSOR_DEV_SOC_TEMP2,
    REG_SENSOR_DEV_SOC_TEMP3,
    REG_SENSOR_DEV_COUNT,
};

enum SENSOR_DEVICES {
    R8A7795_SNSR_THERMAL1,
    R8A7795_SNSR_THERMAL2,
    R8A7795_SNSR_THERMAL3,
    R8A7795_SNSR_COUNT,
};
#endif /* CONFIG_SENSOR_H */

/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SYSTEM_MMAP_H
#define SYSTEM_MMAP_H

#include <rcar_mmap.h>

#define BOARD_UART1_BASE (PERIPHERAL_BASE + 0xe68000)
#define BOARD_UART2_BASE (PERIPHERAL_BASE + 0xe88000)

#define SENSOR_SOC_TEMP1 (PERIPHERAL_BASE + 0x198000)
#define SENSOR_SOC_TEMP2 (PERIPHERAL_BASE + 0x1a0000)
#define SENSOR_SOC_TEMP3 (PERIPHERAL_BASE + 0x1a8000)
#define SENSOR_SOC_TEMP (SENSOR_SOC_TEMP1)

#endif /* SYSTEM_MMAP_H */

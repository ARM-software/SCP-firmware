/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Sensor unit test configuration.
 */

#define SCMI_SENSOR_ELEMENT_INDEX_ZERO   0
#define SCMI_SENSOR_ELEMENT_COUNT_SINGLE 1

#define SCMI_SENSOR_ELEMENT_COUNT_MAXIMUM 0xFFFFU
#define SCMI_SENSOR_ELEMENT_COUNT_OVER_SIZE \
    (SCMI_SENSOR_ELEMENT_COUNT_MAXIMUM + 1)

enum { SCMI_SENSOR_FAKE_INDEX_0 };

#define SCMI_SENSOR_TRIP_COUNT_0 0
#define SCMI_SENSOR_TRIP_COUNT_1 1

#define SCMI_SENSOR_ROUND_0 0
#define SCMI_SENSOR_ROUND_1 1

#define SCMI_SENSOR_OPERATIONS 2
#define SCMI_SENSOR_VALUES     2

#define LONG_SENSOR_VALUE 0x1111222233334444

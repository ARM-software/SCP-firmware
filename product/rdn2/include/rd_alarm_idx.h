/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Alarm indices */
enum rd_alarm_idx {
#ifdef BUILD_HAS_FAST_CHANNELS
    RD_SCMI_FAST_CHANNEL_IDX,
#endif
    RD_ALARM_IDX_COUNT,
};

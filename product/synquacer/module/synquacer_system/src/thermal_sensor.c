/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_status.h>
#include <cmsis_os2.h>

#include <synquacer_debug.h>
#include <low_level_access.h>
#include <sysdef_option.h>

#include <internal/thermal_sensor.h>

#define DELAY_COUNTER 10

int thermal_enable(void)
{
    int32_t sensor_num = sysdef_option_get_sensor_num();
    int32_t i = 0;

    SYNQUACER_DEV_LOG_INFO("[THERMAL] Thermal enable start\n");
    writel(THERMAL_BASE_ADDRESS + THERMAL_ALLCONFIG_OFFSET, 0);
    for (i = 0; i < sensor_num; i++) {
        uint32_t sensor_offset = THERMAL_SENSOR_BASE(i);

        writel(sensor_offset + THERMAL_TS_EN_OFFSET, THERMAL_ENABLE);
        if (readl(sensor_offset + THERMAL_TS_EN_OFFSET) == 0) {
            SYNQUACER_DEV_LOG_INFO(
                "[THERMAL] Enable individual sensor #%x fail\n", i);
            return FWK_E_DEVICE;
        }

        writel(sensor_offset + THERMAL_TS_RESET_OFFSET, THERMAL_ENABLE);
        if (readl(sensor_offset + THERMAL_TS_RESET_OFFSET) == 0) {
            SYNQUACER_DEV_LOG_INFO(
                "[THERMAL] Reset individual sensor #%x fail\n", i);
            return FWK_E_DEVICE;
        }
    }

    for (i = 0; i < sensor_num; i++) {
        uint32_t sensor_offset = THERMAL_SENSOR_BASE(i);
        int32_t delay_counter;
        for (delay_counter = 0; delay_counter < DELAY_COUNTER;
             delay_counter++) {
            if (readl(sensor_offset + THERMAL_TSDATA_VALID_X_OFFSET) == 1)
                break;

            osDelay(1);
        }
    }

    SYNQUACER_DEV_LOG_INFO("[THERMAL] Thermal enable end\n");
    return FWK_SUCCESS;
}

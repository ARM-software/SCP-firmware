/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONFIG_SENSOR_H
#define CONFIG_SENSOR_H

/*
 * Enable full set of sensors available on the platform. Using just the sub-set
 * would not affect functionality but free-up space available.
 */
#define USE_FULL_SET_SENSORS    0

/* Sensor indices for Juno */
enum mod_juno_sensor_element_idx {
    /* XRP7724 sensor */
    MOD_JUNO_SENSOR_XRP7724_PMIC_TEMP_IDX,

    /* PVT sensors */
    MOD_JUNO_PVT_SENSOR_VOLT_BIG,
    MOD_JUNO_PVT_SENSOR_VOLT_LITTLE,
    MOD_JUNO_PVT_SENSOR_TEMP_SOC,
    MOD_JUNO_PVT_SENSOR_VOLT_SYS,

    /* ADC sensors */
    MOD_JUNO_SENSOR_VOLT_SYS_IDX,
    MOD_JUNO_SENSOR_VOLT_BIG_IDX,
    MOD_JUNO_SENSOR_VOLT_LITTLE_IDX,
    MOD_JUNO_SENSOR_VOLT_GPU_IDX,

    #if USE_FULL_SET_SENSORS
    MOD_JUNO_SENSOR_AMPS_SYS_IDX,
    MOD_JUNO_SENSOR_AMPS_BIG_IDX,
    MOD_JUNO_SENSOR_AMPS_LITTLE_IDX,
    MOD_JUNO_SENSOR_AMPS_GPU_IDX,
    MOD_JUNO_SENSOR_WATT_SYS_IDX,
    MOD_JUNO_SENSOR_WATT_BIG_IDX,
    MOD_JUNO_SENSOR_WATT_LITTLE_IDX,
    MOD_JUNO_SENSOR_WATT_GPU_IDX,
    MOD_JUNO_SENSOR_JOULE_SYS_IDX,
    MOD_JUNO_SENSOR_JOULE_BIG_IDX,
    MOD_JUNO_SENSOR_JOULE_LITTLE_IDX,
    MOD_JUNO_SENSOR_JOULE_GPU_IDX,
    #endif

    /* Number of sensors - Juno R0 */
    MOD_JUNO_R0_SENSOR_IDX_COUNT
};

/* Additional PVT Sensors for Juno R1 & R2 */
enum mod_juno_pvt_sensor_r1_r2_element_idx {
    MOD_JUNO_PVT_SENSOR_TEMP_BIG = MOD_JUNO_R0_SENSOR_IDX_COUNT,
    MOD_JUNO_PVT_SENSOR_TEMP_LITTLE,
    MOD_JUNO_PVT_SENSOR_TEMP_GPU_0,
    MOD_JUNO_PVT_SENSOR_TEMP_GPU_1,

    /* Number of sensors - Juno R1 & R2 */
    MOD_JUNO_R1_R2_SENSOR_IDX_COUNT
};

#endif /* CONFIG_SENSOR_H */

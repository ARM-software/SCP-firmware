/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MORELLO_SENSOR_H
#define MOD_MORELLO_SENSOR_H

#include <mod_sensor.h>

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupMORELLOModule MORELLO Product Modules
 * \{
 */

/*!
 * \defgroup GroupMORELLOSensor MORELLO Sensor Support
 *
 * \brief Driver for reading on-chip temperature & voltage sensor values.
 * \{
 */

/*!
 * \brief Length of buffer which stores the sensor values.
 */
#define PVT_HISTORY_LEN 64

/*!
 * \brief Sensor type
 */
enum sensor_type {
    /*! Temperature sensor */
    MOD_MORELLO_TEMP_SENSOR,

    /*! Voltage sensor */
    MOD_MORELLO_VOLT_SENSOR,
};

/*!
 * \brief Temperature sensor indices.
 */
enum mod_morello_temp_sensor_idx {
    /*! Cluster 0 temperature sensor */
    MOD_MORELLO_TEMP_SENSOR_IDX_CLUSTER0,

    /*! Cluster 1 temperature sensor */
    MOD_MORELLO_TEMP_SENSOR_IDX_CLUSTER1,

    /*! SYSTOP temperature sensor */
    MOD_MORELLO_TEMP_SENSOR_IDX_SYSTEM,

    /*! Temperature sensor count */
    MOD_MORELLO_TEMP_SENSOR_COUNT,
};

/*!
 * \brief Voltage sensor indices.
 */
enum mod_morello_volt_sensor_idx {
    /*! Cluster 0 Core 0 voltage sensor */
    MOD_MORELLO_VOLT_SENSOR_IDX_CLUS0CORE0 = MOD_MORELLO_TEMP_SENSOR_COUNT,

    /*! Cluster 0 Core 1 voltage sensor */
    MOD_MORELLO_VOLT_SENSOR_IDX_CLUS0CORE1,

    /*! Cluster 1 Core 0 voltage sensor */
    MOD_MORELLO_VOLT_SENSOR_IDX_CLUS1CORE0,

    /*! Cluster 1 Core 1 voltage sensor */
    MOD_MORELLO_VOLT_SENSOR_IDX_CLUS1CORE1,

    /*! Voltage sensor count */
    MOD_MORELLO_VOLT_SENSOR_COUNT,
};

/*!
 * \brief Sensor interrupt type
 */
enum sensor_interrupt_type {
    /*! No Interrupt */
    MOD_MORELLO_SENSOR_INVALID_INTERRUPT = -1,

    /*! Fault Interrupt */
    MOD_MORELLO_SENSOR_FAULT_INTERRUPT,

    /*! Alarm B Interrupt */
    MOD_MORELLO_SENSOR_ALARM_B_INTERRUPT,

    /*! Alarm A Interrupt */
    MOD_MORELLO_SENSOR_ALARM_A_INTERRUPT,

    /*! Sensor Interrupt Count */
    MOD_MORELLO_SENSOR_INTERRUPT_COUNT,
};

/*!
 * \brief Temperature sensor element configuration.
 * The alarms have programmable hysteresis and can be configured to detect
 * either rising or falling sample values.
 * To disable alarms set the hysteresis threshold equal to the alarm
 * threshold (default condition).
 * For a rising alarm, set hysteresis threshold less than alarm threshold.
 * Once triggered, the alarm is disarmed till the temperature again goes below
 * hysteresis threshold. The values are reversed for falling alarm.
 * Once enabled alarms will generate an interrupt if the value of the recovered
 * data sample equals or exceeds the rising or falling alarm threshold.
 */
struct mod_morello_temp_sensor_config {
    /*! Threshold value to raise an alarm */
    int32_t alarm_threshold;

    /*! Threshold value used to re-arm alarm A */
    int32_t alarm_hyst_threshold;

    /*! Threshold value to shutdown the temperature domain */
    int32_t shutdown_threshold;

    /*! Threshold value used to re-arm alarm B */
    int32_t shutdown_hyst_threshold;

    /*! Auxiliary sensor information */
    struct mod_sensor_info *info;
};

/*!
 * \brief Parameters of the event.
 */
struct mod_morello_sensor_event_param {
    /*! Offset of IP causing interrupt */
    int offset;
    /*! Type of interrupt */
    enum sensor_interrupt_type interrupt_type;
};

/*!
 * \brief Voltage sensor element configuration.
 */
struct mod_morello_volt_sensor_config {
    /*! Auxiliary sensor information */
    struct mod_sensor_info *info;
};

/*!
 * \brief MORELLO sensor - Module configuration.
 */
struct mod_morello_sensor_config {
    /*! Temperature sensor count */
    uint8_t t_sensor_count;

    /*! Voltage sensor count */
    uint8_t v_sensor_count;
};

/* Temperature sensor context */
struct morello_temp_sensor_ctx {
    /* Pointer to temperature sensor config */
    struct mod_morello_temp_sensor_config *config;
};

/* Voltage sensor context */
struct morello_volt_sensor_ctx {
    /* Pointer to voltage sensor config */
    struct mod_morello_volt_sensor_config *config;
};

/* MORELLO Sensor module context */
struct morello_sensor_ctx {
    /* Pointer to module configuration data */
    struct mod_morello_sensor_config *module_config;

    /* Table of temperature sensor contexts */
    struct morello_temp_sensor_ctx *t_dev_ctx_table;

    /* Table of voltage sensor contexts */
    struct morello_volt_sensor_ctx *v_dev_ctx_table;

    /* Pointer to SCP2PCC API */
    struct mod_morello_scp2pcc_api *scp2pcc_api;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_MORELLO_SENSOR_H */

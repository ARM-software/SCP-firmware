/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
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
 * \brief Temperature sensor element configuration.
 */
struct mod_morello_temp_sensor_config {
    /*! Threshold value to raise an alarm */
    int32_t alarm_threshold;

    /*! Threshold value to shutdown the temperature domain */
    int32_t shutdown_threshold;

    /*! Auxiliary sensor information */
    struct mod_sensor_info *info;
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
    /*! Identifier of timer alarm element */
    fwk_id_t alarm_id;

    /*! Identifier of timer alarm API */
    fwk_id_t alarm_api;

    /*! Temperature sensor count */
    uint8_t t_sensor_count;

    /*! Voltage sensor count */
    uint8_t v_sensor_count;
};

/* Temperature sensor context */
struct morello_temp_sensor_ctx {
    /* Pointer to temperature sensor config */
    struct mod_morello_temp_sensor_config *config;

    /* Pointer to history of temperature sensor values */
    int32_t *sensor_data_buffer;

    /* Buffer index pointing to latest temperature value */
    uint8_t buf_index;
};

/* Voltage sensor context */
struct morello_volt_sensor_ctx {
    /* Pointer to voltage sensor config */
    struct mod_morello_volt_sensor_config *config;

    /* Pointer to history of voltage sensor values */
    int32_t *sensor_data_buffer;

    /* Buffer index pointing to latest voltage value */
    uint8_t buf_index;
};

/* MORELLO Sensor module context */
struct morello_sensor_ctx {
    /* Pointer to module configuration data */
    struct mod_morello_sensor_config *module_config;

    /* Table of temperature sensor contexts */
    struct morello_temp_sensor_ctx *t_dev_ctx_table;

    /* Table of voltage sensor contexts */
    struct morello_volt_sensor_ctx *v_dev_ctx_table;

    /* Pointer to timer alarm API */
    struct mod_timer_alarm_api *timer_alarm_api;

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

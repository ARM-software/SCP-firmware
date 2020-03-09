/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_N1SDP_SENSOR_H
#define MOD_N1SDP_SENSOR_H

#include <mod_sensor.h>

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupN1SDPModule N1SDP Product Modules
 * @{
 */

/*!
 * \defgroup GroupN1SDPSensor N1SDP Sensor Support
 *
 * \brief Driver for reading on-chip temperature & voltage sensor values.
 * @{
 */

/*!
 * \brief Temperature sensor indices.
 */
enum mod_n1sdp_temp_sensor_idx {
    /*! Cluster 0 temperature sensor */
    MOD_N1SDP_TEMP_SENSOR_IDX_CLUSTER0,

    /*! Cluster 1 temperature sensor */
    MOD_N1SDP_TEMP_SENSOR_IDX_CLUSTER1,

    /*! SYSTOP temperature sensor */
    MOD_N1SDP_TEMP_SENSOR_IDX_SYSTEM,

    /*! Temperature sensor count */
    MOD_N1SDP_TEMP_SENSOR_COUNT,
};

/*!
 * \brief Voltage sensor indices.
 */
enum mod_n1sdp_volt_sensor_idx {
    /*! Cluster 0 Core 0 voltage sensor */
    MOD_N1SDP_VOLT_SENSOR_IDX_CLUS0CORE0 = MOD_N1SDP_TEMP_SENSOR_COUNT,

    /*! Cluster 0 Core 1 voltage sensor */
    MOD_N1SDP_VOLT_SENSOR_IDX_CLUS0CORE1,

    /*! Cluster 1 Core 0 voltage sensor */
    MOD_N1SDP_VOLT_SENSOR_IDX_CLUS1CORE0,

    /*! Cluster 1 Core 1 voltage sensor */
    MOD_N1SDP_VOLT_SENSOR_IDX_CLUS1CORE1,

    /*! SYSTOP voltage sensor */
    MOD_N1SDP_VOLT_SENSOR_IDX_SYSTEM,

    /*! Voltage sensor count */
    MOD_N1SDP_VOLT_SENSOR_COUNT,
};


/*!
 * \brief Temperature sensor element configuration.
 */
struct mod_n1sdp_temp_sensor_config {
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
struct mod_n1sdp_volt_sensor_config {
    /*! Auxiliary sensor information */
    struct mod_sensor_info *info;
};

/*!
 * \brief N1SDP sensor - Module configuration.
 */
struct mod_n1sdp_sensor_config {
    /*! Identifier of timer alarm element */
    fwk_id_t alarm_id;

    /*! Identifier of timer alarm API */
    fwk_id_t alarm_api;

    /*! Temperature sensor count */
    uint8_t t_sensor_count;

    /*! Voltage sensor count */
    uint8_t v_sensor_count;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_N1SDP_SENSOR_H */

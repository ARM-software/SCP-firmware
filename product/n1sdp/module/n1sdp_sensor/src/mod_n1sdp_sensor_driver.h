/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_N1SDP_SENSOR_DRIVER_H
#define MOD_N1SDP_SENSOR_DRIVER_H

#include <mod_n1sdp_sensor.h>

#define PVT_HISTORY_LEN 64

/* Temperature sensor context */
struct n1sdp_temp_sensor_ctx {
    /* Pointer to temperature sensor config */
    struct mod_n1sdp_temp_sensor_config *config;

    /* Pointer to history of temperature sensor values */
    int32_t *sensor_data_buffer;

    /* Buffer index pointing to latest temperature value */
    uint8_t buf_index;
};

/* Voltage sensor context */
struct n1sdp_volt_sensor_ctx {
    /* Pointer to voltage sensor config */
    struct mod_n1sdp_volt_sensor_config *config;

    /* Pointer to history of voltage sensor values */
    int32_t *sensor_data_buffer;

    /* Buffer index pointing to latest voltage value */
    uint8_t buf_index;
};

/* N1SDP Sensor module context */
struct n1sdp_sensor_ctx {
    /* Pointer to module configuration data */
    struct mod_n1sdp_sensor_config *module_config;

    /* Table of temperature sensor contexts */
    struct n1sdp_temp_sensor_ctx *t_dev_ctx_table;

    /* Table of voltage sensor contexts */
    struct n1sdp_volt_sensor_ctx *v_dev_ctx_table;

    /* Pointer to timer alarm API */
    struct mod_timer_alarm_api *timer_alarm_api;

    /* Pointer to SCP2PCC API */
    struct mod_n1sdp_scp2pcc_api *scp2pcc_api;
};

static const char *const sensor_type_name[] = {
    [MOD_N1SDP_TEMP_SENSOR_IDX_CLUSTER0] = "T-CLUS0",
    [MOD_N1SDP_TEMP_SENSOR_IDX_CLUSTER1] = "T-CLUS1",
    [MOD_N1SDP_TEMP_SENSOR_IDX_SYSTEM] = "T-SYS",
    [MOD_N1SDP_VOLT_SENSOR_IDX_CLUS0CORE0] = "V-CLUS0CORE0",
    [MOD_N1SDP_VOLT_SENSOR_IDX_CLUS0CORE1] = "V-CLUS0CORE1",
    [MOD_N1SDP_VOLT_SENSOR_IDX_CLUS1CORE0] = "V-CLUS1CORE0",
    [MOD_N1SDP_VOLT_SENSOR_IDX_CLUS1CORE1] = "V-CLUS1CORE1",
    [MOD_N1SDP_VOLT_SENSOR_IDX_SYSTEM] = "V-SYS",
};

int n1sdp_sensor_lib_sample(int32_t *value, int sensor_type, int offset);
void n1sdp_sensor_lib_trigger_sample(int sensor_type);
int n1sdp_sensor_lib_init(uint32_t *msg);

#endif

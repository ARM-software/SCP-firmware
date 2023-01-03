/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MORELLO_SENSOR_DRIVER_H
#define MOD_MORELLO_SENSOR_DRIVER_H

#include <mod_morello_sensor.h>

/*!
 * \brief Get the sampled value of the sensor
 *
 * \param value  Sampled value of the sensor.
 * \param type   Sensor type which should be sampled.
 * \param offset Instance of the sensor type which should be sampled.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \return One of the other specific error codes described by the framework.
 */
int morello_sensor_lib_get_sensor_value(
    int32_t *value,
    enum sensor_type type,
    int offset);

/*!
 * \brief Intialize the sensor.
 *
 * \param msg Information regarding the sensor.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \return One of the other specific error codes described by the framework.
 */
int morello_sensor_lib_init(uint32_t *msg);

/*!
 * \brief Sensor library ISR routine.
 *
 * \param offset Sensor offset that triggered interrupt.
 *
 * \return Sensor Type that triggered interrupt.
 */
int morello_sensor_lib_handle_irq(int *offset);

/*!
 * \brief Configure and enable alarms for each Temperature Sensor IP.
 *
 * \param offset Temperature sensor IP offset.
 * \param alarm_A_threshold alarmA threshold for temperature sensor IP.
 * \param alarm_B_threshold alarmB threshold for temperature sensor IP.
 * \param alarm_A_hysteresis hysteresis threshold for alarm A in temperature
 *        sensor IP.
 * \param alarm_B_hysteresis hysteresis threshold for alarm B in temperature
 *        sensor IP.
 * \return Sensor Type that triggered interrupt.
 */
void morello_enable_temp_sensor_alarm(
    int offset,
    float alarm_A_threshold,
    float alarm_B_threshold,
    float alarm_A_hysteresis,
    float alarm_B_hysteresis);
#endif

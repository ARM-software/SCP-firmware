/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
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
int morello_sensor_lib_sample(
    int32_t *value,
    enum sensor_type type,
    int offset);

/*!
 * \brief Trigger sampling of the sensor.
 *
 * \param type Sensor type which should be sampled.
 *
 * \retval NONE
 */
void morello_sensor_lib_trigger_sample(enum sensor_type type);

/*!
 * \brief Intialize the sensor.
 *
 * \param msg Information regarding the sensor.
 *
 * \retval ::FWK_SUCCESS Operation succeeded.
 * \return One of the other specific error codes described by the framework.
 */
int morello_sensor_lib_init(uint32_t *msg);

#endif

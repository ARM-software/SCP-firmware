/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_MOCK_SENSOR_H
#define MOD_MOCK_SENSOR_H

#include <mod_sensor.h>

#include <stdint.h>

/*!
 * \ingroup GroupModules
 * \defgroup GroupMockSensor Mock Sensor Driver
 *
 * \details The `mock_sensor` module provides a simulated asynchronous driver
 *      for use alongside the `sensor` HAL interface on systems that do not
 *      provide a real asynchronous sensor.
 *
 *      This mock driver implements the Sensor HAL driver API and always defers
 *      the 'get_value' request. An example of the execution flow is shown in
 *      `Deferred Response Architecture`.
 *
 * \{
 */

/*! \brief Element configuration */
struct mod_mock_sensor_dev_config {
    /*! Pointer to sensor information */
    struct mod_sensor_info *info;

    /*! Sensor HAL identifier */
    fwk_id_t sensor_hal_id;

    /*! Identifier of the alarm assigned to this element */
    fwk_id_t alarm_id;
};

/*!
 * \}
 */

#endif /* MOD_MOCK_SENSOR_H */

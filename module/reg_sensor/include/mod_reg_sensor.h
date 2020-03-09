/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_REG_SENSOR_H
#define MOD_REG_SENSOR_H

#include <mod_sensor.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupModuleRegSensor Register Sensor Driver
 *
 * \brief Driver for simple, register-based sensors.
 * @{
 */

/*! \brief Element configuration */
struct mod_reg_sensor_dev_config {
    /*! Address of the sensor register */
    uintptr_t reg;

    /*! Auxiliary sensor information */
    struct mod_sensor_info *info;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_REG_SENSOR_H */

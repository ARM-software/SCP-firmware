/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_REG_SENSOR_H
#define MOD_REG_SENSOR_H

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
    uintptr_t reg; /*!< Address of the sensor register */
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_REG_SENSOR_H */

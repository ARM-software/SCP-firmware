/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SENSOR_H
#define MOD_SENSOR_H

#include <fwk_id.h>
#include <stdint.h>
#include <stdbool.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupModuleSensor Sensor
 *
 * \brief Module for reading hardware sensors.
 *
 * \details Module for interfacing with and reading various hardware sensors.
 *
 * @{
 */

/*!
 * \brief Sensor types as defined by SCMI.
 */
enum mod_sensor_type {
    MOD_SENSOR_TYPE_NONE = 0,
    MOD_SENSOR_TYPE_UNSPECIFIED,
    MOD_SENSOR_TYPE_DEGREES_C,
    MOD_SENSOR_TYPE_DEGREES_F,
    MOD_SENSOR_TYPE_DEGREES_K,
    MOD_SENSOR_TYPE_VOLTS,
    MOD_SENSOR_TYPE_AMPS,
    MOD_SENSOR_TYPE_WATTS,
    MOD_SENSOR_TYPE_JOULES,
    MOD_SENSOR_TYPE_COULOMBS,
    MOD_SENSOR_TYPE_VA,
    MOD_SENSOR_TYPE_NITS,
    MOD_SENSOR_TYPE_LUMENS,
    MOD_SENSOR_TYPE_LUX,
    MOD_SENSOR_TYPE_CANDELAS,
    MOD_SENSOR_TYPE_KPA,
    MOD_SENSOR_TYPE_PSI,
    MOD_SENSOR_TYPE_NEWTONS,
    MOD_SENSOR_TYPE_CFM,
    MOD_SENSOR_TYPE_RPM,
    MOD_SENSOR_TYPE_HERTZ,
    MOD_SENSOR_TYPE_SECONDS,
    MOD_SENSOR_TYPE_MINUTES,
    MOD_SENSOR_TYPE_HOURS,
    MOD_SENSOR_TYPE_DAYS,
    MOD_SENSOR_TYPE_WEEKS,
    MOD_SENSOR_TYPE_MILS,
    MOD_SENSOR_TYPE_INCHES,
    MOD_SENSOR_TYPE_FEET,
    MOD_SENSOR_TYPE_CUBIC_INCHES,
    MOD_SENSOR_TYPE_CUBIC_FEET,
    MOD_SENSOR_TYPE_METERS,
    MOD_SENSOR_TYPE_CUBIC_CENTIMETERS,
    MOD_SENSOR_TYPE_CUBIC_METERS,
    MOD_SENSOR_TYPE_LITRES,
    MOD_SENSOR_TYPE_FLUID_OUNCES,
    MOD_SENSOR_TYPE_RADIANS,
    MOD_SENSOR_TYPE_STERADIANS,
    MOD_SENSOR_TYPE_REVOLUTIONS,
    MOD_SENSOR_TYPE_CYCLES,
    MOD_SENSOR_TYPE_GRAVITIES,
    MOD_SENSOR_TYPE_OUNCES,
    MOD_SENSOR_TYPE_POUNDS,
    MOD_SENSOR_TYPE_FOOT_POUNDS,
    MOD_SENSOR_TYPE_OUNCE_INCHES,
    MOD_SENSOR_TYPE_GAUSS,
    MOD_SENSOR_TYPE_GILBERTS,
    MOD_SENSOR_TYPE_HENRIES,
    MOD_SENSOR_TYPE_FARADS,
    MOD_SENSOR_TYPE_OHMS,
    MOD_SENSOR_TYPE_SIEMENS,
    MOD_SENSOR_TYPE_MOLES,
    MOD_SENSOR_TYPE_BECQUERELS,
    MOD_SENSOR_TYPE_PPM,
    MOD_SENSOR_TYPE_DECIBELS,
    MOD_SENSOR_TYPE_DBA,
    MOD_SENSOR_TYPE_DBC,
    MOD_SENSOR_TYPE_GRAYS,
    MOD_SENSOR_TYPE_SIEVERTS,
    MOD_SENSOR_TYPE_COLOR_TEMP_DEGREES_K,
    MOD_SENSOR_TYPE_BITS,
    MOD_SENSOR_TYPE_BYTES,
    MOD_SENSOR_TYPE_WORDS,
    MOD_SENSOR_TYPE_DWORDS,
    MOD_SENSOR_TYPE_QWORDS,
    MOD_SENSOR_TYPE_PERCENTAGE,
    MOD_SENSOR_TYPE_PASCALS,
    MOD_SENSOR_TYPE_COUNTS,
    MOD_SENSOR_TYPE_GRAMS,
    MOD_SENSOR_TYPE_NEWTON_METERS,
    MOD_SENSOR_TYPE_HITS,
    MOD_SENSOR_TYPE_MISSES,
    MOD_SENSOR_TYPE_RETRIES,
    MOD_SENSOR_TYPE_OVERRUNS,
    MOD_SENSOR_TYPE_UNDERRUNS,
    MOD_SENSOR_TYPE_COLLISIONS,
    MOD_SENSOR_TYPE_PACKETS,
    MOD_SENSOR_TYPE_MESSAGES,
    MOD_SENSOR_TYPE_CHARACTERS,
    MOD_SENSOR_TYPE_ERRORS,
    MOD_SENSOR_TYPE_CORRECTED_ERRORS,
    MOD_SENSOR_TYPE_UNCORRECTABLE_ERRORS,
    MOD_SENSOR_TYPE_SQUARE_MILS,
    MOD_SENSOR_TYPE_SQUARE_INCHES,
    MOD_SENSOR_TYPE_SQUARE_FEET,
    MOD_SENSOR_TYPE_SQUARE_CENTIMETERS,
    MOD_SENSOR_TYPE_SQUARE_METERS,
    MOD_SENSOR_TYPE_OEM_UNIT = 0xFF,
    MOD_SENSOR_TYPE_COUNT
};

/*!
 * \brief Structure containing all sensor information.
 *
 * \details Sensor information structure used to configure the sensor and serve
 *     SCMI requests.
 */
struct mod_sensor_info {
    /*! SCMI sensor type */
    enum mod_sensor_type type;

    /*! Time (in seconds) between sensor updates. Set this field to 0 to
     *  indicate that the sensor does not have a minimum update interval. This
     *  field is used with \ref update_interval_multiplier to calculate the
     *  actual update_interval.
     */
    unsigned int update_interval;

    /*!
     *  Power-of-10 multiplier for \ref update_interval \n\n
     *  This is used to calculate the actual interval time:\n
     *  actual = \ref update_interval x10^(\ref update_interval_multiplier)\n
     */
    int update_interval_multiplier;

    /*!
     *  Power-of-10 multiplier applied to the unit (specified by \ref type)\n\n
     *  Used like this: unit x10^(\ref unit_multiplier)
     */
    int unit_multiplier;
};

/*!
 * \brief Sensor device configuration.
 *
 * \details Configuration structure for individual sensors.
 */
struct mod_sensor_dev_config {
    /*! Module or element id of the driver */
    fwk_id_t driver_id;

    /*! Sensor information */
    struct mod_sensor_info *info;
};

/*!
 * \brief Sensor driver API.
 *
 * \details Api used by this module to interface with the driver.
 */
struct mod_sensor_driver_api {
    /*!
     * \brief Read sensor value.
     *
     * \details Synchronously read current sensor value.
     *
     * \param id Specific sensor device id.
     * \param value Pointer to storage for the sensor value.
     *
     * \retval FWK_SUCCESS Value was read successfully.
     * \return One of the standard framework error codes.
     */
    int (*get_value)(fwk_id_t id, uint64_t *value);
};

/*!
 * \brief Sensor API.
 */
struct mod_sensor_api {
    /*!
     * \brief Read sensor value.
     *
     * \details Synchronously read current sensor value.
     *
     * \param id Specific sensor device id.
     * \param value Pointer to storage for the sensor value.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_DEVICE Driver error.
     * \return One of the standard framework error codes.
     */
    int (*get_value)(fwk_id_t id, uint64_t *value);

    /*!
     * \brief Get sensor information.
     *
     * \details Get a pointer to the sensor_info structure of a specific sensor.
     *
     * \param id Specific sensor device id.
     * \param info Pointer to storage for the info struct.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*get_info)(fwk_id_t id, struct mod_sensor_info *info);
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SENSOR_H */

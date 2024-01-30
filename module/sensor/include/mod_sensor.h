/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SENSOR_H
#define MOD_SENSOR_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupModuleSensor Sensor
 *
 * \brief Module for reading hardware sensors.
 *
 * \details Module for interfacing with and reading various hardware sensors.
 *
 * \{
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
    MOD_SENSOR_TYPE_RADIANS_PER_SECOND,
    MOD_SENSOR_TYPE_BEATS_PER_MINUTE,
    MOD_SENSOR_TYPE_METERS_PER_SECOND_SQUARED,
    MOD_SENSOR_TYPE_METERS_PER_SECOND,
    MOD_SENSOR_TYPE_CUBIC_METER_PER_SECOND,
    MOD_SENSOR_TYPE_MILLIMETERS_OF_MERCURY,
    MOD_SENSOR_TYPE_RADIANS_PER_SECOND_SQUARED,
    MOD_SENSOR_TYPE_OEM_UNIT = 0xFF,
    MOD_SENSOR_TYPE_COUNT
};

/*!
 * \brief Structure containing all sensor trip point information.
 */
struct mod_sensor_trip_point_info {
    /*! Sensor trip point count */
    uint32_t count;
};

/*!
 * \brief Sensor value signedness type.
 */
#ifdef BUILD_HAS_SENSOR_SIGNED_VALUE
typedef int64_t mod_sensor_value_t;
#else
typedef uint64_t mod_sensor_value_t;
#endif

#ifdef BUILD_HAS_SENSOR_EXT_ATTRIBS

/*!
 * \brief Structure containing all extended attributes for multi axis
 *        information.
 *
 * \details Sensor information structure used to configure the sensor multi
 *          axis values.
 */
struct mod_sensor_axis_attributes {
    /*! Axis resolution value */
    uint32_t axis_resolution;

    /*! Axis minimum range */
    int64_t axis_min_range;

    /*! Axis maximum range */
    int64_t axis_max_range;
};

/*!
 * \brief Structure containing all sensor property information.
 *
 * \details Sensor information structure used to configure the sensor
 *          property values.
 */
struct mod_sensor_ext_properties {
    /*! Sensor power value */
    uint32_t sensor_power;

    /*! Further sensor property values */
    struct mod_sensor_axis_attributes sensor_property_vals;
};
#endif

#ifdef BUILD_HAS_SENSOR_TIMESTAMP
/*!
 * \brief Structure containing all timestamp information.
 */
struct mod_sensor_timestamp_info {
    /*! Sensor timestamp support */
    bool timestamp_support;

    /*! Sensor timestamp enabled */
    bool enabled;

    /*!
     * \brief Sensor timestamp exponent value
     *
     * \details It is the power-of-10 multiplier that is applied to the
     *      sensor timestamps (timestamp x 10 ^ [timestamp exponent] ) to
     *      represent it in seconds.
     */
    int8_t exponent;
};
#endif
#ifdef BUILD_HAS_SENSOR_MULTI_AXIS
/*!
 * \brief Structure containing infomation for a specific sensor axis
 *
 * \details Sensor axis information structure used to configure the sensor HAL.
 */
struct mod_sensor_axis_info {
    /*! Sensor axis name */
    const char *name;

    /*! SCMI sensor type */
    enum mod_sensor_type type;

    /*!
     * \brief Power-of-10 multiplier applied to the unit specified by
     *      ::mod_sensor_info::type.
     *
     * \details Used per:
     *
     *      ```none
     *      unit * 10^(unit_multiplier)
     *      ```
     */
    int unit_multiplier;
#    ifdef BUILD_HAS_SENSOR_EXT_ATTRIBS
    /*! Extended attributes */
    bool extended_attribs;

    /*! Multi axis property values */
    struct mod_sensor_axis_attributes multi_axis_properties;
#    endif
};
#endif

/*!
 * \brief Structure containing all sensor driver information.
 *
 * \details Sensor information structure used to configure the sensor HAL.
 */
struct mod_sensor_info {
    /*! SCMI sensor type */
    enum mod_sensor_type type;

    /*!
     * \brief Time (in seconds) between sensor updates.
     *
     * \details Set this field to 0 to indicate that the sensor does not have a
     *      minimum update interval. This field is used with
     *      ::mod_sensor_info::update_interval_multiplier to calculate the
     *      actual update interval.
     */
    unsigned int update_interval;

    /*!
     * \brief Power-of-10 multiplier for ::mod_sensor_info::update_interval.
     *
     * \details This is used to calculate the actual interval time:
     *
     *      ```none
     *      actual = update_interval * 10^(update_interval_multiplier)
     *      ```
     */
    int update_interval_multiplier;

    /*!
     * \brief Power-of-10 multiplier applied to the unit specified by
     *      ::mod_sensor_info::type.
     *
     * \details Used per:
     *
     *      ```none
     *      unit * 10^(unit_multiplier)
     *      ```
     */
    int unit_multiplier;

    /*!
     * \brief Boolean flag to indicate whether a sensor is
     *        enabled or disabled.
     *
     * \details Set this to false to indicate that a sensor starts
     *          enabled or true to indicate that it starts disabled.
     *          The flag is updated dynamically when SCMI commands
     *          are received to enable or disable a sensor.
     */
    bool disabled;

#ifdef BUILD_HAS_SENSOR_EXT_ATTRIBS
    /*! Extended attributes information */
    bool ext_attributes;

    /*! Sensor property values */
    struct mod_sensor_ext_properties sensor_properties;
#endif
};

/*!
 * \brief Structure containing all sensor information for SCMI requests.
 *
 * \details Sensor information structure used serve SCMI requests.
 */
struct mod_sensor_complete_info {
    /*!
     * \brief Sensor HAL information.
     *
     * \details If multi axis configuration is supported not all parameters will
     *      be filled here.
     */
    struct mod_sensor_info hal_info;

    /*! Sensor trip information */
    struct mod_sensor_trip_point_info trip_point;

#ifdef BUILD_HAS_SENSOR_TIMESTAMP
    /*! Sensor timestamp information */
    struct mod_sensor_timestamp_info timestamp;
#endif

#ifdef BUILD_HAS_SENSOR_MULTI_AXIS
    /*! Sensor multi axis information */
    struct {
        /*! Multi axis supported feature */
        bool support;
        /*! Number of axis configured */
        unsigned int axis_count;
    } multi_axis;
#endif
};

/*!
 * \brief Sensor trip point detection mode
 */
enum mod_sensor_trip_point_mode {
    MOD_SENSOR_TRIP_POINT_MODE_DISABLED = 0,
    MOD_SENSOR_TRIP_POINT_MODE_POSITIVE,
    MOD_SENSOR_TRIP_POINT_MODE_NEGATIVE,
    MOD_SENSOR_TRIP_POINT_MODE_TRANSITION
};

/*!
 * \brief Structure containing trip point parameters.
 *
 * \details Sensor trip point information structure used to configure
 *     a trip point value.
 */
struct mod_sensor_trip_point_params {
    /*! Sensor trip point value */
    uint64_t tp_value;

    /*! Sensor trip point mode */
    enum mod_sensor_trip_point_mode mode;
};

/*!
 * \brief Sensor device configuration.
 *
 * \details Configuration structure for individual sensors.
 */
struct mod_sensor_dev_config {
    /*! Module or element identifier of the driver */
    fwk_id_t driver_id;

    /*! API identifier of the driver */
    fwk_id_t driver_api_id;

    /*!  Notifications identifier */
    fwk_id_t notification_id;

    /*! Trip point API identifier */
    fwk_id_t trip_point_api_id;

    /*! Sensor trip information */
    struct mod_sensor_trip_point_info trip_point;

#ifdef BUILD_HAS_SENSOR_TIMESTAMP
    /*! Sensor timestamp default values configuration */
    struct mod_sensor_timestamp_info timestamp;
#endif
};

/*!
 * \brief Sensor data.
 *
 * \details Sensor data structure that contains all related value reading
 *      information.
 */
struct mod_sensor_data {
    /*! Status of the response event */
    int status;

    /*! Sensor value */
    union {
        /*! Sensor N-axis value */
        mod_sensor_value_t *axis_value;
        /*! Sensor scalar value */
        mod_sensor_value_t value;
    };

#ifdef BUILD_HAS_SENSOR_TIMESTAMP
    /*! Timestamp value */
    uint64_t timestamp;
#endif

#ifdef BUILD_HAS_SENSOR_MULTI_AXIS
    /*! Number of axis */
    uint32_t axis_count;
#endif
};

/*!
 * \brief Sensor module configuration.
 *
 * \details Configuration structure sensor module.
 */
struct mod_sensor_config {
    /*!  Notifications identifier */
    fwk_id_t notification_id;

    /*! Trip point API identifier */
    fwk_id_t trip_point_api_id;
};

/*!
 * \brief Sensor driver API.
 *
 * \details Api used by this module to interface with the driver.
 */
struct mod_sensor_driver_api {
    /*!
     * \brief Get sensor value.
     *
     * \param id Specific sensor device id.
     * \param[out] value Sensor value, which can be either signed or
     *     unsigned, depending upon the build options.
     *
     * \retval ::FWK_PENDING The request is pending. The driver will provide the
     *      requested value later through the driver response API.
     * \retval ::FWK_SUCCESS Value was read successfully.
     * \return One of the standard framework error codes.
     */
    int (*get_value)(fwk_id_t id, mod_sensor_value_t *value);

    /*!
     * \brief Get sensor information.
     *
     * \param id Specific sensor device id.
     * \param[out] info The sensor information.
     *
     * \retval ::FWK_SUCCESS The information was read successfully.
     * \return One of the standard framework error codes.
     */
    int (*get_info)(fwk_id_t id, struct mod_sensor_info *info);

    /*!
     * \brief Enable_sensor.
     *
     * \param id Specific sensor device id.
     *
     * \retval ::FWK_SUCCESS The operation was performed successfully.
     * \retval ::FWK_E_SUPPORT The operation is not supported by the driver API.
     */
    int (*enable)(fwk_id_t id);

    /*!
     * \brief Disable_sensor.
     *
     * \param id Specific sensor device id.
     *
     * \retval ::FWK_SUCCESS The operation was performed successfully.
     * \retval ::FWK_E_SUPPORT The operation is not supported by the driver API.
     */
    int (*disable)(fwk_id_t id);

    /*!
     * \brief Set update interval.
     *
     * \param id Specific sensor device id.
     * \param update_interval The new update interval value.
     * \param update_interval_multiplier The new update_interval_multiplier
     * value.
     *
     * \retval ::FWK_SUCCESS The operation was performed successfully.
     */

    int (*set_update_interval)(
        fwk_id_t id,
        unsigned int update_interval,
        int update_interval_multiplier);

    /*!
     * \brief Get update interval.
     *
     * \param id Specific sensor device id.
     * \param update_interval An address to hold the update interval value.
     * \param update_interval_multiplier An address to hold the
     * update_interval_multiplier value.
     *
     * \retval ::FWK_SUCCESS The operation was performed successfully.
     */

    int (*get_update_interval)(
        fwk_id_t id,
        unsigned int *update_interval,
        int *update_interval_multiplier);

#ifdef BUILD_HAS_SENSOR_MULTI_AXIS
    /*!
     * \brief Get number of axis.
     *
     * \param id Specific sensor device id.
     *
     * \retval Number of axis.
     */
    unsigned int (*get_axis_count)(fwk_id_t id);

    /*!
     * \brief Get axis sensor information.
     *
     * \param id Specific sensor device id.
     * \param axis Specific axis.
     * \param[out] info The sensor information.
     *
     * \retval ::FWK_SUCCESS The information was read successfully.
     * \return One of the standard framework error codes.
     */
    int (*get_axis_info)(
        fwk_id_t id,
        uint32_t axis,
        struct mod_sensor_axis_info *info);
#endif
};

/*!
 * \brief Sensor API.
 */
struct mod_sensor_api {
    /*!
     * \brief Read sensor data.
     *
     * \details Read current sensor data.
     *
     * \param id Specific sensor device id.
     * \param[out] data Sensor struct data will be returned.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_DEVICE Driver error.
     * \retval ::FWK_E_BUSY At least one reading of the sensor data is already
     *      on-going.
     * \retval ::FWK_PENDING The request is pending. The requested data will be
     *      provided via a response event.
     * \return One of the standard framework error codes.
     */
    int (*get_data)(fwk_id_t id, struct mod_sensor_data *data);

    /*!
     * \brief Get sensor information.
     *
     * \details Get a pointer to the sensor_info structure of a specific sensor.
     *
     * \param id Specific sensor device id.
     * \param[out] info The information structure.
     *
     * \retval ::FWK_SUCCESS Operation succeeded.
     * \retval ::FWK_E_DEVICE Driver error.
     * \return One of the standard framework error codes.
     */
    int (*get_info)(fwk_id_t id, struct mod_sensor_complete_info *info);

    /*!
     * \brief Set trip point.
     *
     * \details Set trip point sensor configuration.
     *
     * \param id Specific sensor device id.
     * \param trip_point_idx Specific trip point index.
     * \param params Pointer to trip points parameters structure.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*set_trip_point)(
        fwk_id_t id,
        uint32_t trip_point_idx,
        struct mod_sensor_trip_point_params *params);

    /*!
     * \brief Get trip point.
     *
     * \details Get trip point sensor configuration.
     *
     * \param id Specific sensor device id.
     * \param trip_point_idx Specific trip point index.
     * \param[out] params Pointer to trip points parameters structure.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*get_trip_point)(
        fwk_id_t id,
        uint32_t trip_point_idx,
        struct mod_sensor_trip_point_params *params);

    /*!
     * \brief Enable.
     *
     * \details Changes the "enabled" state of a sensor to true.
     *
     * \param id Specific sensor device id.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_PARAM "sensor_get_timestamp_config" returned
     *      "configuration is null".
     */
    int (*enable)(fwk_id_t id);

    /*!
     * \brief Disable.
     *
     * \details Changes the "enabled" state of a sensor to false.
     *
     * \param id Specific sensor device id.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_PARAM "sensor_get_timestamp_config" returned
     *      "configuration is null".
     */
    int (*disable)(fwk_id_t id);

    /*!
     * \brief Set update interval.
     *
     * \details Updates the update time interval of a sensor.
     *
     * \param id Specific sensor device id.
     * \param time_interval New time interval value.
     * \param time_interval_multiplier New time interval multiplier value.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_SUPPORT Operation not supported by driver.
     */
    int (*set_update_interval)(
        fwk_id_t id,
        unsigned int time_interval,
        int time_interval_multiplier);

    /*!
     * \brief Get update interval.
     *
     * \details Returns the current update time interval of a sensor.
     *
     * \param id Specific sensor device id.
     * \param[out] time_interval Pointer to a variable to take
     *      the time interval value.
     * \param[out] time_interval_multiplier Pointer to a variable
     *      to take the time interval multiplier value.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_DEVICE "get_info" returned error.
     * \retval FWK_E_SUPPORT "sensor_get_timestamp_config" returned
     *      "no timestamp support".
     * \retval  FWK_E_PARAM "sensor_get_timestamp_config" returned
     *      "configuration is null".
     */
    int (*get_update_interval)(
        fwk_id_t id,
        unsigned int *time_interval,
        int *time_interval_multiplier);

#ifdef BUILD_HAS_SENSOR_TIMESTAMP
    /*!
     * \brief Configure timestamp
     *
     * \details Set timestamp configuration
     *
     * \param id Specific sensor device id.
     * \param config Timestamp configuration structure.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \retval FWK_E_SUPPORT Operation not supported by sensor.
     * \return One of the standard framework error codes.
     */
    int (*set_timestamp_config)(
        fwk_id_t id,
        const struct mod_sensor_timestamp_info *config);

    /*!
     * \brief Read timestamp configuration
     *
     * \details Get timestamp configuration
     *
     * \param id Specific sensor device id.
     * \param[out] config Timestamp configuration structure.
     *
     * \retval FWK_SUCCESS Operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*get_timestamp_config)(
        fwk_id_t id,
        struct mod_sensor_timestamp_info *config);

    /*!
     * \brief Get axis sensor information.
     *
     * \param id Specific sensor device id.
     * \param axis Specific axis.
     * \param[out] info The sensor information.
     *
     * \retval ::FWK_SUCCESS The information was read successfully.
     * \return One of the standard framework error codes.
     */
    int (*get_axis_info)(
        fwk_id_t id,
        uint32_t axis,
        struct mod_sensor_axis_info *info);
#endif
};

/*!
 * \brief Driver response parameters.
 */
struct mod_sensor_driver_resp_params {
    /*! Status of the requested operation */
    int status;

    /*! Sensor value */
    union {
        /*! Sensor N-axis value */
        mod_sensor_value_t *axis_value;
        /*! Sensor scalar value */
        mod_sensor_value_t value;
    };
};

/*!
 * \brief Driver response API.
 *
 * \details API used by the driver to notify the HAL when a pending request
 *      has completed.
 */
struct mod_sensor_driver_response_api {
    /*!
     * \brief Inform the completion of a sensor reading.
     *
     * \param id Specific sensor device identifier.
     * \param[out] response The response data structure.
     */
    void (*reading_complete)(fwk_id_t id,
                             struct mod_sensor_driver_resp_params *response);
};

/*!
 * \brief API indices.
 */
enum mod_sensor_api_idx {
    /*!
     * \brief Driver API index.
     *
     * \note This API implements the ::mod_sensor_api interface.
     */
    MOD_SENSOR_API_IDX_SENSOR,

    /*!
     * \brief Driver response API.
     */
    MOD_SENSOR_API_IDX_DRIVER_RESPONSE,

    /*!
     * \brief Number of defined APIs.
     */
    MOD_SENSOR_API_IDX_COUNT,
};

/*!
 * \brief Module API identifier.
 */
static const fwk_id_t mod_sensor_api_id_sensor =
    FWK_ID_API_INIT(FWK_MODULE_IDX_SENSOR, MOD_SENSOR_API_IDX_SENSOR);

/*!
 * \brief Driver input API identifier.
 */
static const fwk_id_t mod_sensor_api_id_driver_response =
    FWK_ID_API_INIT(FWK_MODULE_IDX_SENSOR, MOD_SENSOR_API_IDX_DRIVER_RESPONSE);

/*!
 * \brief Shared event parameters.
 */
struct mod_sensor_event_params {
    /*! Sensor value pointer */
    struct mod_sensor_data *sensor_data;
};

/*!
 * Sensor module read request event index
 */
#define MOD_SENSOR_EVENT_IDX_READ_REQUEST    0

/*!
 * \brief Read request event identifier.
 *
 * \details Clients which expect to receive a response event from this module
 *      should use this identifier to properly identify the response.
 */
static const fwk_id_t mod_sensor_event_id_read_request =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_SENSOR, MOD_SENSOR_EVENT_IDX_READ_REQUEST);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SENSOR_H */

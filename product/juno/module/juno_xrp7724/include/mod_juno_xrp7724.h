/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     XRP7724 driver (aka Juno PMIC)
 */

#ifndef MOD_JUNO_XRP7724_H
#define MOD_JUNO_XRP7724_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \ingroup GroupJunoModule
 *
 * \defgroup GroupXRP7724 XRP7724 Driver
 * \{
 */

/*!
 * \brief Module configuration.
 */
struct mod_juno_xrp7724_config {
    /*! Slave address of the I2C device */
    unsigned int slave_address;

    /*! Identifier of the I2C HAL */
    fwk_id_t i2c_hal_id;

    /*! Identifier of the timer */
    fwk_id_t timer_hal_id;

    /*! Identifier of the GPIO for the assert command */
    fwk_id_t gpio_assert_id;

    /*! Identifier of the GPIO for the system mode selection */
    fwk_id_t gpio_mode_id;
};

/*!
 * \brief Element type.
 */
enum mod_juno_xrp7724_element_type {
    MOD_JUNO_XRP7724_ELEMENT_TYPE_GPIO,
    MOD_JUNO_XRP7724_ELEMENT_TYPE_SENSOR,
    MOD_JUNO_XRP7724_ELEMENT_TYPE_PSU,
    MOD_JUNO_XRP7724_ELEMENT_TYPE_COUNT,
};

/*!
 * \brief Element configuration
 */
struct mod_juno_xrp7724_dev_config {
    /*! Identifier of the element for the driver response */
    fwk_id_t driver_response_id;

    /*! Identifier of the driver response API */
    fwk_id_t driver_response_api_id;

    /*!
     * \brief Identifier of the ADC sensor associated with the PSU channel.
     *
     * \note Only provided for a PSU element
     */
    fwk_id_t psu_adc_id;

    /*!
     * \brief Index of the bus associated to the PSU.
     *
     * \note Only provided for a PSU element
     */
    uint8_t psu_bus_idx;

    /*!
     * \brief Sensor information
     *
     * \note Only provided for a sensor element
     */
    struct mod_sensor_info *sensor_info;

    /*! Element type */
    enum mod_juno_xrp7724_element_type type;

    /*!
     * \brief Identifier of the alarm.
     *
     * \note  When setting the voltage, it is necessary to wait for the PSU to
     * \note  stabilize at the new voltage. We use the Timer HAL to insert a
     * \note  short delay for this.
     */
    fwk_id_t alarm_hal_id;
};

/*! API for system mode */
struct mod_juno_xrp7724_api_system_mode {
    /*!
     * \brief Perform a shutdown.
     *
     * \details When the function returns the request may not be completed.
     *      When the operation has successfully finished, no response will be
     *      returned to caller since the hardware will be entirely turned off.
     *      The caller may get a response through an event in case the request
     *      failed.
     *
     * \retval FWK_PENDING The request was submitted, not yet completed.
     * \return One of the standard framework error codes.
     */
    int (*shutdown)(void);

    /*!
     * \brief Perform a cold reset.
     *
     * \details When the function returns the request may not be completed.
     *      When the operation has successfully finished, no response will be
     *      returned to caller since the hardware will be restarted.
     *      The caller may get a response through an event in case the request
     *      failed.

     * \retval FWK_PENDING The request was submitted, not yet completed.
     * \return One of the standard framework error codes.
     */
    int (*reset)(void);
};

/*! Index of the available APIs */
enum mod_juno_xrp7724_api_idx {
    MOD_JUNO_XRP7724_API_IDX_SENSOR,
    MOD_JUNO_XRP7724_API_IDX_SYSTEM_MODE,
    MOD_JUNO_XRP7724_API_IDX_PSU,
    MOD_JUNO_XRP7724_API_IDX_COUNT,
};

/*! Identifier of the system mode API */
static const fwk_id_t mod_juno_xrp7724_api_id_system_mode = FWK_ID_API_INIT(
    FWK_MODULE_IDX_JUNO_XRP7724, MOD_JUNO_XRP7724_API_IDX_SYSTEM_MODE);

/*! Identifier of the sensor driver API */
static const fwk_id_t mod_juno_xrp7724_api_id_sensor = FWK_ID_API_INIT(
    FWK_MODULE_IDX_JUNO_XRP7724, MOD_JUNO_XRP7724_API_IDX_SENSOR);

/*! Identifier of the PSU driver API */
static const fwk_id_t mod_juno_xrp7724_api_id_psu = FWK_ID_API_INIT(
    FWK_MODULE_IDX_JUNO_XRP7724, MOD_JUNO_XRP7724_API_IDX_PSU);

/*!
 * \}
 */

#endif /* MOD_JUNO_XRP7724_H */

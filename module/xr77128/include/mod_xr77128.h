/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power Management IC (XR77128) module.
 */

#ifndef MOD_XR77128_H
#define MOD_XR77128_H

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupModuleXR77128 XR77128 Driver
 *
 * \brief Driver support for XR77128
 *
 * \details This module provides support for SCP to Power Management IC (PMIC)
 *      and clocks I2C communication.
 *
 * \{
 */

/*!
 * \brief PMIC Power commands - commands to control, monitor and adjust
 * power parameters in a running system.
 */
enum mod_xr77128_power_cmd {
    /*! Read voltage Ch1 */
    XR77128_PWR_READ_VOLTAGE_CH1 = 0x10,
    XR77128_PWR_READ_VOLTAGE_CHx = XR77128_PWR_READ_VOLTAGE_CH1,
    /*! Read voltage Ch2 */
    XR77128_PWR_READ_VOLTAGE_CH2 = 0x11,
    /*! Read voltage Ch3 */
    XR77128_PWR_READ_VOLTAGE_CH3 = 0x12,
    /*! Read voltage Ch4 */
    XR77128_PWR_READ_VOLTAGE_CH4 = 0x13,
    /*! Read voltage Vin */
    XR77128_PWR_READ_VOLTAGE_VIN = 0x14,
    /*! Read voltage VTj */
    XR77128_PWR_READ_TEMP_VTJ = 0x15,
    /*! Read ViL Ch1 */
    XR77128_PWR_READ_CURRENT_CH1 = 0x16,
    XR77128_PWR_READ_CURRENT_CHx = XR77128_PWR_READ_CURRENT_CH1,
    /*! Read ViL Ch2 */
    XR77128_PWR_READ_CURRENT_CH2 = 0x17,
    /*! Read ViL Ch3 */
    XR77128_PWR_READ_CURRENT_CH3 = 0x18,
    /*! Read ViL Ch4 */
    XR77128_PWR_READ_CURRENT_CH4 = 0x19,
    /*! PWREN groups enable / disable */
    XR77128_PWR_ENABLE_SUP_GROUP = 0x1D,
    /*! Supply channel enable / disable */
    XR77128_PWR_ENABLE_SUP = 0x1E,
    /*! Set voltage Ch1 */
    XR77128_PWR_SET_VOLTAGE_CH1 = 0x20,
    XR77128_PWR_SET_VOLTAGE_CHx = XR77128_PWR_SET_VOLTAGE_CH1,
    /*! Set voltage Ch2 */
    XR77128_PWR_SET_VOLTAGE_CH2 = 0x21,
    /*! Set voltage Ch3 */
    XR77128_PWR_SET_VOLTAGE_CH3 = 0x22,
    /*! Set voltage Ch4 */
    XR77128_PWR_SET_VOLTAGE_CH4 = 0x23,
    /*! Set current Ch1 */
    XR77128_PWR_SET_CURRENT_CH1 = 0x24,
    XR77128_PWR_SET_CURRENT_CHx = XR77128_PWR_SET_CURRENT_CH1,
    /*! Set current Ch2 */
    XR77128_PWR_SET_CURRENT_CH2 = 0x25,
    /*! Set current Ch3 */
    XR77128_PWR_SET_CURRENT_CH3 = 0x26,
    /*! Set current Ch4 */
    XR77128_PWR_SET_CURRENT_CH4 = 0x27,
    /* RESERVED 0x28 */
    /* RESERVED 0x29 */
    /*! Power OK configuration */
    XR77128_PWR_POWER_OK_CFG = 0x2A
};

/*!
 * \brief XR77128 channel configuration
 */
struct mod_xr77128_channel_config {
    /*! Identifier of the element for the driver response */
    fwk_id_t driver_response_id;

    /*! Identifier of the driver response API */
    fwk_id_t driver_response_api_id;

    /*!
     * \brief Identifier of the alarm.
     *
     * \note  When setting the voltage, it is necessary to wait for the PSU to
     * \note  ramp up to the new voltage. We use the Timer HAL to insert a
     * \note  short delay in order to account for the ramp-up delay.
     */
    fwk_id_t alarm_hal_id;

    /*! Maximum supported voltage for a given PSU rail (in mV) */
    uint32_t psu_max_vout;

    /*!
     * \brief Index of the bus associated to the PSU.
     *
     * \note Only provided for a PSU element
     */
    uint8_t psu_bus_idx;

    /*! Indicates if the PMIC channel is already enabled */
    bool enabled;
};

/*!
 * \brief XR77128 device configuration
 */
struct mod_xr77128_dev_config {
    /*! Identifier of I2C HAL */
    fwk_id_t i2c_hal_id;

    /*! Identifier of I2C API */
    fwk_id_t i2c_api_id;

    /*! Channel(s) configuration */
    struct mod_xr77128_channel_config *channels;

    /*! Target address of the I2C device */
    uint8_t target_addr;
};

/*!
 * \brief Module API indices
 */
enum mod_xr77128_api_idx {
    /*! Index of the PMIC PSU API */
    MOD_XR77128_API_IDX_PSU,
    /*! Number of APIs */
    MOD_XR77128_API_COUNT
};

/*! Maximum required length for the I2C communication */
#define XR77128_DATA_SIZE_MAX 3

/*! Maximum required length for the I2C transmissions */
#define XR77128_TRANSMIT_DATA_MAX 3

/*! Maximum required length for the I2C receptions */
#define XR77128_RECEIVE_DATA_MAX 2

/*!
 * XR77128 channel count
 */
#define XR77128_CHANNEL_COUNT 4

/*!
 * Maximum fine adjustment possible to set the desired voltage
 */
#define XR77128_MAX_FINE_ADJUST 0x7

/*!
 * Fine adjustment bit position
 */
#define XR77128_FINE_ADJUST_POS 12

/*!
 * \}
 */

/*!
 * \}
 */
#endif /* MOD_XR77128_H */

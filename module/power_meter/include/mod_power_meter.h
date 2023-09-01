/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Power meter module header.
 */

#ifndef MOD_POWER_METER_H
#define MOD_POWER_METER_H

#include <fwk_id.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupPowerMeter Power meter
 *
 * \details Responsible for power measurements
 *
 * \{
 */

/*!
 * \brief Power meter device configuration.
 */
struct mod_power_meter_dev_config {
    /*!
     * \brief Module or element identifier of the driver
     */
    fwk_id_t driver_id;
    /*!
     * \brief API identifier of the driver.
     */
    fwk_id_t driver_api_id;
};

/*!
 * \brief Power meter driver interface.
 */
struct mod_power_meter_driver_api {
    /*!
     * \brief Get the power measurement.
     *
     * \param id Specific power meter device ID.
     * \param[out] power Power measured by the power meter device.
     *
     * \retval ::FWK_SUCCESS The cap is returned successfully.
     */
    int (*get_power)(fwk_id_t id, uint32_t *power);
};

/*!
 * \brief Power meter interface.
 */
struct mod_power_meter_api {
    /*!
     * \brief Get the power measurement.
     *
     * \param id Specific power meter device ID.
     * \param[out] power Power measured by the power meter device.
     *
     * \retval ::FWK_SUCCESS The cap is returned successfully.
     */
    int (*get_power)(fwk_id_t id, uint32_t *power);

    /*!
     * \brief Set the power change notification hysteresis.
     *
     * \details The hysteresis is needed to avoid multiple unneeded
     *     notifications for a fluctuating power measurement.
     *
     * \param id Specific power meter device ID.
     * \param threshold_low The lower threshold of the measured power to trigger
     *     a notification.
     *
     * \param threshold_high The higher threshold of the measured power to
     *     trigger a notification.
     *
     * \retval ::FWK_SUCCESS The threshold values are set successfully.
     */
    int (*set_power_change_notif_thresholds)(
        fwk_id_t id,
        uint32_t threshold_low,
        uint32_t threshold_high);
};

/*!
 * \brief API indices.
 */
enum mod_power_meter_api_idx {
    /*! Get power measurements */
    MOD_POWER_METER_API_IDX_MEASUREMENT,

    /*! Number of defined APIs. */
    MOD_POWER_METER_API_IDX_COUNT,
};

/*!
 * \brief Power meter notification indices.
 */
enum mod_power_meter_notification_idx {
    /*! Measured power changed notification. */
    MOD_POWER_METER_NOTIFICATION_IDX_MEASUREMENTS_CHANGED,

    /*! Number of defined notifications. */
    MOD_POWER_METER_NOTIFICATION_IDX_COUNT,
};

/*!
 * \}
 */

/*!
 * \}
 */
#endif /* MOD_POWER_METER_H */

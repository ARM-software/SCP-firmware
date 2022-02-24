/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI sensor domain management protocol support.
 */

#ifndef MOD_SCMI_SENSOR_H
#define MOD_SCMI_SENSOR_H

#include <fwk_id.h>
#include <fwk_macros.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupSCMI_SENSOR SCMI Sensor Protocol
 * \{
 */

/*!
 * \brief Sensor trip point event notification API.
 *
 * \details API used by the sensor module to notify the agents through the scmi
 *      interface when a sensor trip point is triggered.
 */
struct mod_sensor_trip_point_api {
    /*!
     * \brief Inform the HAL that a sensor trip point is triggered.
     *
     * \param sensor_id Specific sensor Id.
     * \param state Trip point state.
     * \param trip_point_id trip point index.
     */
    void (*notify_sensor_trip_point)(
        fwk_id_t sensor_id,
        uint32_t state,
        uint32_t trip_point_idx);
};

/*!
 * \brief SCMI Sensor APIs.
 *
 * \details APIs exported by SCMI Sensor Protocol.
 */
enum scmi_sensor_api_idx {
    /*! Index for the SCMI Sensor request API */
    SCMI_SENSOR_API_IDX_REQUEST,

    /*! Index for the SCMI Sensor trip point API */
    SCMI_SENSOR_API_IDX_TRIP_POINT,

    /*! Number of APIs */
    SCMI_SENSOR_API_IDX_COUNT,
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SCMI_SENSOR_H */

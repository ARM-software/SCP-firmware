/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) Sensor
 *      requester support.
 */

#ifndef MOD_SCMI_SENSOR_REQ_H
#define MOD_SCMI_SENSOR_REQ_H

#include <fwk_id.h>
#include <fwk_macros.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupSCMISensorRequester SCMI Sensor Protocol Requester
 *
 * \brief SCMI Sensor Protocol requester implementation.
 *
 * \{
 */

/*!
 * \brief Sensor requester get value mode
 */
enum scmi_sensor_req_async_flag {
    SENSOR_REQ_SYNC = 0,
    SENSOR_REQ_ASYNC,
};

/*!
 * \brief SCMI sensor requester configuration
 */
struct scmi_sensor_req_config {
    /*!
     * \brief SCMI Service ID
     *
     * \details The service ID which corresponds to the required
     *      channel in the transport layer.
     */
    fwk_id_t service_id;
    /*!
     * \brief SCMI Sensor ID
     *
     * \details This is the sensor ID exposed by SCMI on the platform side.
     */
    uint32_t scmi_sensor_id;
    /*!
     * \brief Sensor HAL identifier
     *
     * \details This the identifier for the HAL module that communicates
     *      directly with this module within the same entity.
     */
    fwk_id_t sensor_hal_id;
    /*!
     * \brief Async flag
     */
    enum scmi_sensor_req_async_flag async_flag;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SCMI_SENSOR_REQ_H */

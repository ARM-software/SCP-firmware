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

#ifndef INTERNAL_SCMI_SENSOR_REQ_H
#define INTERNAL_SCMI_SENSOR_REQ_H

#include <stdint.h>
/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupSCMISensorRequester SCMI Sensor Management Protocol
 * \{
 */
#define SCMI_PROTOCOL_VERSION_SENSOR UINT32_C(0x30000)

/*
 * SENSOR_READING_GET
 */

struct scmi_sensor_protocol_reading_get_a2p {
    uint32_t sensor_id;
    uint32_t flags;
};

struct scmi_sensor_protocol_reading_get_p2a {
    int32_t status;
    uint32_t sensor_value_low;
    uint32_t sensor_value_high;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* INTERNAL_SCMI_SENSOR_REQ_H */

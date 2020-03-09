/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SENSOR_H
#define SENSOR_H

#include <mod_sensor.h>

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \cond
 * @{
 */

/*
 * Sensor element context
 */
struct sensor_dev_ctx {
    struct mod_sensor_dev_config *config;
    struct mod_sensor_driver_api *driver_api;
    uint32_t cookie;
    bool read_busy;
};

/*
 * Sensor event indexes
 */
enum mod_sensor_event_idx {
    SENSOR_EVENT_IDX_READ_REQUEST = MOD_SENSOR_EVENT_IDX_READ_REQUEST,
    SENSOR_EVENT_IDX_READ_COMPLETE,
    SENSOR_EVENT_IDX_COUNT
};

/*
 * Event identifiers
 */
static const fwk_id_t mod_sensor_event_id_read_complete =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_SENSOR,
                      SENSOR_EVENT_IDX_READ_COMPLETE);

/*!
 * \endcond
 * @}
 */

#endif /* SENSOR_H */

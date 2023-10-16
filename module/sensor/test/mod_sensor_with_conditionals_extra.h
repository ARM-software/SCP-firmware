/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Sensor unit test support.
 */

#include <sensor.h>

#include <mod_sensor.h>

#include <fwk_id.h>

int sensor_set_timestamp_config(
    fwk_id_t id,
    const struct mod_sensor_timestamp_info *config);

int sensor_get_timestamp_config(
    fwk_id_t id,
    struct mod_sensor_timestamp_info *config);

int sensor_get_axis_info(
    fwk_id_t id,
    uint32_t axis,
    struct mod_sensor_axis_info *info);

int sensor_axis_start(fwk_id_t id);

int sensor_timestamp_dev_init(fwk_id_t id, struct sensor_dev_ctx *ctx);

uint64_t sensor_get_timestamp(fwk_id_t id);

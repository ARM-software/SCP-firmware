/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Thermal Management unit test support.
 */
#include <mod_sensor.h>
#include <mod_thermal_mgmt.h>

/*! Thermal protection warning alarm callback. */
void mod_thermal_mgmt_protection_api_warning(
    fwk_id_t driver_id,
    fwk_id_t thermal_id);

/*! Thermal protection critical alarm callback. */
void mod_thermal_mgmt_protection_api_critical(
    fwk_id_t driver_id,
    fwk_id_t thermal_id);

/*! Read sensor data. */
int mod_sensor_get_data(fwk_id_t id, struct mod_sensor_data *data);

/* Power allocation */
void distribute_power(
    fwk_id_t id,
    const uint32_t *perf_request,
    uint32_t *perf_limit);

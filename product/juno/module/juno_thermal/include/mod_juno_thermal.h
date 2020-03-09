/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Juno Thermal Protection
 */
#ifndef MOD_JUNO_THERMAL_H
#define MOD_JUNO_THERMAL_H

#include <fwk_id.h>

/*!
 * \ingroup GroupJunoModule Juno Product Modules
 * \defgroup GroupThermalProtection Thermal Protection
 * \{
 */

/*!
 * \brief Element configuration.
 */
struct mod_juno_thermal_element_config {
    /*!
     * \brief The temperature threshold in millidegrees celsius.
     */
    uint64_t thermal_threshold_mdc;

    /*!
     * \brief The alarm interval in milliseconds.
     */
    unsigned int period_ms;

    /*!
     * \brief Identifier of the sensor element to read the temperature from.
     */
    fwk_id_t sensor_id;

    /*!
     * \brief Identifier of the alarm for reading the temperature periodically.
     */
    fwk_id_t alarm_id;
};

/*!
 * \}
 */

#endif /* MOD_JUNO_THERMAL_H */

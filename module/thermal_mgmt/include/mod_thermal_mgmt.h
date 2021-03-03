/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_THERMAL_MGMT_H
#define MOD_THERMAL_MGMT_H

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \ingroup GroupModules
 * \defgroup GroupThermal Thermal Management
 *
 * \details Module for controlling/limiting the Performance of a platform based
 *      on thermal input.
 *
 * \{
 */

/*!
 * \brief Number of defined APIs for Thermal Management Module.
 */
enum mod_thermal_api_idx {
    /*!
     * \brief Performance Update API index.
     *
     * \note This API implements the ::perf_plugins_api interface.
     */
    MOD_THERMAL_API_PERF_UPDATE_IDX,

    MOD_THERMAL_API_COUNT,
};

/*!
 * \brief Thermal Mgmt device configuration.
 *
 * \details Configuration structure for individual thermal devices.
 *      Please refer to the doc section for further details.
 */
struct mod_thermal_mgmt_dev_config {
    /*! Element identifier of the corresponding power model */
    fwk_id_t driver_id;

    /*! Power Model API identifier */
    fwk_id_t driver_api_id;

    /*! Corresponding DVFS Element identifier for this device */
    fwk_id_t dvfs_domain_id;

    /*!
     * \brief Weight coefficient for power distribution.
     *
     * \details Its value can be any. It expresses the relative weight to other
     *      devices.
     */
    uint16_t weight;
};

/*!
 * \brief Thermal Mgmt configuration.
 *
 * \details Configuration structure for the whole thermal management.
 *      Please refer to the doc section for further details.
 */
struct mod_thermal_mgmt_config {
    /*!
     * \brief Slow loop multiplier.
     *
     * \details The slow loop time interval is derived as follow:
     *      slow_loop_period = slow_loop_mult * perf_update_period.
     */
    unsigned int slow_loop_mult;

    /*! The thermal design power (TDP) for all the devices being controlled */
    uint16_t tdp;

    /*!
     * \brief Switch-on temperature threshold.
     *
     * \details The temperature above which the PI loop runs. Below this
     *      threshold the power is allocated only on bias coefficients.
     */
    uint32_t switch_on_temperature;

    /*!
     * \brief Control temperature
     *
     * \details The temperature that the system will achive once stabilised.
     *      Due to the PI nature of the controller, some overshoot/undershoot
     *      may occur.
     *      Note that the controller can only limit the temperature by placing a
     *      limit to the power to the heat source. It has no direct control on
     *      the heat source itself and therefore only the upper limit can be
     *      controlled.
     */
    uint32_t control_temperature;

    /*!
     * \brief Integral cut-off threshold.
     *
     * \details Below this value the errors are accumulated. This is useful to
     *      avoid accumulating errors when the temperature is below the target.
     */
    int32_t integral_cutoff;

    /*!
     * \brief Integral maximum.
     *
     * \details This is the upper limit the accumulated errors.
     */
    int32_t integral_max;

    /*! Proportional term when undershooting (PI loop)*/
    int32_t k_p_undershoot;

    /*! Proportional term when overhooting (PI loop) */
    int32_t k_p_overshoot;

    /*! Integral term (PI loop) */
    int32_t k_integral;

    /*! Temperature sensor identifier */
    fwk_id_t sensor_id;
};

/*!
 * \brief Power Model API.
 *
 * \ details This API must be implemented by the platform.
 */
struct mod_thermal_mgmt_driver_api {
    /*!
     * \brief Performance Level to Power conversion.
     *
     * \details Convert a performance level into its corresponding power usage.
     *
     * \param domain_id Specific power model device id.
     * \param level Performance level
     *
     * \retval power The corresponding power for a given performance level.
     */
    uint32_t (*level_to_power)(fwk_id_t domain_id, const uint32_t level);

    /*!
     * \brief Power to Performance Level conversion.
     *
     * \details Convert a power usage into its corresponding performance level.
     *
     * \param domain_id Specific power model device id.
     * \param power Power
     *
     * \retval level The corresponding performance level for a given power.
     */
    uint32_t (*power_to_level)(fwk_id_t domain_id, const uint32_t power);
};

/*!
 * \}
 */

#endif /* MOD_THERMAL_MGMT_H */

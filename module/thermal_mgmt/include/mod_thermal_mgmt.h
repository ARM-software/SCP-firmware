/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
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
 * \brief Thermal Mgmt activity factor configuration.
 *
 * \details Configuration structure for activity factor.
 */
struct mod_thermal_mgmt_activity_factor_config {
    /*! Activity factor driver identifier */
    fwk_id_t driver_id;

    /*! Activity factor driver API identifier */
    fwk_id_t driver_api_id;
};

/*!
 * \brief Thermal Mgmt actors configuration.
 *
 * \details Configuration structure for individual thermal actors.
 *      Please refer to the doc section for further details.
 */
struct mod_thermal_mgmt_actor_config {
    /*! Element identifier of the corresponding power model */
    fwk_id_t driver_id;

    /*! Corresponding DVFS Element identifier for this device */
    fwk_id_t dvfs_domain_id;

    /*!
     * \brief Weight coefficient for power distribution.
     *
     * \details Its value can be any. It expresses the relative weight to other
     *      devices.
     */
    uint16_t weight;

    /*! Activity factor configuration */
    struct mod_thermal_mgmt_activity_factor_config *activity_factor;
};

/*!
 * \brief Thermal Mgmt protection configuration.
 *
 * \details Configuration structure for thermal protection.
 */
struct mod_thermal_mgmt_protection_config {
    /*! Thermal protection driver identifier */
    fwk_id_t driver_id;

    /*! Thermal protection driver API identifier*/
    fwk_id_t driver_api_id;

    /*!
     * \brief Warning temperature value threshold.
     *
     * \details Warning temperature threshold value that activates the thermal
     *      protection. The corresponding callback is invoked when the
     *      temperature is above the threshold.
     */
    uint32_t warn_temp_threshold;

    /*!
     * \brief Critical temperature threshold value.
     *
     * \details Critical temperature threshold value that activates the thermal
     *      protection. It is expected that `warn_temp_threshold` >
     *      `crit_temp_threshold`. The corresponding callback is invoked when
     *      the temperature is above the threshold.
     */
    uint32_t crit_temp_threshold;
};

/*!
 * \brief Thermal Mgmt device configuration.
 *
 * \details Configuration structure for individual thermal devices.
 *      Please refer to the doc section for further details.
 */
struct mod_thermal_mgmt_dev_config {
    /*!
     * \brief Slow loop multiplier.
     *
     * \details The slow loop time interval is derived as follow:
     *      slow_loop_period = slow_loop_mult * perf_update_period.
     */
    unsigned int slow_loop_mult;

    /*! The thermal design power (TDP) for all the devices being controlled */
    uint16_t tdp;

    /*! The cold state power */
    uint16_t cold_state_power;

    /*!
     * \brief Switch-on temperature threshold.
     *
     * \details The temperature above which the PID loop runs. Below this
     *      threshold the power is allocated only on bias coefficients.
     */
    struct {
        /*!
         * \brief Switch-on temperature threshold.
         *
         * \details The temperature above which the PID loop runs. Below this
         *      threshold the power is allocated only on bias coefficients.
         */
        uint32_t switch_on_temperature;

        /*!
         * \brief Control temperature
         *
         * \details The temperature that the system will achive once stabilised.
         *      Due to the PID nature of the controller, some
         *      overshoot/undershoot may occur. Note that the controller can
         *      only limit the temperature by placing a limit to the power to
         *      the heat source. It has no direct control on the heat source
         *      itself and therefore only the upper limit can be controlled.
         */
        uint32_t control_temperature;

        /*!
         * \brief Integral cut-off threshold.
         *
         * \details Below this value the errors are accumulated. This is useful
         *      to avoid accumulating errors when the temperature is below the
         *      target.
         */
        int32_t integral_cutoff;

        /*!
         * \brief Integral maximum.
         *
         * \details This is the upper limit the accumulated errors.
         */
        int32_t integral_max;

        /*! Proportional term when undershooting (PID loop)*/
        int32_t k_p_undershoot;

        /*! Proportional term when overhooting (PID loop) */
        int32_t k_p_overshoot;

        /*! Integral term (PID loop) */
        int32_t k_integral;

        /*! Derivative term (PID loop) */
        int32_t k_derivative;
    } pid_controller;

    /*! Temperature sensor identifier */
    fwk_id_t sensor_id;

    /*!
     * \brief Thermal protection configuration.
     *
     * \details It is an optional feature. If it is left NULL it will not
     *      operate.
     */
    struct mod_thermal_mgmt_protection_config *temp_protection;

    /*! Power Model API identifier */
    fwk_id_t driver_api_id;

    /*! Thermal actors lookup table */
    struct mod_thermal_mgmt_actor_config *thermal_actors_table;

    /*!
     * \brief The number of actors in the thermal actors lookup table.
     *
     * \details If it is left zero it will not run the PID controller and power
     *      distribution will not take place.
     */
    uint32_t thermal_actors_count;
};

/*!
 * \brief Power Model API.
 *
 * \details This API must be implemented by the platform.
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
 * \brief Thermal management temperature protection API.
 *
 * \ details This API must be implemented by the platform.
 */
struct mod_thermal_mgmt_protection_api {
    /*!
     * \brief Thermal protection warning alarm callback.
     *
     * \details Informs once every time the temperature exceeds
     *      `warn_temp_threshold` value. The periodicity of this callback
     *      is not more often than `slow_loop_mult`.
     *
     * \param driver_id Specific driver identifier.
     * \param thermal_id Specific thermal management controller device
     *      identifier.
     */
    void (*warning)(fwk_id_t driver_id, fwk_id_t thermal_id);

    /*!
     * \brief Thermal protection critical alarm callback.
     *
     * \details Informs once every time the temperature exceeds
     *      `crit_temp_threshold` value. The periodicity of this callback
     *      is not more often than `slow_loop_mult`.
     *
     * \param driver_id Specific driver identifier.
     * \param thermal_id Specific thermal management controller device
     *      identifier.
     */
    void (*critical)(fwk_id_t driver_id, fwk_id_t thermal_id);
};

/*!
 * \brief Activity counter API.
 *
 * \details This API must be implemented by the platform.
 */
struct mod_thermal_mgmt_activity_factor_api {
    /*!
     * \brief Gets activity factor from the specified domain.
     *
     * \details Activity factor is referred to the corresponding actor and
     *      implemented by the platform. It must return a 10 bits
     *      resolution value (0-1023) where 0 corresponds to a complete
     *      inactivity and 1023 to 100% activity.
     *
     * \param domain_id Specific device id.
     * \param[out] activity activity factor normalized to 10 bits.
     *
     * \retval ::FWK_E_PARAM One or more parameters were invalid.
     * \retval ::FWK_SUCCESS The operation succeeded.
     *
     * \return Status code representing the result of the operation.
     */
    int (*get_activity_factor)(fwk_id_t domain_id, uint16_t *activity);
};

/*!
 * \}
 */

#endif /* MOD_THERMAL_MGMT_H */

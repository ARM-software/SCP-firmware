/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_VOLTAGE_DOMAIN_H
#define MOD_VOLTAGE_DOMAIN_H

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupVoltageDomain Voltage domain HAL
 *
 * \details A Hardware Abstraction Layer for controlling voltage
 *     domains regulators.
 * @{
 */

/*!
 * \brief Voltage domain mode IDs.
 */
enum mod_voltd_mode_id {
    /*! Voltage domain is Off */
    MOD_VOLTD_MODE_ID_OFF,

    /*! Voltage domain is On */
    MOD_VOLTD_MODE_ID_ON,
};

/*!
 * \brief Voltage domain mode types of operation.
 */
enum mod_voltd_mode_type {
    /*! Voltage domain is in Architectural mode */
    MOD_VOLTD_MODE_TYPE_ARCH,

    /*! Voltage domain is in Implementation defined mode */
    MOD_VOLTD_MODE_TYPE_IMPL
};

/*!
 * \brief APIs that the module makes available to entities requesting binding.
 */
enum mod_voltd_api_type {
    /*! Voltage Domain (voltd) HAL */
    MOD_VOLTD_API_TYPE_HAL,

    /*! Number of defined APIs */
    MOD_VOLTD_API_COUNT,
};

/*!
 * \brief Voltage level description types.
 */
enum mod_voltd_voltage_level_type {
    /*! The voltage domain has a discrete set of levels that it can attain */
    MOD_VOLTD_VOLTAGE_LEVEL_DISCRETE,

    /*!
     * The voltage domain has a continuous range of levels with a constant step
     */
    MOD_VOLTD_VOLTAGE_LEVEL_CONTINUOUS,
};

/*!
 * \brief Voltage domain element configuration data.
 */
struct mod_voltd_dev_config {
    /*! Reference to the device element within the associated driver module */
    const fwk_id_t driver_id;

    /*! Reference to the API provided by the device driver module */
    const fwk_id_t api_id;
};

/*!
 * \brief Range of supported levels for a voltage domain.
 */
struct mod_voltd_range {
    /*! The type of level range description provided (discrete or continuous) */
    enum mod_voltd_voltage_level_type level_type;

    /*! Minimum voltage level (in uV) */
    int32_t min_uv;

    /*! Maximum voltage level (in uV) */
    int32_t max_uv;

    /*!
     * The number of uV by which the level can be incremented at each step
     * throughout the domain's range. Valid only when level_type is equal to
     * \ref mod_voltd_voltage_level_type.MOD_VOLTD_VOLTAGE_LEVEL_CONTINUOUS.
     */
    int32_t step_uv;

    /*! The number of unique voltage levels that the domain can attain */
    size_t level_count;
};

/*!
 * \brief Voltage domain properties exposed via the get_info() API function.
 *
 * \details This structure is intended to store voltage domain information that
 *     is static and which does not change during runtime. Dynamic information,
 *     such as the current voltage domain mode id, are exposed through
 *     functions in the voltage domain and voltage domain driver APIs.
 */
struct mod_voltd_info {
    /*! Human-friendly voltage domain name */
    const char *name;

    /*! Range of supported voltage levels */
    struct mod_voltd_range level_range;
};

/*!
 * \brief Voltage domain driver interface.
 */
struct mod_voltd_drv_api {

    /*!
     * \brief Set a new voltage level by providing a a level in microvolts.
     *
     * \param voltd_id Voltage domain device identifier.
     *
     * \param level_uv The desired voltage in microvolt.
     *
     * \retval FWK_PENDING The request is pending. The driver will provide the
     *      requested value later through the driver response API.
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*set_level)(fwk_id_t voltd_id, int32_t level_uv);

    /*!
     * \brief Get the current voltage level of a domain in microvolts (uV).
     *
     * \param voltd_id Voltage domain device identifier.
     *
     * \param[out] level_uv The current voltage level in microvolts.
     *
     * \retval FWK_PENDING The request is pending. The driver will provide the
     *      requested value later through the driver response API.
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*get_level)(fwk_id_t voltd_id, int32_t *level_uv);

    /*!
     * \brief Set the running configuration of a voltage domain.
     *
     * \param voltd_id Voltage domain device identifier.
     *
     * \param mode_type Mode type of voltage domain operation either
     *      Architectural or Implementation Defined.
     *
     * \param mode_id One of the valid voltage domain mode IDs.
     *
     * \retval FWK_PENDING The request is pending. The driver will provide the
     *      requested value later through the driver response API.
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*set_config)(fwk_id_t voltd_id, uint8_t mode_type, uint8_t mode_id);

    /*!
     * \brief Get the running configuration of a voltage domain.
     *
     * \param voltd_id Voltage domain device identifier.
     *
     * \param[out] mode_type The current mode type of operation.
     *
     * \param[out] mode_id The current voltage domain mode ID.
     *
     * \retval FWK_PENDING The request is pending. The driver will provide the
     *      requested value later through the driver response API.
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*get_config)(fwk_id_t voltd_id, uint8_t *mode_type, uint8_t *mode_id);

    /*!
     * \brief Get information a voltage domain element.
     *
     * \param voltd_id Voltage domain device identifier.
     *
     * \param[out] info The voltage domain information.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
     int (*get_info)(fwk_id_t voltd_id, struct mod_voltd_info *info);

    /*!
     * \brief Get voltage level in microvolts from an index in level's range.
     *
     * \param elt_id Voltage domain device identifier.
     *
     * \param index The index into the domain level's range.
     *
     * \param[out] level_uv The voltage level, in microvolts, for the index.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
     int (*get_level_from_index)(
         fwk_id_t elt_id,
         unsigned int index,
         int32_t *level_uv);
};

/*!
 * \brief Voltage domain interface.
 */
struct mod_voltd_api {
    /*!
     * \brief Set a new voltage level by providing a level in microvolts (uV).
     *
     * \param voltd_id Voltage domain device identifier.
     *
     * \param level_uv The desired voltage level in microvolts (uV).
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_PENDING The request is pending. The result for this operation
     *      will be provided via a response event.
     * \retval FWK_E_PARAM The voltage domain identifier was invalid.
     * \retval FWK_E_SUPPORT Deferred handling of asynchronous drivers is not
     *      supported.
     * \return One of the standard framework error codes.
     */
    int (*set_level)(fwk_id_t voltd_id, int32_t level_uv);

    /*!
     * \brief Get the current level of a voltd in micro volt (uV).
     *
     * \param voltd_id Voltage domain device identifier.
     *
     * \param[out] level_uv The current voltage level in microvolts.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_PENDING The request is pending. The requested level will be
     *      provided via a response event.
     * \retval FWK_E_PARAM The voltage domain identifier was invalid or
     *      the level pointer was NULL.
     * \retval FWK_E_SUPPORT Deferred handling of asynchronous drivers is not
     *      supported.
     * \return One of the standard framework error codes.
     */
    int (*get_level)(fwk_id_t voltd_id, int32_t *level_uv);

    /*!
     * \brief Set the running configuration of a voltage domain.
     *
     * \param voltd_id Voltage domain device identifier.
     *
     * \param mode_type Mode type of voltage domain operation either
     *      Architectural or Implementation Defined.
     *
     * \param mode_id One of the valid voltage domain mode IDs.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_PENDING The request is pending. The result for this operation
     *      will be provided via a response event.
     * \retval FWK_E_PARAM The voltage domain identifier was invalid.
     * \retval FWK_E_SUPPORT Deferred handling of asynchronous drivers is not
     *      supported.
     * \return One of the standard framework error codes.
     */
    int (*set_config)(fwk_id_t voltd_id, uint8_t mode_type, uint8_t mode_id);

    /*!
     * \brief Get the running configuration of a voltage domain.
     *
     * \param voltd_id Voltage domain device identifier.
     *
     * \param[out] mode_type Mode type of operation either Architectural or
     *      Implementation Defined.
     *
     * \param[out] mode_id The current voltage domain mode ID.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_PENDING The request is pending. The requested configuration
     *      will be provided via a response event.
     * \retval FWK_E_PARAM The voltage domain identifier was invalid,
     *      the mode_id pointer was NULL or the mode_type pointer was NULL.
     * \retval FWK_E_SUPPORT Deferred handling of asynchronous drivers is not
     *      supported.
     * \return One of the standard framework error codes.
     */
    int (*get_config)(fwk_id_t voltd_id, uint8_t *mode_type, uint8_t *mode_id);

    /*!
     * \brief Get information about a voltage domain element.
     *
     * \param voltd_id Voltage domain device identifier.
     *
     * \param[out] info The voltage domain information.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard framework error codes.
     */
    int (*get_info)(fwk_id_t voltd_id, struct mod_voltd_info *info);

    /*!
     * \brief Get a voltage level in microvolts from an index in the range.
     *
     * \param voltd_id Voltage domain device identifier.
     *
     * \param index The index into the domain's voltage level's range.
     *
     * \param[out] level_uv The voltage level, in microvolts.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM The voltage domain identifier was invalid.
     *      the level pointer was NULL.
     * \return One of the standard framework error codes.
     */
    int (*get_level_from_index)(
        fwk_id_t voltd_id,
        unsigned int index,
        int32_t *level_uv);
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_VOLTAGE_DOMAIN_H */

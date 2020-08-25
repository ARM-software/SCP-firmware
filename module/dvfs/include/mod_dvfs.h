/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_DVFS_H
#define MOD_DVFS_H

#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules
 * \defgroup GroupDvfs Dynamic Voltage and Frequency Scaling (DVFS)
 * \{
 */

/*!
 * \defgroup GroupDvfsTypes Types
 * \{
 */

/*!
 * \brief Level limits.
 */
struct mod_dvfs_level_limits {
    uint32_t minimum; /*!< Minimum permitted level */
    uint32_t maximum; /*!< Maximum permitted level */
};

/*!
 * \brief Operating Performance Point (OPP).
 */
struct mod_dvfs_opp {
    uint32_t level; /*!< Level value of the OPP */
    uint32_t voltage; /*!< Power supply voltage in millivolts (mV) */
    uint32_t frequency; /*!< Clock rate in Hertz (Hz) */
    uint32_t power; /*!< Power draw in milliwatts (mW) */
};

/*!
 * \}
 */

/*!
 * \defgroup GroupDvfsConfig Configuration
 * \{
 */

/*!
 * \brief Domain configuration.
 */
struct mod_dvfs_domain_config {
    /*!
     * \brief Power supply identifier.
     *
     * \warning This identifier must refer to an element of the \c psu module.
     */
    fwk_id_t psu_id;

    /*!
     * \brief Clock identifier.
     *
     * \warning This identifier must refer to an element of the \c clock module.
     */
    fwk_id_t clock_id;

    /*!
     * \brief Alarm identifier.
     *
     * \warning This identifier must refer to an alarm of the \c timer module.
     */
    fwk_id_t alarm_id;

    /*!
     * \brief Notifications identifier.
     */
    fwk_id_t notification_id;

    /*!
     * \brief Updates API identifier.
     */
    fwk_id_t updates_api_id;

    /*! Delay in milliseconds before retrying a request */
    uint16_t retry_ms;

    /*! Worst-case transition latency in microseconds */
    uint16_t latency;

    /*! Sustained operating point index */
    size_t sustained_idx;

    /*!
     * \brief Operating points.
     *
     * \note The frequencies and levels of these operating points must be in
     *      ascending order.
     */
    struct mod_dvfs_opp *opps;

    /*! \brief Allow inexact performance levels.
     *
     * \note It will set any immediately higher than the requested value if it
     * is possible, otherwise it will set the highest possible.
     */
    bool approximate_level;
};

/*!
 * \}
 */

/*!
 * \defgroup GroupDvfsApis APIs
 * \{
 */

/*!
 * \brief Domain API.
 */
struct mod_dvfs_domain_api {
    /*!
     * \brief Get the current operating point of a domain.
     *
     * \param domain_id Element identifier of the domain.
     * \param [out] opp Current operating point.
     */
    int (*get_current_opp)(fwk_id_t domain_id, struct mod_dvfs_opp *opp);

    /*!
     * \brief Get the sustained operating point of a domain.
     *
     * \param domain_id Element identifier of the domain.
     * \param [out] opp Sustained operating point.
     */
    int (*get_sustained_opp)(
        fwk_id_t domain_id,
        struct mod_dvfs_opp *opp);

    /*!
     * \brief Get an operating point from its index.
     *
     * \param domain_id Element identifier of the domain.
     * \param n Index of the operating point to retrieve.
     * \param [out] opp Requested operating point.
     */
    int (*get_nth_opp)(
        fwk_id_t domain_id,
        size_t n,
        struct mod_dvfs_opp *opp);

    /*!
     * \brief Get the number of operating points of a domain.
     *
     * \param domain_id Element identifier of the domain.
     * \param [out] opp_count Number of operating points.
     */
    int (*get_opp_count)(fwk_id_t domain_id, size_t *opp_count);

    /*!
     * \brief Get the level id for the given level.
     *
     * \param domain_id Element identifier of the domain.
     * \param level Requested level.
     * \param [out] level id inside the OPP table.
     */
    int (*get_level_id)(fwk_id_t domain_id, uint32_t level, size_t *level_id);

    /*!
     * \brief Get the worst-case transition latency of a domain.
     *
     * \param domain_id Element identifier of the domain.
     * \param [out] latency Worst-case transition latency.
     */
    int (*get_latency)(fwk_id_t domain_id, uint16_t *latency);

    /*!
     * \brief Set the level of a domain.
     *
     * \param domain_id Element identifier of the domain.
     * \param cookie Context-specific value.
     * \param level Requested level.
     */
    int (*set_level)(fwk_id_t domain_id, uintptr_t cookie, uint32_t level);

    /*!
     * \brief Get the level of a domain.
     *
     * \param domain_id Element identifier of the domain.
     * \param [out] limits Current level limits.
     */
    int (*get_level_limits)(
        fwk_id_t domain_id,
        struct mod_dvfs_level_limits *limits);

    /*!
     * \brief Set the level of a domain.
     *
     * \param domain_id Element identifier of the domain.
     * \param cookie Context-specific value.
     * \param limits Pointer to the new limits.
     */
    int (*set_level_limits)(
        fwk_id_t domain_id,
        uintptr_t cookie,
        const struct mod_dvfs_level_limits *limits);
};

/*!
 * \brief DVFS updates notification API.
 *
 * \details API used by the domain to notify the HAL when either the
 *      limits or level has been changed.
 */
struct mod_dvfs_perf_updated_api {
    /*!
     * \brief Inform the HAL that the domain limits have been updated.
     *
     * \param domain_id Domain identifier.
     * \param cookie Context-specific value.
     * \param range_min Min allowed performance level.
     * \param range_max Max allowed performance level.
     */
    void (*notify_limits_updated)(
        fwk_id_t domain_id,
        uintptr_t cookie,
        uint32_t range_min,
        uint32_t range_max);

    /*!
     * \brief Inform the HAL that the domain level has been updated.
     *
     * \param domain_id Domain identifier.
     * \param cookie Context-specific value.
     * \param level The new performance level of the domain.
     */
    void (*notify_level_updated)(
        fwk_id_t domain_id,
        uintptr_t cookie,
        uint32_t level);
};

/*!
 * \}
 */

/*!
 * \defgroup GroupDvfsEvents Events
 * \{
 */

/*!
 * \brief <tt>Get current OPP </tt> event response parameters.
 */
struct mod_dvfs_params_response {
    /*! Event response status */
    int status;

    /*! Event response frequency */
    uint32_t performance_level;
};

/*!
 * \}
 */
/*!
 * \defgroup GroupDvfsIds Identifiers
 * \{
 */

/*!
 * \brief API indices.
 */
enum mod_dvfs_api_idx {
    MOD_DVFS_API_IDX_DVFS, /*!< API index for mod_dvfs_api_id_dvfs() */
    MOD_DVFS_API_IDX_COUNT /*!< Number of defined APIs */
};

/*! Module API identifier */
static const fwk_id_t mod_dvfs_api_id_dvfs =
    FWK_ID_API_INIT(FWK_MODULE_IDX_DVFS, MOD_DVFS_API_IDX_DVFS);

/*!
 * \brief Event indices.
 */
enum mod_dvfs_event_idx {
    MOD_DVFS_EVENT_IDX_SET, /*!< Set level/limits */
    MOD_DVFS_EVENT_IDX_GET_OPP, /*!< Get Operating Performance Point */
    MOD_DVFS_EVENT_IDX_COUNT, /*!< event count */
};

/*! <tt>Set operating point/limits</tt> event identifier */
static const fwk_id_t mod_dvfs_event_id_set =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_DVFS, MOD_DVFS_EVENT_IDX_SET);

/*! <tt>Get current OPP </tt> event identifier */
static const fwk_id_t mod_dvfs_event_id_get_opp =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_DVFS, MOD_DVFS_EVENT_IDX_GET_OPP);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_DVFS_H */

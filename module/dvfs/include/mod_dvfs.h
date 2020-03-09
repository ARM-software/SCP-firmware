/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_DVFS_H
#define MOD_DVFS_H

#include <fwk_id.h>
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
 * \brief Frequency limits.
 */
struct mod_dvfs_frequency_limits {
    uint64_t minimum; /*!< Minimum permitted rate */
    uint64_t maximum; /*!< Maximum permitted rate */
};

/*!
 * \brief Operating Performance Point (OPP).
 */
struct mod_dvfs_opp {
    uint64_t voltage; /*!< Power supply voltage in millivolts (mV) */
    uint64_t frequency; /*!< Clock rate in Hertz (Hz) */
    uint64_t power; /*!< Power draw in milliwatts (mW) */
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

    /*! Delay in milliseconds before retrying a request */
    uint16_t retry_ms;

    /*! Worst-case transition latency in microseconds */
    uint16_t latency;

    /*! Sustained operating point index */
    size_t sustained_idx;

    /*!
     * \brief Operating points.
     *
     * \note The frequencies of these operating points must be in ascending
     *      order.
     */
    struct mod_dvfs_opp *opps;
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
     * \brief Get the worst-case transition latency of a domain.
     *
     * \param domain_id Element identifier of the domain.
     * \param [out] latency Worst-case transition latency.
     */
    int (*get_latency)(fwk_id_t domain_id, uint16_t *latency);

    /*!
     * \brief Set the frequency of a domain.
     *
     * \param domain_id Element identifier of the domain.
     * \param idx Index of the operating point to transition to.
     */
    int (*set_frequency)(fwk_id_t domain_id, uint64_t frequency);

    /*!
     * \brief Get the frequency of a domain.
     *
     * \param domain_id Element identifier of the domain.
     * \param [out] limits Current frequency limits.
     */
    int (*get_frequency_limits)(
        fwk_id_t domain_id,
        struct mod_dvfs_frequency_limits *limits);

    /*!
     * \brief Set the frequency of a domain.
     *
     * \param domain_id Element identifier of the domain.
     * \param limits Pointer to the new limits.
     */
    int (*set_frequency_limits)(
        fwk_id_t domain_id,
        const struct mod_dvfs_frequency_limits *limits);
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
    uint64_t performance_level;
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
    MOD_DVFS_EVENT_IDX_SET,     /*!< Set level/limits */
    MOD_DVFS_EVENT_IDX_GET_OPP, /*!< Get frequency */
    MOD_DVFS_EVENT_IDX_COUNT,   /*!< event count */
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

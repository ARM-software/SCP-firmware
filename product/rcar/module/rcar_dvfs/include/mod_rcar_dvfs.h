/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_RCAR_DVFS_H
#define MOD_RCAR_DVFS_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \ingroup GroupRCARModule RCAR Product Modules
 * \defgroup GroupRCARDvfs Dynamic Voltage and Frequency Scaling (DVFS)
 * \{
 */

/*!
 * \defgroup GroupRCARDvfsTypes Types
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
};

/*!
 * \}
 */

/*!
 * \defgroup GroupRCARDvfsConfig Configuration
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
 * \defgroup GroupRCARDvfsApis APIs
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
    int (*get_sustained_opp)(fwk_id_t domain_id, struct mod_dvfs_opp *opp);

    /*!
     * \brief Get an operating point from its index.
     *
     * \param domain_id Element identifier of the domain.
     * \param n Index of the operating point to retrieve.
     * \param [out] opp Requested operating point.
     */
    int (*get_nth_opp)(fwk_id_t domain_id, size_t n, struct mod_dvfs_opp *opp);

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
     * \brief Set the frequency of a domain.
     *
     * \note This function is asynchronous.
     *
     * \param domain_id Element identifier of the domain.
     * \param idx Index of the operating point to transition to.
     */
    int (*set_frequency_async)(fwk_id_t domain_id, uint64_t frequency);

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

    /*!
     * \brief Set the frequency of a domain.
     *
     * \note This function is asynchronous.
     *
     * \param domain_id Element identifier of the domain.
     * \param limits Pointer to the new limits.
     */
    int (*set_frequency_limits_async)(
        fwk_id_t domain_id,
        const struct mod_dvfs_frequency_limits *limits);
};

/*!
 * \}
 */

/*!
 * \defgroup GroupRCARDvfsEvents Events
 * \{
 */

/*!
 * \brief <tt>Set operating point</tt> event response parameters.
 */
struct mod_dvfs_event_params_set_frequency_response {
    int status; /*!< Status of the request */
};

/*!
 * \brief <tt>Set limits</tt> event response parameters.
 */
struct mod_dvfs_event_params_set_frequency_limits_response {
    int status; /*!< Status of the request */
};

/*!
 * \}
 */

/*!
 * \defgroup GroupRCARDvfsIds Identifiers
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
    /*! Event index for mod_dvfs_event_id_set_frequency() */
    MOD_DVFS_EVENT_IDX_SET_FREQUENCY,

    /*! Event index for mod_dvfs_event_id_set_frequency_limits() */
    MOD_DVFS_EVENT_IDX_SET_FREQUENCY_LIMITS,

    /*! Number of defined events */
    MOD_DVFS_EVENT_IDX_COUNT
};

/*! <tt>Set operating point</tt> event identifier */
static const fwk_id_t mod_dvfs_event_id_set_frequency =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_DVFS, MOD_DVFS_EVENT_IDX_SET_FREQUENCY);

/*! <tt>Set frequency limits</tt> event identifier */
static const fwk_id_t mod_dvfs_event_id_set_frequency_limits =
    FWK_ID_EVENT_INIT(
        FWK_MODULE_IDX_DVFS,
        MOD_DVFS_EVENT_IDX_SET_FREQUENCY_LIMITS);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_RCAR_DVFS_H */

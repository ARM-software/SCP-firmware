/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Power capping and monitoring protocol completer.
 */

#ifndef MOD_SCMI_POWER_CAPPING_H
#define MOD_SCMI_POWER_CAPPING_H

#include <fwk_id.h>

#include <stdbool.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupSCMI_POWER_CAPPING SCMI power capping protocol
 * \{
 */

/*!
 * \brief Fast channels address index.
 */
enum mod_scmi_power_capping_fast_channels_cmd_handler_index {
    MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_GET,
    MOD_SCMI_PCAPPING_FAST_CHANNEL_CAP_SET,
    MOD_SCMI_PCAPPING_FAST_CHANNEL_PAI_GET,
    MOD_SCMI_PCAPPING_FAST_CHANNEL_PAI_SET,
    MOD_SCMI_PCAPPING_FAST_CHANNEL_COUNT
};

/*!
 * \brief SCMI Power capping domain unit.
 */
enum mod_scmi_power_capping_power_cap_unit {
    MOD_SCMI_PCAPPING_POWER_CAP_ABSTRACT = 0x0,
    MOD_SCMI_PCAPPING_POWER_CAP_MICROWATT = 0x1,
    MOD_SCMI_PCAPPING_POWER_CAP_MILLIWATT = 0x2,
};

/*!
 * \brief Power Capping fast channels domain configuration.
 */
struct scmi_pcapping_fch_config {
    /*!
     * \brief Corresponding transport element ID.
     */
    fwk_id_t transport_id;
    /*!
     * \brief Corresponding transport API ID.
     */
    fwk_id_t transport_api_id;
    /*!
     * \brief Fast channel support for the required domain.
     */
    bool fch_support;
};

/*!
 * \brief SCMI Power capping domain configuration.
 */
struct mod_scmi_power_capping_domain_config {
#ifdef BUILD_HAS_SCMI_POWER_CAPPING_STD_COMMANDS
    /*!
     * \brief Minimum PAI.
     *
     * \details The minimum supported power averaging interval (PAI),
     *     expressed in microseconds, its value cannot be zero.
     */
    uint32_t min_pai;

    /*!
     * \brief Maximum PAI.
     *
     * \details The maximum supported power averaging interval (PAI),
     *     expressed in microseconds, its value cannot be zero.
     */
    uint32_t max_pai;

    /*!
     * \brief PAI step.
     *
     * \details The step size between two consecutive PAI supported by
     *     this power capping domain. This value cannot be zero if min_pai and
     *     max_pai have different values.
     */
    uint32_t pai_step;
    /*!
     * \brief Minimum power cap.
     *
     * \details The minimum power value that can be set as the power cap for
     *     this domain, its value cannot be zero.
     */
    uint32_t min_power_cap;

    /*!
     * \brief Maximum power cap.
     *
     * \details The maximum power value that can be set as the power cap for
     *     this domain, its value cannot be zero.
     */
    uint32_t max_power_cap;

    /*!
     * \brief Power cap step.
     *
     * \details The step size between two consecutive power
     *     cap values supported by this power capping domain.
     */
    uint32_t power_cap_step;

    /*!
     * \brief Sustainable Power.
     *
     * \details The maximum sustainable power consumption
     *     for this domain under normal conditions considering
     *     all known external factors like thermal constraints.
     */
    uint32_t max_sustainable_power;

    /*!
     * \brief Parent id.
     *
     * \details This field identifies the parent power capping domain. This
     *     field is useful in constructing a power capping domain
     *     topology when the power capping domain (identified by domain_id)
     *     has a parent power capping domain (identified by parent_id).
     *
     *     A value of 0xFFFFFFFF implies that this power capping domain is
     *     contained within no parent domains.
     */
    uint32_t parent_idx;

    /*!
     * \brief Power unit used for capping and monitoring.
     */
    enum mod_scmi_power_capping_power_cap_unit power_cap_unit;
#endif
#ifdef BUILD_HAS_SCMI_POWER_CAPPING_FAST_CHANNELS_COMMANDS
    /*!
     * \brief Fast channels configuration.
     *
     * \details Assign the table of configurations for the fast channels using
     *      this domain.
     */
    const struct scmi_pcapping_fch_config *fch_config;
#endif
    /*!
     * \brief ID of the corresponding power allocator domain.
     */
    fwk_id_t power_allocator_domain_id;

    /*!
     * \brief ID of the corresponding power coordinator domain.
     */
    fwk_id_t power_coordinator_domain_id;

    /*!
     * \brief ID of the corresponding power meter domain.
     */
    fwk_id_t power_meter_domain_id;
};

/*!
 * \brief SCMI power capping APIs.
 *
 * \details APIs exported by SCMI power capping protocol.
 */
enum mod_scmi_power_capping_api {
#ifdef BUILD_HAS_SCMI_POWER_CAPPING_STD_COMMANDS
    /*! Index for the SCMI power capping request API */
    MOD_SCMI_POWER_CAPPING_API_IDX_REQUEST,
#endif
    /*! Number of APIs */
    MOD_SCMI_POWER_CAPPING_API_IDX_COUNT,
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SCMI_POWER_CAPPING_H */

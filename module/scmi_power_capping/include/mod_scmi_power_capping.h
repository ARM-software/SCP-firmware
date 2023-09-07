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
 * \defgroup GroupSCMI_POWER_CAPPING SCMI Power capping Protocol
 * \{
 */

/*!
 * \brief SCMI Power capping domain unit.
 */
enum mod_scmi_power_capping_power_cap_unit {
    MOD_SCMI_PCAPPING_POWER_CAP_ABSTRACT = 0x0,
    MOD_SCMI_PCAPPING_POWER_CAP_MICROWATT = 0x1,
    MOD_SCMI_PCAPPING_POWER_CAP_MILLIWATT = 0x2,
};

/*!
 * \brief SCMI Power capping domain configuration.
 */
struct mod_scmi_power_capping_domain_config {
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
    /*! Index for the SCMI power capping request API */
    MOD_SCMI_POWER_CAPPING_API_IDX_REQUEST,
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

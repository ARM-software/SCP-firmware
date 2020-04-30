/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI performance domain management protocol support.
 */

#ifndef MOD_SCMI_PERF_H
#define MOD_SCMI_PERF_H

#include <fwk_id.h>
#include <fwk_macros.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupSCMI_PERF SCMI Performance Domain Management Protocol
 * @{
 */

/*!
 * \brief Agent permissions.
 */
enum mod_scmi_perf_permissions {
    /*! No permissions */
    MOD_SCMI_PERF_PERMS_NONE = 0,

    /*! Permission to set performance level */
    MOD_SCMI_PERF_PERMS_SET_LEVEL  = (1 << 0),

    /*! Permission to set performance limits */
    MOD_SCMI_PERF_PERMS_SET_LIMITS = (1 << 1),
};

/*!
 * \brief Performance domain configuration data.
 */
struct mod_scmi_perf_domain_config {
    const uint32_t (*permissions)[]; /*!< Per-agent permission flags */

    /*!
     * \brief Domain fast channels.
     *
     * \details Platform Domain fast channel address
     *
     * \note May be set to 0x0, in which case support for fast
     *       channels is disabled for the platform.
     */
    uint64_t fast_channels_addr_scp;

   /*!
     * \brief Agent Domain fast channel address
     *
     * \details Address of shared memory for the agent
     */
    uint64_t fast_channels_addr_ap;

    /*!
     * \brief Rate limit in microsecs
     */
    uint32_t fast_channels_rate_limit;
};

/*!
 *\brief Domain Fast Channel
 *
 *\note Layout of the Per-Domain Fast Channel in shared memory.
 */
struct mod_scmi_perf_fast_channel {
    FWK_R uint32_t set_level; /*!< Performance level to be set */
    FWK_R uint32_t set_limit_range_max; /*!< max limit to be set */
    FWK_R uint32_t set_limit_range_min; /*!< min limit to be set */
    FWK_W uint32_t get_level; /*!< Current performance level */
    FWK_W uint32_t get_limit_range_max; /*!< Current limits, max */
    FWK_W uint32_t get_limit_range_min; /*!< Current limits, min */
};

/*!
 * \brief SCMI Performance Domain Management Protocol configuration data.
 */
struct mod_scmi_perf_config {
    /*! Per-domain configuration data */
    const struct mod_scmi_perf_domain_config (*domains)[];

    /*!
     * \brief Fast Channels Alarm ID
     *
     * \details The Fast Channel alarm triggers the callback which
     *    polls the fast channels and initiates the set_level and
     *    set_limits operations.
     */
    fwk_id_t fast_channels_alarm_id;

    /*! Fast Channel polling rate */
    uint32_t fast_channels_rate_limit;
};

/*!
 * \brief SCMI Performance APIs.
 *
 * \details APIs exported by SCMI Performance Protocol.
 */
enum scmi_perf_api_idx {
    /*! Index for the SCMI protocol API */
    MOD_SCMI_PERF_PROTOCOL_API,

    /*! Index of the notification API */
    MOD_SCMI_PERF_DVFS_NOTIFICATION_API,

    /*! Number of APIs */
    MOD_SCMI_PERF_API_COUNT
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* SCP_SCMI_PERF_H */

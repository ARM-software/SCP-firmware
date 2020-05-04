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

    /*! Flag indicating that statistics are collected for this domain */
    bool stats_collected;
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

    /*! Flag indicating statistics in use */
    bool stats_enabled;
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
 * \defgroup GroupScmiPerformancePolicyHandlers Policy Handlers
 *
 * \brief SCMI Performance Policy Handlers.
 *
 * \details The SCMI policy handlers are weak definitions to allow a platform
 *      to implement a policy appropriate to that platform. The SCMI
 *      performance policy functions may be overridden in the
 * `product/<platform>/src` directory.
 *
 * \note The `frequency`/`range` values may be changed by the policy handlers.
 * \note See `product/juno/src/juno_scmi_clock.c` for an example policy
 *      handler.
 *
 * \{
 */

/*!
 * \brief Policy handler policies.
 *
 * \details These values are returned to the message handler by the policy
 *      handlers to determine whether the message handler should continue
 *      processing the message, or whether the request has been rejected.
 */
enum mod_scmi_perf_policy_status {
    /*! Do not execute the message handler */
    MOD_SCMI_PERF_SKIP_MESSAGE_HANDLER,

    /*! Execute the message handler */
    MOD_SCMI_PERF_EXECUTE_MESSAGE_HANDLER,
};

/*!
 *
 * \brief SCMI Performance Set Level command policy.
 *
 * \details This function determines whether the SCMI message handler should
 *      allow or reject a given SCMI Performance Set Level command.
 *
 *      The SCMI policy handler is executed before the message handler is
 *      called. The SCMI protocol message handler will only continue if the
 *      policy handler both returns ::FWK_SUCCESS and sets the policy status to
 *      ::MOD_SCMI_PERF_EXECUTE_MESSAGE_HANDLER.
 *
 * \param[out] policy_status Whether the command should be accepted or not.
 * \param[in, out] frequency Frequency requested to be set.
 * \param[in] agent_id Identifier of the agent requesting the service.
 * \param[in] domain_id Identifier of the performance domain.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int scmi_perf_level_set_policy(
    enum mod_scmi_perf_policy_status *policy_status,
    uint64_t *frequency,
    unsigned int agent_id,
    fwk_id_t domain_id);

/*!
 * \brief SCMI Performance Set Limits command policy.
 *
 * \details This function determines whether the SCMI message handler should
 *      allow or reject a given SCMI Performance Set Limits command.
 *
 *      The SCMI policy handler is executed before the message handler is
 *      called. The SCMI protocol message handler will only continue if the
 *      policy handler both returns ::FWK_SUCCESS and sets the policy status to
 *      ::MOD_SCMI_PERF_EXECUTE_MESSAGE_HANDLER.
 *
 * \param[out] policy_status Whether the command should be accepted or not.
 * \param[in, out] range_min Minimum frequency range.
 * \param[in, out] range_max Maximum frequency range.
 * \param[in] agent_id Identifier of the agent requesting the service.
 * \param[in] domain_id Identifier of the performance domain.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int scmi_perf_limits_set_policy(
    enum mod_scmi_perf_policy_status *policy_status,
    uint64_t *range_min,
    uint64_t *range_max,
    unsigned int agent_id,
    fwk_id_t domain_id);

/*!
 * @}
 */

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* SCP_SCMI_PERF_H */

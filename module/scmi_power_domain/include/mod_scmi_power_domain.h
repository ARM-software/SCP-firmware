/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef MOD_SCMI_POWER_DOMAIN_H
#define MOD_SCMI_POWER_DOMAIN_H

#include <fwk_id.h>

/*!
 * \ingroup GroupModules Modules
 * \defgroup GroupSCMIPowerDomain SCMI Power Domain
 * \{
 */

#ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
struct mod_scmi_pd_agent_config {
    /*!
     * \brief Agent view of Power domains.
     *      Identifiers of all the power domains an agent can view in per agent
     *      organisation. This must be a subset of all Power domain module
     *      elements to restrict the agent view to only operable domains.
     * FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, POWER_DOMAIN_IDX)
     */
    uint32_t *domains;

    /*!
     * \brief Number of domains operable by the agent.
     */
    unsigned int domain_count;
};
#endif

/*!
 * \brief SCMI Power domain module configuration.
 */
struct mod_scmi_pd_config {
#ifdef BUILD_HAS_MOD_DEBUG
    /*!
     * \brief Identifier of the debug power domain.
     *
     * \details This identifier is required only for platforms using the debug
     *      module.
     *
     * \note Only one element is supported at the moment.
     *
     */
    fwk_id_t debug_pd_id;

    /*!
     * \brief Identifier of one the debug module devices.
     *
     * \details This identifier is required only for platforms using the debug
     *      module.
     */
    fwk_id_t debug_id;
#endif

#ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
    /*!
     * \brief Agent config table
     */
    struct mod_scmi_pd_agent_config *agent_config_table;
#endif
};

/*!
 * \defgroup GroupScmiPowerPolicyHandlers Policy Handlers
 *
 * \brief SCMI Power Policy Handlers.
 *
 * \details The SCMI policy handlers are weak definitions to allow a platform
 *      to implement a policy appropriate to that platform. The SCMI
 *      power domain policy functions may be overridden in the
 *      `product/<platform>/src` directory.
 *
 * \note The `state` value may be changed by the policy handler.
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
enum mod_scmi_pd_policy_status {
    /*! Do not execute the message handler */
    MOD_SCMI_PD_SKIP_MESSAGE_HANDLER,

    /*! Execute the message handler */
    MOD_SCMI_PD_EXECUTE_MESSAGE_HANDLER,
};

/*!
 * \brief SCMI Power Domain State Set command policy.
 *
 * \details This function determines whether the SCMI message handler should
 *      allow or reject a given SCMI Power Domain State Set command.
 *
 *      The SCMI policy handler is executed before the message handler is
 *      called. The SCMI protocol message handler will only continue if the
 *      policy handler both returns ::FWK_SUCCESS and sets the policy status to
 *      ::MOD_SCMI_CLOCK_EXECUTE_MESSAGE_HANDLER.
 *
 *      The SCMI policy handlers have default weak implementations that allow a
 *      platform to implement a policy appropriate for that platform.
 *
 * \note The state data may be changed by the policy handler.
 *
 * \note See `product/juno/src/juno_scmi_clock.c` for an example policy
 *      handler.
 *
 * \param[out] policy_status Whether the command should be accepted or not.
 * \param[in, out] state Pointer to the requested state.
 * \param[in] agent_id Identifier of the agent requesting the service.
 * \param[in] pd_id Element identifier of the power domain.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int scmi_pd_power_state_set_policy(
    enum mod_scmi_pd_policy_status *policy_status,
    uint32_t *state,
    unsigned int agent_id,
    fwk_id_t pd_id);

/*!
 * \}
 */

#endif /* MOD_SCMI_POWER_DOMAIN_H */

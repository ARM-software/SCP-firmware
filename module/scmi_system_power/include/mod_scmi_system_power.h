/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SCMI_SYSTEM_POWER_H
#define MOD_SCMI_SYSTEM_POWER_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupSCMI_SYS SCMI System Power Management Protocol
 * \{
 */

/*!
 * \brief SCMI system views.
 */
enum mod_scmi_system_view {
    /*! OSPM view of the system */
    MOD_SCMI_SYSTEM_VIEW_OSPM,

    /*! Full view of the system */
    MOD_SCMI_SYSTEM_VIEW_FULL,

    MOD_SCMI_SYSTEM_VIEW_COUNT,
};

/*!
 * \brief SCMI System Power Management Protocol configuration data.
 */
struct mod_scmi_system_power_config {
    /*! System view */
    enum mod_scmi_system_view system_view;

    /*!
     * \brief Identifier of the power domain to target for a system wake-up.
     *
     * \note This is only used with the OSPM view.
     */
    fwk_id_t wakeup_power_domain_id;

    /*!
     * \brief Composite state for a system wake-up.
     *
     * \note This is only used with the OSPM view.
     */
    uint32_t wakeup_composite_state;

    /*!
     * \brief System suspend state.
     */
    unsigned int system_suspend_state;

    /*!
     * \brief Identifier of the alarm for graceful request timeout.
     *
     * \note This alarm is optional, if it is not used it must be set to
     * FWK_ID_NONE
     */
    fwk_id_t alarm_id;

    /*!
     * Timeout period to wait for graceful response (milliseconds)
     */
    uint32_t graceful_timeout;
};

/*!
 * \defgroup GroupScmiSystemPowerPolicyHandlers Policy Handlers
 *
 * \brief SCMI System Power Policy Handlers.
 *
 * \details The SCMI policy handlers are weak definitions to allow a platform
 *      to implement a policy appropriate to that platform. The SCMI
 *      system power policy functions may be overridden in the
 *      `product/<platform>/src` directory.
 *
 * \note The `state` data may be modified by the policy handler.
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
enum mod_scmi_sys_power_policy_status {
    /*! Do not execute the message handler */
    MOD_SCMI_SYS_POWER_SKIP_MESSAGE_HANDLER,

    /*! Execute the message handler */
    MOD_SCMI_SYS_POWER_EXECUTE_MESSAGE_HANDLER,
};

/*!
 * \brief SCMI System Power State Set command policy.
 *
 * \details This function determines whether the SCMI message handler should
 *      allow or reject a given SCMI  System Power State Set command.
 *
 *      The SCMI policy handler is executed before the message handler is
 *      called. The SCMI protocol message handler will only continue if the
 *      policy handler both returns ::FWK_SUCCESS and sets the policy status to
 *      ::MOD_SCMI_CLOCK_EXECUTE_MESSAGE_HANDLER.
 *
 *      The SCMI policy handlers have default weak implementations that allow a
 *      platform to implement a policy appropriate for that platform.
 *
 * \note See `product/juno/src/juno_scmi_clock.c` for an example policy
 *      handler.
 *
 * \param[out] policy_status Whether the command should be accepted or not.
 * \param[in, out] state Pointer to the requested state.
 * \param[in] service_id Identifier of the service requesting the change.
 * \param[in] graceful The set state request is graceful or not.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int scmi_sys_power_state_set_policy(
    enum mod_scmi_sys_power_policy_status *policy_status,
    uint32_t *state,
    fwk_id_t service_id,
    bool graceful);

/*!
 * \}
 */

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SCMI_SYSTEM_POWER_H */

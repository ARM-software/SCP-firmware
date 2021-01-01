/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Clock Management Protocol Support.
 */

#ifndef MOD_SCMI_CLOCK_H
#define MOD_SCMI_CLOCK_H

#include <fwk_id.h>

#include <mod_clock.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules Modules
 * \defgroup GroupSCMI_CLOCK SCMI Clock Management Protocol
 * \{
 */

/*!
 * \brief Permission flags governing the ability to use certain SCMI commands to
 *      interact with a clock.
 *
 * \details Setting a permission flag for a clock enables the corresponding
 *      functionality for any agent that has visibilty of the clock
 *      through its clock device table.
 */
enum mod_scmi_clock_permissions {
    /*! No permissions (at least one must be granted) */
    MOD_SCMI_CLOCK_PERM_INVALID = 0,

    /*! The clock's attributes can be queried */
    MOD_SCMI_CLOCK_PERM_ATTRIBUTES = (1 << 0),

    /*! The clock's supported rates can be queried */
    MOD_SCMI_CLOCK_PERM_DESCRIBE_RATES = (1 << 1),

    /*! The clock's current rate can be queried */
    MOD_SCMI_CLOCK_PERM_GET_RATE = (1 << 2),

    /*! The clock can be set to a new rate */
    MOD_SCMI_CLOCK_PERM_SET_RATE = (1 << 3),

    /*! The clock can be enabled and disabled */
    MOD_SCMI_CLOCK_PERM_SET_CONFIG = (1 << 4),
};

/*!
 * \brief Clock device.
 *
 * \details Clock device structures are used in per-agent clock device tables.
 *      Each contains an identifier of an element that will be bound to in order
 *      to use the clock device. The permission flags for the clock are applied
 *      to any agent that uses the device configuration in its clock device
 *      table.
 */
struct mod_scmi_clock_device {
    /*!
     * \brief Clock element identifier.
     *
     * \details The module that owns the element must implement the Clock API
     *      that is defined by the \c clock module.
     */
    fwk_id_t element_id;

    /*! \brief Startup clock status flag.
     *
     *  \details State of the clock at startup. If set, it specifies that
     *       this clock starts running. This flag does not affect the actual
     *       state of the clock, but it provides an indication for the initial
     *       internal state map.
     */
    bool starts_enabled;
};

/*!
 * \brief Agent descriptor.
 *
 * \details Describes an agent that uses the SCMI Clock Management protocol.
 *      Provides a pointer to the agent's clock device table and the number of
 *      devices within the table.
 */
struct mod_scmi_clock_agent {
    /*! Pointer to a table of clock devices that are visible to the agent */
    const struct mod_scmi_clock_device *device_table;

    /*!
     * \brief The number of \c mod_scmi_clock_device structures in the table
     *      pointed to by \c device_table.
     */
    uint8_t device_count;
};

/*!
 * \brief Module configuration.
 */
struct mod_scmi_clock_config {
    /*! Maximum supported number of pending, asynchronous clock rate changes */
    uint8_t max_pending_transactions;

    /*!
     * \brief Pointer to the table of agent descriptors, used to provide
     *      per-agent views of clocks in the system.
     */
    const struct mod_scmi_clock_agent *agent_table;

    /*! Number of agents in ::mod_scmi_clock_config::agent_table */
    size_t agent_count;
};

/*!
 * \defgroup GroupScmiClockPolicyHandlers Policy Handlers
 *
 * \brief SCMI Clock Policy Handlers.
 *
 * \details The SCMI policy handlers are weak definitions to allow a platform
 *      to implement a policy appropriate to that platform. The SCMI
 *      clock policy functions may be overridden in the
 *      `product/<platform>/src` directory.
 *
 * \note The `rate`/`round_mode`/`state` values may be changed by the policy
 *      handlers.
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
enum mod_scmi_clock_policy_status {
    /*! Do not execute the message handler */
    MOD_SCMI_CLOCK_SKIP_MESSAGE_HANDLER,

    /*! Execute the message handler */
    MOD_SCMI_CLOCK_EXECUTE_MESSAGE_HANDLER,
};

/*!
 * \brief Directive for Policy handler.
 *
 * \details These values are passed to the policy handlers to indicate
 *      whether the policy handler is being invoked prior to the operation
 *      processing the message or on completion of the message. If the message
 *      handler has completed it is safe to commit any changes the policy
 *      handler may need to make to any internal state.
 */
enum mod_scmi_clock_policy_commit {
    /*! The message handler has not run yet */
    MOD_SCMI_CLOCK_PRE_MESSAGE_HANDLER,

    /*! The message handler has completed */
    MOD_SCMI_CLOCK_POST_MESSAGE_HANDLER,
};

/*!
 * \brief SCMI Clock Set Rate command policy.
 *
 * \details This function determines whether the SCMI message handler should
 *      allow or reject a given SCMI Clock Set Rate command.
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
 * \param[in, out] round_mode Rounding operation to perform, if required, to
 *      achieve the given rate.
 * \param[in, out] rate Desired frequency in hertz.
 * \param[in] policy_commit Whether the message handler has completed or not.
 * \param[in] service_id Identifier of the agent making the request.
 * \param[in] clock_dev_id SCMI clock device identifier.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int mod_scmi_clock_rate_set_policy(
    enum mod_scmi_clock_policy_status *policy_status,
    enum mod_clock_round_mode *round_mode,
    uint64_t *rate,
    enum mod_scmi_clock_policy_commit policy_commit,
    fwk_id_t service_id,
    uint32_t clock_dev_id);

/*!
 * \brief SCMI Clock Set Config command policy.
 *
 * \details This function determines whether the SCMI message handler should
 *      allow or reject a given SCMI Clock Set Config command.
 *
 *      The SCMI policy handler is executed before the message handler is
 *      called. The SCMI protocol message handler will only continue if the
 *      policy handler both returns ::FWK_SUCCESS and sets the policy status to
 *      ::MOD_SCMI_CLOCK_EXECUTE_MESSAGE_HANDLER.
 *
 * \param[out] policy_status Whether the command should be accepted or not.
 * \param[in, out] state Pointer to one of the valid clock states.
 * \param[in] policy_commit Whether the message handler has completed or not.
 * \param[in] service_id Identifier of the agent making the request.
 * \param[in] clock_dev_id Identifier of the clock.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 *
 * \return Status code representing the result of the operation.
 */
int mod_scmi_clock_config_set_policy(
    enum mod_scmi_clock_policy_status *policy_status,
    enum mod_clock_state *state,
    enum mod_scmi_clock_policy_commit policy_commit,
    fwk_id_t service_id,
    uint32_t clock_dev_id);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SCMI_CLOCK_H */

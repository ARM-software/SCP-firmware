/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Clock Management Protocol Support.
 */

#ifndef MOD_SCMI_CLOCK_H
#define MOD_SCMI_CLOCK_H

#include <fwk_id.h>

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

    /*! Mask of permission flags defined by \ref mod_scmi_clock_permissions */
    uint8_t permissions;
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

    /*! Number of agents in \ref agent_table */
    size_t agent_count;
};

/*!
 * \}
 */

#endif /* MOD_SCMI_CLOCK_H */

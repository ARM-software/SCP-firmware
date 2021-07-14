/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Voltage Domain Management Protocol Support.
 */

#ifndef MOD_SCMI_VOLTAGE_DOMAIN_H
#define MOD_SCMI_VOLTAGE_DOMAIN_H

#include <fwk_id.h>

#include <mod_voltage_domain.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules Modules
 * \defgroup GroupSCMI_VOLTD SCMI Voltage Domain Management Protocol
 * \{
 */

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
/*!
 * \brief Permission flags governing the ability to use certain SCMI commands to
 *      interact with a voltage domain.
 *
 * \details Setting a permission flag for a voltage domain enables the
 *      corresponding functionality for any agent that has visibilty of
 *      the voltage domain through its voltage domain device table.
 */
enum mod_scmi_voltd_permissions {
    /*! No permissions (at least one must be granted) */
    MOD_SCMI_VOLTD_PERM_INVALID = 0,

    /*! The voltage domain information can be queried */
    MOD_SCMI_VOLTD_PERM_GET_INFO = (1 << 1),

    /*! The voltage domain config can be modified */
    MOD_SCMI_VOLTD_PERM_SET_CONFIG = (1 << 2),

    /*! The voltage level can be set to a new value */
    MOD_SCMI_VOLTD_PERM_SET_LEVEL = (1 << 3),
};
#endif

/*!
 * \brief Voltage Domain device.
 *
 * \details Voltage Domain device structures are used in per-agent voltage
 *      domain device tables. Each contains an identifier of an element that
 *      will be bound to in order to use the voltage domain device. The
 *      permission flags for the voltage domain are applied to any agent
 *      that uses the device configuration in its voltage domain device
 *      table.
 */
struct mod_scmi_voltd_device {
    /*!
     * \brief Voltage Domain element identifier.
     *
     * \details The module that owns the element must implement the Voltage
     *      Domain API that is defined by the \c voltage domain module.
     */
    fwk_id_t element_id;

    /*!
     * \brief Exposed configuration of the domain.
     *
     * \details This reflects the configuration state of the voltage domain.
     */
    uint32_t config;
};

/*!
 * \brief Agent descriptor.
 *
 * \details Describes an agent that uses the SCMI Voltage Domain Management
 *      protocol. Provides a pointer to the agent's voltage domain device
 *      table and the number of devices within the table.
 */
struct mod_scmi_voltd_agent {
    /*! Pointer to a table of voltage domain devices visible to the agent */
    struct mod_scmi_voltd_device *device_table;

    /*!
     * \brief The number of \c mod_scmi_voltd_device structures in the table
     *      pointed to by \c device_table.
     */
    uint32_t domain_count;
};

/*!
 * \brief Module configuration.
 */
struct mod_scmi_voltd_config {
    /*!
     * \brief Pointer to the table of agent descriptors, used to provide
     *      per-agent views of voltage domain in the system.
     */
    const struct mod_scmi_voltd_agent *agent_table;

    /*! Number of agents in \ref agent_table */
    size_t agent_count;
};

/*!
 * \}
 */

#endif /* MOD_SCMI_VOLTAGE_DOMAIN_H */

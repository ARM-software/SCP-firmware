/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Reset Domain Protocol Support.
 */

#ifndef MOD_SCMI_RESET_DOMAIN_H
#define MOD_SCMI_RESET_DOMAIN_H

#include <stdint.h>

/*!
 * \ingroup GroupModules Modules
 * \defgroup GroupSCMI_RESET SCMI Reset Domain Management Protocol
 * \{
 */

/*!
 * \brief Reset domain device.
 *
 * \details Reset domain device structures are used in per-agent reset device
 *      tables. Each contains an identifier of an element that will be bound
 *      to in order to use the reset device.
 */
struct mod_scmi_reset_domain_device {
    /*!
     * \brief Reset element identifier.
     *
     * \details The module that owns the element must implement the Reset API
     *      that is defined by the \c reset module.
     */
    fwk_id_t element_id;
};

/*!
 * \brief Agent descriptor.
 *
 * \details Describes an agent that uses the SCMI Reset Domain protocol.
 *      Provides a pointer to the agent's reset device table and the number of
 *      devices within the table.
 */
struct mod_scmi_reset_domain_agent {
    /*! Pointer to a table of clock devices that are visible to the agent */
    const struct mod_scmi_reset_domain_device *device_table;

    /*!
     * \brief The number of \c mod_scmi_reset_domain_device structures in the
     *      table pointed to by \c device_table.
     */
    uint8_t device_count;
};

/*!
 * \brief Module configuration.
 */
struct mod_scmi_reset_domain_config {
    /*!
     * \brief Pointer to the table of agent descriptors, used to provide
     *      per-agent views of reset in the system.
     */
    const struct mod_scmi_reset_domain_agent *agent_table;

    /*! Number of agents in \ref agent_table */
    size_t agent_count;
};

/*!
 * \}
 */

#endif /* MOD_SCMI_RESET_DOMAIN_H */

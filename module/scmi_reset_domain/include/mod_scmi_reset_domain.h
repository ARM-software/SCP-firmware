/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Reset Domain Protocol Support.
 */

#ifndef MOD_SCMI_RESET_DOMAIN_H
#define MOD_SCMI_RESET_DOMAIN_H

#include <fwk_id.h>
#include <stdint.h>
#include <stdlib.h>

/*!
 * \ingroup GroupModules Modules
 * \defgroup GroupSCMI_RESET SCMI Reset Domain Management Protocol
 * \{
 */

/*!
 * \brief Permission flags governing the ability to use certain SCMI commands
 *     to interact with a reset domain.
 *
 * \details Setting a permission flag for a reset domain enables the
 *      corresponding functionality for any agent that has visibilty of the
 *      reset domain through it's reset domain device table.
 */
enum mod_scmi_reset_domain_permissions {
    /*! No permissions (at least one must be granted) */
    MOD_SCMI_RESET_DOMAIN_PERM_INVALID = 0,

    /*! The reset domain's attributes can be queried */
    MOD_SCMI_RESET_DOMAIN_PERM_ATTRIBUTES = (1 << 0),

    /*! The permission to reset the domain */
    MOD_SCMI_RESET_DOMAIN_PERM_RESET = (1 << 1),
};

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

    /*! Mask of permission flags defined by reset domain configuration */
   uint8_t permissions;

};

/*!
 * \brief Agent descriptor.
 *
 * \details Describes an agent that uses the SCMI Reset Domain protocol.
 *      Provides a pointer to the agent's reset device table and the number of
 *      devices within the table.
 */
struct mod_scmi_reset_domain_agent {
    /*! Pointer to a table of reset devices that are visible to the agent */
    const struct mod_scmi_reset_domain_device *device_table;

    /*!
     * \brief The number of \c mod_scmi_reset_domain_device structures in the
     *      table pointed to by \c device_table.
     */
    uint8_t agent_domain_count;
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
    unsigned int agent_count;

};

/*!
 * \brief SCMI Reset Domain APIs.
 *
 * \details APIs exported by SCMI Reset Domain Protocol.
 */
enum scmi_reset_domain_api_idx {
    /*! Index for the SCMI protocol API */
    MOD_SCMI_RESET_DOMAIN_PROTOCOL_API,

    /*! Number of APIs */
    MOD_SCMI_RESET_DOMAIN_API_COUNT
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SCMI_RESET_DOMAIN_H */

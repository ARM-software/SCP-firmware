/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
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

/*!
 * \brief SCMI Power domain module configuration.
 */
struct mod_scmi_pd_config {
    /*!
     * \brief Identifier of one the debug module devices.
     *
     * \details This identifier is required only for platforms using the debug
     *      module.
     */
    fwk_id_t debug_id;
};

/*!
 * \}
 */

#endif /* MOD_SCMI_POWER_DOMAIN_H */

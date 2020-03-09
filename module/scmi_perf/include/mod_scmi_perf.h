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
};

/*!
 * \brief SCMI Performance Domain Management Protocol configuration data.
 */
struct mod_scmi_perf_config {
    /*! Per-domain configuration data */
    const struct mod_scmi_perf_domain_config (*domains)[];
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* SCP_SCMI_PERF_H */

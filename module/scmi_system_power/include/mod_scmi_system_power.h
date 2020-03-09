/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SCMI_SYSTEM_POWER_H
#define MOD_SCMI_SYSTEM_POWER_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupSCMI_SYS SCMI System Power Management Protocol
 * @{
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
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SCMI_SYSTEM_POWER_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCP Platform Support
 */

#ifndef MOD_SCP_PLATFORM_H
#define MOD_SCP_PLATFORM_H

#include <mod_power_domain.h>

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupPLATFORMModule PLATFORM Product Modules
 * @{
 */

/*!
 * \defgroup GroupSCPPlatform SCP Platform Support
 * @{
 */

/*!
 * \brief Indices of the interfaces exposed by the module.
 */
enum mod_scp_platform_api_idx {
    /*! API index for the driver interface of the SYSTEM POWER module */
    MOD_SCP_PLATFORM_API_IDX_SYSTEM_POWER_DRIVER,

    /*! Interface for Transport module */
    MOD_SCP_PLATFORM_API_IDX_TRANSPORT_SIGNAL,

    /*! Number of exposed interfaces */
    MOD_SCP_PLATFORM_API_COUNT
};

/*!
 * \brief Notification indices.
 */
enum mod_scp_platform_notification_idx {
    /*! SCP subsystem initialization completion notification */
    MOD_SCP_PLATFORM_NOTIFICATION_IDX_SUBSYS_INITIALIZED,

    /*! Number of notifications defined by the module */
    MOD_SCP_PLATFORM_NOTIFICATION_COUNT,
};

/*!
 * \brief Identifier for the
 * ::MOD_SCP_PLATFORM_NOTIFICATION_IDX_SUBSYS_INITIALIZED notification.
 */
static const fwk_id_t mod_scp_platform_notification_subsys_init =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_SCP_PLATFORM,
        MOD_SCP_PLATFORM_NOTIFICATION_IDX_SUBSYS_INITIALIZED);

/*!
 * \brief SCP platform configuration data.
 */
struct mod_scp_platform_config {
    /*! Timer identifier */
    fwk_id_t timer_id;

    /*! Transport channel identifier */
    fwk_id_t transport_id;

    /*!
     * Maximum amount of time, in microseconds, to wait for the RSS handshake
     * event.
     */
    uint32_t rss_sync_wait_us;
};

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SCP_PLATFORM_H */

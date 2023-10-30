
/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_APCONTEXT_H
#define MOD_APCONTEXT_H

#include <fwk_id.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \ingroup GroupModules Modules
 * \defgroup GroupModuleAPContext AP Context
 *
 * \brief Application Processor (AP) context module.
 *
 * \details This module implements the AP context zero-initialization.
 * \{
 */

/*!
 * \brief Platform notification source and notification id
 *
 * \details On platforms that require platform configuration (in addition to
 *      the clock configuration) to access the AP context memory region, the
 *      platform notification can be subscribed. This is optional for a platform
 *      and if provided as module configuration data, the AP context memory
 *      region will be accessed only after this notification is processed.
 */
struct mod_transport_platform_notification {
    /*! Identifier of the notification id */
    const fwk_id_t notification_id;

    /*! Identifier of the module sending the notification */
    const fwk_id_t source_id;
};

/*!
 * \brief AP context configuration data
 */
struct mod_apcontext_config {
    /*! Base address of the AP context */
    uintptr_t base;

    /*! Size of the AP context */
    size_t size;

    /*! Identifier of the clock this module depends on */
    fwk_id_t clock_id;

    /*!
     * Platform notification source and notification id (optional)
     */
    struct mod_transport_platform_notification platform_notification;
};

/*!
 * \}
 */

#endif /* MOD_APCONTEXT_H */

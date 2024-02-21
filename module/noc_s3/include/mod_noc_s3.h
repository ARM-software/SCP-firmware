/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef MOD_NOC_S3_H
#define MOD_NOC_S3_H

#include <fwk_id.h>

#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupNoCS3 Network on Chip S3
 * \{
 */

/*!
 * \brief Platform notification source and notification id.
 *
 * \details If the module is dependant on notification from other modules, then
 *          the module will subscribe to its notification and start only after
 *          receiving it.
 */
struct mod_noc_s3_platform_notification {
    /*! Identifier of the notification id */
    const fwk_id_t notification_id;
    /*! Identifier of the module sending the notification */
    const fwk_id_t source_id;
};

/*!
 * \brief Info to configure ports in the NoC S3 block.
 */
struct mod_noc_s3_element_config {
    /*! Platform notification source and notification id */
    struct mod_noc_s3_platform_notification plat_notification;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_NOC_S3_H */

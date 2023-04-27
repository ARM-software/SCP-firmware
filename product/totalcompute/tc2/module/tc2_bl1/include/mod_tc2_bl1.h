/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description: TC2 BL1 module.
 */

#ifndef MOD_TC2_BL1_H
#define MOD_TC2_BL1_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \ingroup GroupTC2BL1
 * \defgroup GroupTC2BL1 BL1 Support
 * \{
 */

/*!
 * \brief Module configuration data.
 */
struct mod_tc2_bl1_config {
    /*! Base address of the Application Processor (AP) context area */
    const uintptr_t ap_context_base;

    /*! Size of the AP context area */
    const size_t ap_context_size;

    /*! Element ID of the primary cluster PPU */
    const fwk_id_t id_primary_cluster;

    /*! Element ID of the primary core PPU */
    const fwk_id_t id_primary_core;
};

/*!
 * \brief Type of the interfaces exposed by the module.
 */
enum mod_tc2_bl1_api_idx {
    /*! Interface for transport module */
    MOD_TC2_BL1_API_TRANSPORT_FIRMWARE_SIGNAL_INPUT,
    /*! Number of defined interfaces */
    MOD_TC2_BL1_API_IDX_COUNT,
};

/*!
 * \brief Notification indices.
 */
enum mod_tc2_bl1_notification_idx {
    /*! <tt>SYSTOP powered on</tt> notification */
    MOD_TC2_BL1_NOTIFICATION_IDX_POWER_SYSTOP,

    /*! Number of notifications defined by the module */
    MOD_TC2_BL1_NOTIFICATION_COUNT,
};

/*! <tt>SYSTOP powered on</tt> notification identifier */
static const fwk_id_t mod_tc2_bl1_notification_id_systop =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_TC2_BL1,
        MOD_TC2_BL1_NOTIFICATION_IDX_POWER_SYSTOP);

/*!
 * \}
 */

#endif /* MOD_TC2_BL1_H */

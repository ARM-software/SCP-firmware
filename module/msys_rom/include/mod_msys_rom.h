/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description: Module dedicated to the ROM firmware of the mobile platforms to
 *     initiate the boot of the primary AP core and then to jump to the SCP RAM
 *     firmware.
 */

#ifndef MOD_MSYS_ROM_H
#define MOD_MSYS_ROM_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \ingroup GroupMSYSModule
 * \defgroup GroupMSYSROM ROM Support
 * @{
 */

/*!
 * \brief Module configuration data.
 */
struct msys_rom_config {
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
 * \brief Notification indices.
 */
enum mod_msys_rom_notification_idx {
    /*! <tt>SYSTOP powered on</tt> notification */
    MOD_MSYS_ROM_NOTIFICATION_IDX_POWER_SYSTOP,

    /*! Number of notifications defined by the module */
    MOD_MSYS_ROM_NOTIFICATION_COUNT,
};

/*! <tt>SYSTOP powered on</tt> notification identifier */
static const fwk_id_t mod_msys_rom_notification_id_systop =
    FWK_ID_NOTIFICATION_INIT(FWK_MODULE_IDX_MSYS_ROM,
                             MOD_MSYS_ROM_NOTIFICATION_IDX_POWER_SYSTOP);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_MSYS_ROM_H */

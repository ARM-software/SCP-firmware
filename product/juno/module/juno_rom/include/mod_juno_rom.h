/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_JUNO_ROM_H
#define MOD_JUNO_ROM_H

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupJunoModule Juno Product Modules
 * \{
 */

/*!
 * \defgroup GroupJunoROM Juno ROM
 * \{
 */

/*!
 * \brief Event indices.
 */
enum mod_juno_rom_event_idx {
    /*! 'Run' event */
    MOD_JUNO_ROM_EVENT_IDX_RUN,

    /*! Number of defined events */
    MOD_JUNO_ROM_EVENT_COUNT,
};

/*!
 * \brief 'Run' event identifier.
 */
static const fwk_id_t mod_juno_rom_event_id_run =
    FWK_ID_EVENT_INIT(FWK_MODULE_IDX_JUNO_ROM, MOD_JUNO_ROM_EVENT_IDX_RUN);

/*!
 * \brief Notification indices.
 */
enum mod_juno_rom_notification_idx {
    /*! 'SYSTOP ON' notification */
    MOD_JUNO_ROM_NOTIFICATION_IDX_SYSTOP,

    /*! Number of defined notifications */
    MOD_JUNO_ROM_NOTIFICATION_COUNT,
};

/*!
 * \brief 'SYSTOP ON' notification identifier.
 */
static const fwk_id_t mod_juno_rom_notification_id_systop =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_JUNO_ROM,
        MOD_JUNO_ROM_NOTIFICATION_IDX_SYSTOP);

/*!
 * \brief Module configuration.
 */
struct mod_juno_rom_config {
    /*! Base address of the AP context area */
    uintptr_t ap_context_base;

    /*! Size of the AP context area */
    size_t ap_context_size;

    /*! Base address of the RAM firmware image */
    uintptr_t ramfw_base;
};

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_JUNO_ROM_H */

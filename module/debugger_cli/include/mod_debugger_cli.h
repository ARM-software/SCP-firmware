/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Debugger CLI module.
 */

#ifndef MOD_DEBUGGER_CLI_H
#define MOD_DEBUGGER_CLI_H

#include <mod_timer.h>

#include <fwk_module.h>

/*!
 * \brief Debugger CLI module configuration.
 */
struct mod_debugger_cli_module_config {
    /*!
     * Element identifier of the alarm used for polling the UART
     */
    fwk_id_t alarm_id;

    /*!
     * Time period to set for the poll alarm delay (milliseconds)
     */
    uint32_t poll_period;
};

#endif /* MOD_DEBUGGER_CLI_H */

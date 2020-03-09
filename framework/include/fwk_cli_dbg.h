/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef FWK_CLI_DBG_H
#define FWK_CLI_DBG_H

/*!
 * \addtogroup GroupLibFramework Framework
 * @{
 */

/*!
 * \defgroup GroupCLI Debugger
 * @{
 */

#if defined(BUILD_HAS_DEBUGGER)
#    include <cli.h>

/*!
 * \brief Define the CLI Debugger function
 *
 * \details If the CLI Debugger is not included in the build this macro
 *          will be empty
 *
 */
#define CLI_DEBUGGER() cli_init()

#else

/*!
 * \brief Define the CLI Debugger function
 *
 * \details If the CLI Debugger is not included in the build this macro
 *          will be empty
 *
 */
#define CLI_DEBUGGER() do { } while (0)

#endif

/*!
 * @}
 */

/*!
 * @}
 */
#endif /* FWK_CLI_DBG_H */

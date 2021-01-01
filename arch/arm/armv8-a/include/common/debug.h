/*
 * Arm SCP/MCP Software
 * Copyright (c) 2013-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <lib/utils_def.h>

/*
 * The log output macros print output to the console. These macros produce
 * compiled log output only if the LOG_LEVEL defined in the makefile (or the
 * make command line) is greater or equal than the level required for that
 * type of log output.
 *
 * The format expected is the same as for printf(). For example:
 * INFO("Info %s.\n", "message")    -> INFO:    Info message.
 * WARN("Warning %s.\n", "message") -> WARNING: Warning message.
 */

#define LOG_LEVEL_NONE U(0)
#define LOG_LEVEL_ERROR U(10)
#define LOG_LEVEL_NOTICE U(20)
#define LOG_LEVEL_WARNING U(30)
#define LOG_LEVEL_INFO U(40)
#define LOG_LEVEL_VERBOSE U(50)

#endif /* DEBUG_H */

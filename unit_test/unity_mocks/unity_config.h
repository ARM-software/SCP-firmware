/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H
#include <fwk_log.h>
#define UNITY_OUTPUT_CHAR(a) fwk_io_putch(fwk_io_stdout, a)
#define UNITY_OUTPUT_FLUSH()

#endif

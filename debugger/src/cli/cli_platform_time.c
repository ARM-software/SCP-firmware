/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cli.h>
#include <cli_platform.h>

#include <stdint.h>

__attribute__((weak)) void cli_platform_get_time(cli_timestamp_t *t)
{
}

__attribute__((weak)) void cli_platform_delay_ms(uint32_t ms)
{
}

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <cli.h>
#include <cli_platform.h>

#include <fwk_attributes.h>

#include <stdint.h>

FWK_WEAK void cli_platform_get_time(cli_timestamp_t *t)
{
}

FWK_WEAK void cli_platform_delay_ms(uint32_t ms)
{
}

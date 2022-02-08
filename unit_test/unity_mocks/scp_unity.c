/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <scp_unity.h>

#include <stdarg.h>
#include <stdio.h>

int _ut_unit_status = UT_SUCCESS;

int _ut_fail_line_no;

void __wrap_fwk_log_printf(const char *format, ...)
{
    char buffer[88];

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    printf("%s\n", buffer);
    va_end(args);
}

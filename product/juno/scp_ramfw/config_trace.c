/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_time.h>
#include <fwk_trace.h>

#include <inttypes.h>

#define UNIT_STR "ns"

fwk_trace_count_t get_trace_count(void)
{
    return FWK_NS(fwk_time_current());
}

void report_trace_entry(
    const char *filename,
    const char *func,
    const unsigned int line,
    const fwk_trace_id_t id,
    const fwk_trace_count_t trace_count,
    const char *msg)
{
    FWK_LOG_CRIT(
        "%s:%d:[TRACE][%" PRItraceid "]: %" PRIu32 " " UNIT_STR,
        func,
        line,
        id,
        (uint32_t)trace_count);
}

struct fwk_trace_driver fmw_trace_driver(void)
{
    return (struct fwk_trace_driver){
        .trace_entry_count = 5,
        .get_trace_count = get_trace_count,
        .report_trace_entry = report_trace_entry,
    };
}

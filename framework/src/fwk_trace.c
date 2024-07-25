/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_attributes.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_string.h>
#include <fwk_trace.h>

#include <string.h>

static struct {
    struct fwk_trace_driver driver; /* Trace driver */
    fwk_trace_count_t *start_timestamp;
    bool *entry_pending;
} fwk_trace_ctx;

FWK_CONSTRUCTOR void fwk_trace_init(void)
{
    struct fwk_trace_driver driver = fmw_trace_driver();
    (void)fwk_str_memcpy(&fwk_trace_ctx.driver, &driver, sizeof(driver));
    if (fwk_trace_ctx.driver.trace_entry_count != 0) {
        fwk_trace_ctx.start_timestamp = fwk_mm_calloc(
            fwk_trace_ctx.driver.trace_entry_count + 1,
            sizeof(fwk_trace_count_t));
        fwk_trace_ctx.entry_pending = fwk_mm_calloc(
            fwk_trace_ctx.driver.trace_entry_count + 1,
            sizeof(fwk_trace_count_t));
    }
}

static inline fwk_trace_count_t calc_delta(
    fwk_trace_count_t start,
    fwk_trace_count_t end)
{
    return (end >= start) ? (end - start) :
                            ((fwk_trace_count_t)(-1) - start + end);
}

fwk_trace_count_t fwk_trace_calc_overhead(void)
{
    fwk_trace_count_t start = fwk_trace_ctx.driver.get_trace_count();
    (void)FWK_TRACE_START(fwk_trace_ctx.driver.trace_entry_count);
    (void)FWK_TRACE_FINISH(fwk_trace_ctx.driver.trace_entry_count, "");
    return calc_delta(start, fwk_trace_ctx.driver.get_trace_count());
}

int fwk_trace_start(fwk_trace_id_t id)
{
    if (id >= fwk_trace_ctx.driver.trace_entry_count) {
        FWK_LOG_ERR("id is not valid");
        return FWK_E_PARAM;
    }
    if (fwk_trace_ctx.entry_pending[id] == true) {
        FWK_LOG_ERR("tracing id 0x%" PRItraceid " has already started", id);
        return FWK_E_STATE;
    }
    if (fwk_trace_ctx.driver.get_trace_count == NULL) {
        FWK_LOG_ERR("start trace driver is not set!");
        return FWK_E_DEVICE;
    }
    fwk_trace_ctx.entry_pending[id] = true;
    fwk_trace_ctx.start_timestamp[id] = fwk_trace_ctx.driver.get_trace_count();

    return FWK_SUCCESS;
}

int fwk_trace_finish(
    const char *filename,
    const char *func,
    const unsigned int line,
    fwk_trace_id_t id,
    const char *msg)
{
    if (id >= fwk_trace_ctx.driver.trace_entry_count) {
        FWK_LOG_ERR("id is not valid");
        return FWK_E_PARAM;
    }
    if (fwk_trace_ctx.entry_pending[id] == false) {
        FWK_LOG_ERR(
            "%s:%u: tracing id 0x%" PRItraceid " has not been started",
            func,
            line,
            id);
        return FWK_E_STATE;
    }
    fwk_trace_ctx.entry_pending[id] = false;
    if (fwk_trace_ctx.driver.get_trace_count == NULL ||
        fwk_trace_ctx.driver.report_trace_entry == NULL) {
        FWK_LOG_ERR("finish trace driver is not set!");
        return FWK_E_DEVICE;
    }
    fwk_trace_count_t trace_count = calc_delta(
        fwk_trace_ctx.start_timestamp[id],
        fwk_trace_ctx.driver.get_trace_count());
    fwk_trace_ctx.start_timestamp[id] = 0;
    fwk_trace_ctx.driver.report_trace_entry(
        filename, func, line, id, trace_count, msg);

    return FWK_SUCCESS;
}

FWK_WEAK struct fwk_trace_driver fmw_trace_driver(void)
{
    return (struct fwk_trace_driver){
        .trace_entry_count = 0,
        .get_trace_count = NULL,
        .report_trace_entry = NULL,
    };
}

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_arch.h>
#include <fwk_attributes.h>
#include <fwk_status.h>
#include <fwk_time.h>

#include <string.h>

struct {
    struct fwk_time_driver driver; /* Time driver */
    const void *driver_ctx; /* Time driver context */
} fwk_time_ctx;

static FWK_CONSTRUCTOR void fwk_time_init(void)
{
    struct fwk_time_driver driver = fmw_time_driver(&fwk_time_ctx.driver_ctx);

    (void)memcpy(&fwk_time_ctx.driver, &driver, sizeof(driver));
}

fwk_timestamp_t fwk_time_current(void)
{
    if (fwk_time_ctx.driver.timestamp == NULL) {
        return FWK_NS(0);
    }

    return fwk_time_ctx.driver.timestamp(fwk_time_ctx.driver_ctx);
}

fwk_duration_ns_t fwk_time_stamp_duration(fwk_timestamp_t timestamp)
{
    return FWK_NS(timestamp);
}

fwk_duration_ns_t fwk_time_duration(fwk_timestamp_t start, fwk_timestamp_t end)
{
    fwk_assert(end > start);

    return end - start;
}

fwk_duration_us_t fwk_time_duration_us(fwk_duration_ns_t duration)
{
    return duration / FWK_US(1);
}

fwk_duration_ms_t fwk_time_duration_ms(fwk_duration_ns_t duration)
{
    return duration / FWK_MS(1);
}

fwk_duration_s_t fwk_time_duration_s(fwk_duration_ns_t duration)
{
    return duration / FWK_S(1);
}

fwk_duration_m_t fwk_time_duration_m(fwk_duration_ns_t duration)
{
    return (uint32_t)(duration / FWK_M(1));
}

fwk_duration_h_t fwk_time_duration_h(fwk_duration_ns_t duration)
{
    return (uint32_t)(duration / FWK_H(1));
}

FWK_WEAK struct fwk_time_driver fmw_time_driver(const void **ctx)
{
    return (struct fwk_time_driver){
        .timestamp = NULL,
    };
}

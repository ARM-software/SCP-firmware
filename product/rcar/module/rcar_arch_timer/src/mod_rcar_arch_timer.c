/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2021, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>

#include <mod_clock.h>
#include <mod_rcar_arch_timer.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_time.h>

#include <stddef.h>
#include <stdint.h>

#define CNTBASE_P_CTL_ENABLE UINT32_C(0x00000001)
#define CNTBASE_P_CTL_IMASK UINT32_C(0x00000002)
#define CNTBASE_P_CTL_ISTATUS UINT32_C(0x00000004)
#define GTIMER_MIN_TIMESTAMP 2000

/* Device content */
struct dev_ctx {
    const struct mod_arch_timer_dev_config *config;
};

static struct mod_arch_timer_ctx {
    bool initialized; /* Whether the device context has been initialized */

    struct dev_ctx *table; /* Device context table */
} mod_arch_timer_ctx;

static uint64_t mod_arch_timer_get_counter(void)
{
    uint64_t counter;

    counter = read_cntpct_el0();

    return counter;
}

/*
 * Functions fulfilling the Timer module's driver interface
 */

static int enable(fwk_id_t dev_id)
{
    uint32_t cntp_ctl;

    cntp_ctl = read_cntp_ctl_el0() & ~CNTBASE_P_CTL_IMASK;
    cntp_ctl |= CNTBASE_P_CTL_ENABLE;
    write_cntp_ctl_el0(cntp_ctl);

    return FWK_SUCCESS;
}

static int disable(fwk_id_t dev_id)
{
    uint32_t cntp_ctl;

    cntp_ctl = read_cntp_ctl_el0() | CNTBASE_P_CTL_IMASK;
    cntp_ctl &= ~CNTBASE_P_CTL_ENABLE;
    write_cntp_ctl_el0(cntp_ctl);

    return FWK_SUCCESS;
}

static int get_counter(fwk_id_t dev_id, uint64_t *value)
{
    *value = mod_arch_timer_get_counter();

    return FWK_SUCCESS;
}

static int set_timer(fwk_id_t dev_id, uint64_t timestamp)
{
    uint64_t counter;
    int status;

    status = get_counter(dev_id, &counter);
    if (status != FWK_SUCCESS)
        return status;

    timestamp = FWK_MAX(counter + GTIMER_MIN_TIMESTAMP, timestamp);

    write_cntp_cval_el0(timestamp);

    return FWK_SUCCESS;
}

static int get_timer(fwk_id_t dev_id, uint64_t *timestamp)
{
    *timestamp = read_cntp_cval_el0();

    return FWK_SUCCESS;
}

static int get_frequency(fwk_id_t dev_id, uint32_t *frequency)
{
    *frequency = read_cntfrq_el0();

    return FWK_SUCCESS;
}

static const struct mod_timer_driver_api module_api = {
    .name = "Arch Timer Driver",
    .enable = enable,
    .disable = disable,
    .set_timer = set_timer,
    .get_timer = get_timer,
    .get_counter = get_counter,
    .get_frequency = get_frequency,
};

/*
 * Functions fulfilling the framework's module interface
 */
static int arch_timer_init(fwk_id_t module_id,
                       unsigned int element_count,
                       const void *data)
{
    mod_arch_timer_ctx.table =
        fwk_mm_alloc(element_count, sizeof(struct dev_ctx));

    return FWK_SUCCESS;
}

static int arch_timer_device_init(fwk_id_t element_id, unsigned int unused,
                              const void *data)
{
    struct dev_ctx *ctx;

    ctx = mod_arch_timer_ctx.table;

    ctx->config = data;

    disable(element_id);

    return FWK_SUCCESS;
}

static int arch_timer_process_bind_request(fwk_id_t requester_id,
                                       fwk_id_t id,
                                       fwk_id_t api_type,
                                       const void **api)
{
    /* No binding to the module */
    if (fwk_module_is_valid_module_id(id))
        return FWK_E_ACCESS;

    *api = &module_api;

    return FWK_SUCCESS;
}

static void arch_timer_control_init(void)
{
    /* Disable & Imask */
    write_cntp_ctl_el0(CNTBASE_P_CTL_IMASK);
}

static int arch_timer_start(fwk_id_t id)
{
    struct dev_ctx *ctx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return FWK_SUCCESS;

    ctx = mod_arch_timer_ctx.table;

    if (!fwk_id_is_type(ctx->config->clock_id, FWK_ID_TYPE_NONE)) {
        /* Register for clock state notifications */
        return fwk_notification_subscribe(
            mod_clock_notification_id_state_changed,
            ctx->config->clock_id,
            id);
    } else
        arch_timer_control_init();

    return FWK_SUCCESS;
}

static int arch_timer_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params;

    fwk_assert(
        fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed));
    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    params = (struct clock_notification_params *)event->params;

    if (params->new_state == MOD_CLOCK_STATE_RUNNING) {
        arch_timer_control_init();
    }

    return FWK_SUCCESS;
}

/*
 * Module descriptor
 */
const struct fwk_module module_rcar_arch_timer = {
    .api_count = 1,
    .event_count = 0,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = arch_timer_init,
    .element_init = arch_timer_device_init,
    .start = arch_timer_start,
    .process_bind_request = arch_timer_process_bind_request,
    .process_notification = arch_timer_process_notification,
};

static fwk_timestamp_t mod_arch_timer_timestamp(const void *ctx)
{
    fwk_timestamp_t timestamp;

    uint32_t frequency = read_cntfrq_el0();
    uint64_t counter = mod_arch_timer_get_counter();

    timestamp = (FWK_S(1) / frequency) * counter;

    return timestamp;
}

struct fwk_time_driver mod_arch_timer_driver(
    const void **ctx,
    const struct mod_arch_timer_dev_config *cfg)
{
    *ctx = cfg;

    return (struct fwk_time_driver){
        .timestamp = mod_arch_timer_timestamp,
    };
}

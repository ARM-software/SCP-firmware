/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gtimer_reg.h>

#include <mod_clock.h>
#include <mod_gtimer.h>
#include <mod_timer.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>

#define GTIMER_FREQUENCY_MIN_HZ  UINT32_C(1)
#define GTIMER_FREQUENCY_MAX_HZ  UINT32_C(1000000000)
#define GTIMER_MIN_TIMESTAMP 2000

/* Device content */
struct dev_ctx {
    struct cntbase_reg *hw_timer;
    struct cntctl_reg *hw_counter;
    struct cntcontrol_reg *control;
    const struct mod_gtimer_dev_config *config;
};

/* Table of device context structures */
static struct dev_ctx *ctx_table;

/*
 * Functions fulfilling the Timer module's driver interface
 */

static int enable(fwk_id_t dev_id)
{
    struct dev_ctx *ctx;

    ctx = ctx_table + fwk_id_get_element_idx(dev_id);

    ctx->hw_timer->P_CTL &= ~CNTBASE_P_CTL_IMASK;
    ctx->hw_timer->P_CTL |=  CNTBASE_P_CTL_ENABLE;

    return FWK_SUCCESS;
}

static int disable(fwk_id_t dev_id)
{
    struct dev_ctx *ctx;

    ctx = ctx_table + fwk_id_get_element_idx(dev_id);

    ctx->hw_timer->P_CTL |=  CNTBASE_P_CTL_IMASK;
    ctx->hw_timer->P_CTL &= ~CNTBASE_P_CTL_ENABLE;

    return FWK_SUCCESS;
}

static int get_counter(fwk_id_t dev_id, uint64_t *value)
{
    const struct dev_ctx *ctx;
    uint32_t counter_low;
    uint32_t counter_high;

    ctx = ctx_table + fwk_id_get_element_idx(dev_id);

    /*
     * To avoid race conditions where the high half of the counter increments
     * after it has been sampled but before the low half is sampled, the values
     * are resampled until the high half has stabilized. This assumes that the
     * loop is faster than the high half incrementation.
     */
    do {
        counter_high = ctx->hw_timer->PCTH;
        counter_low = ctx->hw_timer->PCTL;
    } while (counter_high != ctx->hw_timer->PCTH);

    *value = ((uint64_t)counter_high << 32) | counter_low;

    return FWK_SUCCESS;
}

static int set_timer(fwk_id_t dev_id, uint64_t timestamp)
{
    struct dev_ctx *ctx;
    uint64_t counter;
    int status;

    status = get_counter(dev_id, &counter);
    if (status != FWK_SUCCESS)
        return status;

    /*
     * If an alarm's period is very small, the timer device could be configured
     * to interrupt on a timestamp that is "in the past". In this case, with the
     * current FVP implementation an interrupt will not be generated. To avoid
     * this issue, the minimum timestamp is GTIMER_MIN_TIMESTAMP ticks from now.
     *
     * It is assumed here that the 64-bit counter will never loop back during
     * the course of the execution (@1GHz it would loop back after ~585 years).
     */
    timestamp = FWK_MAX(counter + GTIMER_MIN_TIMESTAMP, timestamp);

    ctx = ctx_table + fwk_id_get_element_idx(dev_id);

    ctx->hw_timer->P_CVALL = timestamp & 0xFFFFFFFF;
    ctx->hw_timer->P_CVALH = timestamp >> 32;

    return FWK_SUCCESS;
}

static int get_timer(fwk_id_t dev_id, uint64_t *timestamp)
{
    struct dev_ctx *ctx;
    uint32_t counter_low;
    uint32_t counter_high;

    ctx = ctx_table + fwk_id_get_element_idx(dev_id);

    /* Read 64-bit timer value */
    counter_low = ctx->hw_timer->P_CVALL;
    counter_high = ctx->hw_timer->P_CVALH;

    *timestamp = ((uint64_t)counter_high << 32) | counter_low;

    return FWK_SUCCESS;
}

static int get_frequency(fwk_id_t dev_id, uint32_t *frequency)
{
    struct dev_ctx *ctx;

    if (frequency == NULL)
        return FWK_E_PARAM;

    ctx = ctx_table + fwk_id_get_element_idx(dev_id);

    *frequency = ctx->config->frequency;

    return FWK_SUCCESS;
}

static const struct mod_timer_driver_api module_api = {
    .name = "Generic Timer Driver",
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

static int gtimer_init(fwk_id_t module_id,
                       unsigned int element_count,
                       const void *data)
{
    ctx_table = fwk_mm_alloc(element_count, sizeof(struct dev_ctx));

    return FWK_SUCCESS;
}

static int gtimer_device_init(fwk_id_t element_id, unsigned int unused,
                              const void *data)
{
    struct dev_ctx *ctx;

    ctx = ctx_table + fwk_id_get_element_idx(element_id);

    ctx->config = data;
    if (ctx->config->hw_timer == 0   ||
        ctx->config->hw_counter == 0 ||
        ctx->config->control == 0    ||
        ctx->config->frequency < GTIMER_FREQUENCY_MIN_HZ ||
        ctx->config->frequency > GTIMER_FREQUENCY_MAX_HZ) {

        return FWK_E_DEVICE;
    }

    ctx->hw_timer   = (struct cntbase_reg *)ctx->config->hw_timer;
    ctx->hw_counter = (struct cntctl_reg *)ctx->config->hw_counter;
    ctx->control    = (struct cntcontrol_reg *)ctx->config->control;

    disable(element_id);

    ctx->hw_counter->ACR = CNTCTL_ACR_RPCT |
                           CNTCTL_ACR_RVCT |
                           CNTCTL_ACR_RFRQ |
                           CNTCTL_ACR_RVOFF|
                           CNTCTL_ACR_RWPT;
    ctx->hw_counter->FRQ = ctx->config->frequency;

    return FWK_SUCCESS;
}

static int gtimer_process_bind_request(fwk_id_t requester_id,
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

static void gtimer_control_init(struct dev_ctx *ctx)
{
    /* Set primary counter update frequency and enable counter. */
    ctx->control->CR &= ~CNTCONTROL_CR_EN;
    ctx->control->FID0 = ctx->config->frequency;
    ctx->control->CR |= CNTCONTROL_CR_FCREQ | CNTCONTROL_CR_EN;
}

static int gtimer_start(fwk_id_t id)
{
    struct dev_ctx *ctx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return FWK_SUCCESS;

    ctx = ctx_table + fwk_id_get_element_idx(id);

    if (!fwk_id_is_type(ctx->config->clock_id, FWK_ID_TYPE_NONE)) {
        /* Register for clock state notifications */
        return fwk_notification_subscribe(
            mod_clock_notification_id_state_changed,
            ctx->config->clock_id,
            id);
    } else
        gtimer_control_init(ctx);

    return FWK_SUCCESS;
}

static int gtimer_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params;
    struct dev_ctx *ctx;

    assert(fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed));
    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    params = (struct clock_notification_params *)event->params;

    if (params->new_state == MOD_CLOCK_STATE_RUNNING) {
        ctx = ctx_table + fwk_id_get_element_idx(event->target_id);
        gtimer_control_init(ctx);
    }

    return FWK_SUCCESS;
}


/*
 * Module descriptor
 */
const struct fwk_module module_gtimer = {
    .name = "Generic Timer Driver",
    .api_count = 1,
    .event_count = 0,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = gtimer_init,
    .element_init = gtimer_device_init,
    .start = gtimer_start,
    .process_bind_request = gtimer_process_bind_request,
    .process_notification = gtimer_process_notification,
};

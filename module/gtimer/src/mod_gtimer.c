/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtimer_reg.h"

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
#include <fwk_time.h>

#include <stddef.h>
#include <stdint.h>

#define GTIMER_FREQUENCY_MIN_HZ  UINT32_C(1)
#define GTIMER_FREQUENCY_MAX_HZ  UINT32_C(1000000000)
#define GTIMER_MIN_TIMESTAMP 2000

/*
 * The offset range of implementation defined registers in the system counter
 * register frame is 0xC0 to 0xFC as defined by the Arm Architecture Reference
 * manual. Each impdef register in this offset range is treated as a 32-bit
 * register by the gtimer module. So the maximum number of impdef registers
 * configurable is 16.
 */
#define MAX_IMPDEF_CFG_CNT (16)
#define IS_IMPDEF_OFFSET_VALID(offset) \
    (((offset >= 0xC0) && (offset <= 0xFC)) && ((offset & 0x3u) == 0))

/* Device content */
struct gtimer_dev_ctx {
    struct cntbase_reg *hw_timer;
    struct cntctl_reg *hw_counter;
    struct cntcontrol_reg *control;
    const struct mod_gtimer_dev_config *config;
};

static struct mod_gtimer_mod_ctx {
    bool initialized; /* Whether the device context has been initialized */

    struct gtimer_dev_ctx *table; /* Device context table */
} mod_gtimer_ctx;

static uint64_t mod_gtimer_get_counter(const struct cntbase_reg *hw_timer)
{
    uint32_t counter_low;
    uint32_t counter_high;

    /*
     * To avoid race conditions where the high half of the counter increments
     * after it has been sampled but before the low half is sampled, the values
     * are resampled until the high half has stabilized. This assumes that the
     * loop is faster than the high half incrementation.
     */

    do {
        counter_high = hw_timer->PCTH;
        counter_low = hw_timer->PCTL;
    } while (counter_high != hw_timer->PCTH);

    return ((uint64_t)counter_high << 32) | counter_low;
}

/*
 * Functions fulfilling the Timer module's driver interface
 */

static int enable(fwk_id_t dev_id)
{
    struct gtimer_dev_ctx *ctx;

    ctx = mod_gtimer_ctx.table + fwk_id_get_element_idx(dev_id);

    ctx->hw_timer->P_CTL &= ~CNTBASE_P_CTL_IMASK;
    ctx->hw_timer->P_CTL |=  CNTBASE_P_CTL_ENABLE;

    return FWK_SUCCESS;
}

static int disable(fwk_id_t dev_id)
{
    struct gtimer_dev_ctx *ctx;

    ctx = mod_gtimer_ctx.table + fwk_id_get_element_idx(dev_id);

    ctx->hw_timer->P_CTL |=  CNTBASE_P_CTL_IMASK;
    ctx->hw_timer->P_CTL &= ~CNTBASE_P_CTL_ENABLE;

    return FWK_SUCCESS;
}

static int get_counter(fwk_id_t dev_id, uint64_t *value)
{
    const struct gtimer_dev_ctx *ctx;

    ctx = mod_gtimer_ctx.table + fwk_id_get_element_idx(dev_id);

    *value = mod_gtimer_get_counter(ctx->hw_timer);

    return FWK_SUCCESS;
}

static int set_timer(fwk_id_t dev_id, uint64_t timestamp)
{
    struct gtimer_dev_ctx *ctx;
    uint64_t counter;
    int status;

    status = get_counter(dev_id, &counter);
    if (status != FWK_SUCCESS) {
        return status;
    }

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

    ctx = mod_gtimer_ctx.table + fwk_id_get_element_idx(dev_id);

    ctx->hw_timer->P_CVALL = (uint32_t)(timestamp & 0xFFFFFFFFUL);
    ctx->hw_timer->P_CVALH = (uint32_t)(timestamp >> 32UL);

    return FWK_SUCCESS;
}

static int get_timer(fwk_id_t dev_id, uint64_t *timestamp)
{
    struct gtimer_dev_ctx *ctx;
    uint32_t counter_low;
    uint32_t counter_high;

    ctx = mod_gtimer_ctx.table + fwk_id_get_element_idx(dev_id);

    /* Read 64-bit timer value */
    counter_low = ctx->hw_timer->P_CVALL;
    counter_high = ctx->hw_timer->P_CVALH;

    *timestamp = ((uint64_t)counter_high << 32) | counter_low;

    return FWK_SUCCESS;
}

static int get_frequency(fwk_id_t dev_id, uint32_t *frequency)
{
    struct gtimer_dev_ctx *ctx;

    if (frequency == NULL) {
        return FWK_E_PARAM;
    }

    ctx = mod_gtimer_ctx.table + fwk_id_get_element_idx(dev_id);

    *frequency = ctx->config->frequency;

    return FWK_SUCCESS;
}

static const struct mod_timer_driver_api module_api = {
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
    mod_gtimer_ctx.table =
        fwk_mm_calloc(element_count, sizeof(struct gtimer_dev_ctx));

    return FWK_SUCCESS;
}

static int gtimer_device_init(fwk_id_t element_id, unsigned int unused,
                              const void *data)
{
    int status;
    uint8_t cnt;
    struct gtimer_dev_ctx *ctx;
    struct mod_gtimer_syscounter_impdef_config *impdef_cfg;

    ctx = mod_gtimer_ctx.table + fwk_id_get_element_idx(element_id);

    ctx->config = data;
    if (ctx->config->hw_timer == 0   ||
        ctx->config->hw_counter == 0 ||
        ctx->config->control == 0    ||
        ctx->config->frequency < GTIMER_FREQUENCY_MIN_HZ ||
        ctx->config->frequency > GTIMER_FREQUENCY_MAX_HZ) {

        return FWK_E_DEVICE;
    }

    /*
     * Ensure the number of impdef register config table entries is within
     * bounds.
     */
    if (ctx->config->syscnt_impdef_cfg_cnt > MAX_IMPDEF_CFG_CNT) {
        return FWK_E_PARAM;
    }

    /*
     * If the number of impdef register config table entries is not zero,
     * the impdef register configuration table cannot be NULL.
     */
    impdef_cfg = ctx->config->syscnt_impdef_cfg;
    if ((ctx->config->syscnt_impdef_cfg_cnt != 0) && (impdef_cfg == NULL)) {
        return FWK_E_PARAM;
    }

    /*
     * Validate the register offsets specified in the impdef register config
     * table.
     */
    for (cnt = 0; cnt < ctx->config->syscnt_impdef_cfg_cnt; cnt++) {
        if (!IS_IMPDEF_OFFSET_VALID(impdef_cfg[cnt].offset)) {
            return FWK_E_PARAM;
        }
    }

    ctx->hw_timer   = (struct cntbase_reg *)ctx->config->hw_timer;
    ctx->hw_counter = (struct cntctl_reg *)ctx->config->hw_counter;
    ctx->control    = (struct cntcontrol_reg *)ctx->config->control;

    status = disable(element_id);
    fwk_assert(status == FWK_SUCCESS);

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
    enum mod_gtimer_api_idx api_idx;
    int status;

    /* No binding to the module */
    if (fwk_module_is_valid_module_id(id)) {
        return FWK_E_ACCESS;
    }

    api_idx = (enum mod_gtimer_api_idx)fwk_id_get_api_idx(api_type);
    switch (api_idx) {
    case MOD_GTIMER_API_IDX_DRIVER:
        *api = &module_api;
        status = FWK_SUCCESS;
        break;
    default:
        status = FWK_E_PARAM;
    }

    return status;
}

static void gtimer_control_init(struct gtimer_dev_ctx *ctx)
{
    struct mod_gtimer_syscounter_impdef_config *impdef_cfg;
    uintptr_t impdef_reg;
    uint8_t cnt;

    /* Disable counter */
    ctx->control->CR &= ~CNTCONTROL_CR_EN;

    /* Optionally, configure implementation specific registers */
    impdef_cfg = ctx->config->syscnt_impdef_cfg;
    for (cnt = 0; cnt < ctx->config->syscnt_impdef_cfg_cnt; cnt++) {
        impdef_reg = (uintptr_t)ctx->control + impdef_cfg[cnt].offset;
        *(volatile uintptr_t *)impdef_reg = impdef_cfg[cnt].value;
    }

    /* Set primary counter update frequency and enable counter. */
    ctx->control->FID0 = ctx->config->frequency;
    ctx->control->CR |= CNTCONTROL_CR_FCREQ | CNTCONTROL_CR_EN;
}

static int gtimer_start(fwk_id_t id)
{
    struct gtimer_dev_ctx *ctx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_SUCCESS;
    }

    ctx = mod_gtimer_ctx.table + fwk_id_get_element_idx(id);

    if (ctx->config->skip_cntcontrol_init) {
        return FWK_SUCCESS;
    }

    if (!fwk_id_is_type(ctx->config->clock_id, FWK_ID_TYPE_NONE)) {
        /* Register for clock state notifications */
        return fwk_notification_subscribe(
            mod_clock_notification_id_state_changed,
            ctx->config->clock_id,
            id);
    } else {
        gtimer_control_init(ctx);
    }

    return FWK_SUCCESS;
}

static int gtimer_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params;
    struct gtimer_dev_ctx *ctx;

    fwk_assert(
        fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed));
    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    params = (struct clock_notification_params *)event->params;

    if (params->new_state == MOD_CLOCK_STATE_RUNNING) {
        ctx = mod_gtimer_ctx.table + fwk_id_get_element_idx(event->target_id);
        gtimer_control_init(ctx);
    }

    return FWK_SUCCESS;
}

/*
 * Module descriptor
 */
const struct fwk_module module_gtimer = {
    .api_count = (unsigned int)MOD_GTIMER_API_IDX_COUNT,
    .event_count = 0,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = gtimer_init,
    .element_init = gtimer_device_init,
    .start = gtimer_start,
    .process_bind_request = gtimer_process_bind_request,
    .process_notification = gtimer_process_notification,
};

static fwk_timestamp_t mod_gtimer_timestamp(const void *ctx)
{
    fwk_timestamp_t timestamp;

    const struct mod_gtimer_dev_config *cfg = ctx;
    const struct cntbase_reg *hw_timer = (const void *)cfg->hw_timer;

    uint32_t frequency = cfg->frequency;
    uint64_t counter = mod_gtimer_get_counter(hw_timer);

    timestamp = (FWK_S(1) / frequency) * counter;

    return timestamp;
}

struct fwk_time_driver mod_gtimer_driver(
    const void **ctx,
    const struct mod_gtimer_dev_config *cfg)
{
    *ctx = cfg;

    return (struct fwk_time_driver){
        .timestamp = mod_gtimer_timestamp,
    };
}

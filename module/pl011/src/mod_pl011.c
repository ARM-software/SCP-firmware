/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pl011.h"

#include <mod_pl011.h>

#include <fwk_assert.h>
#include <fwk_attributes.h>
#include <fwk_event.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>

#ifdef BUILD_HAS_MOD_CLOCK
#    include <mod_clock.h>
#endif

#ifdef BUILD_HAS_MOD_POWER_DOMAIN
#    include <mod_power_domain.h>
#endif

#ifdef BUILD_HAS_MOD_SYSTEM_POWER
#    include <mod_system_power.h>
#endif

struct mod_pl011_element_ctx {
    /* Whether the device's power domain device is available */
    bool powered;

    /* Whether the device's clock domain is available */
    bool clocked;

    /* Whether the device has an open file stream */
    bool open;
};

static struct mod_pl011_ctx {
    bool initialized; /* Whether the context has been initialized */

    struct mod_pl011_element_ctx *elements; /* Element context table */
} pl011_ctx;

static void mod_pl011_init_ctx(void)
{
    size_t element_count;

    fwk_assert(!pl011_ctx.initialized);

    element_count = (size_t)fwk_module_get_element_count(fwk_module_id_pl011);
    if (element_count == 0) {
        return;
    }

    pl011_ctx.elements =
        fwk_mm_alloc(element_count, sizeof(pl011_ctx.elements[0]));

    for (size_t i = 0; i < element_count; i++) {
        const struct mod_pl011_element_cfg *cfg =
            fwk_module_get_data(FWK_ID_ELEMENT(FWK_MODULE_IDX_PL011, i));

        pl011_ctx.elements[i] = (struct mod_pl011_element_ctx){
            .powered = true, /* Assume the device is always powered */
            .clocked = true, /* Assume the device is always clocked */

            .open = false,
        };

#ifdef BUILD_HAS_MOD_POWER_DOMAIN
        pl011_ctx.elements[i].powered =
            fwk_id_is_equal(cfg->pd_id, FWK_ID_NONE);
#endif

#ifdef BUILD_HAS_MOD_CLOCK
        pl011_ctx.elements[i].clocked =
            fwk_id_is_equal(cfg->clock_id, FWK_ID_NONE);
#endif

        (void)cfg;
    }

    pl011_ctx.initialized = true;
}

static void mod_pl011_set_baud_rate(const struct mod_pl011_element_cfg *cfg)
{
    struct pl011_reg *reg = (void *)cfg->reg_base;

    uint32_t clock_rate_x4;

    uint32_t divisor_integer;
    uint32_t divisor_fractional;
    uint32_t divisor;

    fwk_assert(reg != NULL);

    fwk_assert(cfg->baud_rate_bps != 0);
    fwk_assert(cfg->clock_rate_hz >= PL011_UARTCLK_MIN);
    fwk_assert(cfg->clock_rate_hz <= PL011_UARTCLK_MAX);

    /* The highest clock x4 should still fit in 32 bits */
    fwk_assert((PL011_UARTCLK_MAX * UINT64_C(4)) < UINT32_MAX);

    /* Ensure baud rate is not higher than the clock can support */
    clock_rate_x4 = (uint32_t)(cfg->clock_rate_hz * 4);
    fwk_assert(cfg->baud_rate_bps <= clock_rate_x4);

    /* Calculate integer and fractional divisors */
    divisor = clock_rate_x4 / cfg->baud_rate_bps;
    divisor_integer = divisor / 64;
    divisor_fractional = divisor % 64;

    /* The integer divisor must fit in 16 bits */
    fwk_assert(divisor_integer <= 0xFFFF);

    /* The fractional divisor must fit in 6 bits */
    fwk_assert(divisor_fractional <= 0x3F);

    /*
     * When the integer divisor equals 0xFFFF, the fractional divisor can only
     * be 0.
     */
    fwk_assert(!((divisor_integer == 0xFFFF) && (divisor_fractional != 0)));

    reg->IBRD = (uint16_t)divisor_integer;
    reg->FBRD = divisor_fractional;
}

static void mod_pl011_enable(fwk_id_t id)
{
    const struct mod_pl011_element_cfg *cfg = fwk_module_get_data(id);
    struct mod_pl011_element_ctx *ctx =
        &pl011_ctx.elements[fwk_id_get_element_idx(id)];

    struct pl011_reg *reg = (void *)cfg->reg_base;

    fwk_assert(ctx->powered); /* Must be powered to enable */
    fwk_assert(ctx->clocked); /* Must be clocked to enable */

    mod_pl011_set_baud_rate(cfg);

    reg->ECR = PL011_ECR_CLR;
    reg->LCR_H = PL011_LCR_H_WLEN_8BITS | PL011_LCR_H_FEN;
    reg->CR = PL011_CR_UARTEN | PL011_CR_RXE | PL011_CR_TXE;
}

static bool mod_pl011_putch(fwk_id_t id, char ch)
{
    const struct mod_pl011_element_cfg *cfg = fwk_module_get_data(id);
    struct mod_pl011_element_ctx *ctx =
        &pl011_ctx.elements[fwk_id_get_element_idx(id)];

    struct pl011_reg *reg = (void *)cfg->reg_base;

    fwk_assert(ctx->powered);
    fwk_assert(ctx->clocked);

    /* Check if buffer is full. */
    if ((reg->FR & PL011_FR_TXFF) > 0) {
        return false;
    }

    reg->DR = (uint16_t)ch;

    return true;
}

static bool mod_pl011_getch(fwk_id_t id, char *ch)
{
    const struct mod_pl011_element_cfg *cfg = fwk_module_get_data(id);
    struct mod_pl011_element_ctx *ctx =
        &pl011_ctx.elements[fwk_id_get_element_idx(id)];

    struct pl011_reg *reg = (void *)cfg->reg_base;

    fwk_assert(ctx->powered);
    fwk_assert(ctx->clocked);

    if (reg->FR & PL011_FR_RXFE) {
        return false;
    }

    *ch = (char)reg->DR;

    return true;
}

static void mod_pl011_flush(fwk_id_t id)
{
    const struct mod_pl011_element_cfg *cfg = fwk_module_get_data(id);
    struct mod_pl011_element_ctx *ctx =
        &pl011_ctx.elements[fwk_id_get_element_idx(id)];

    struct pl011_reg *reg = (void *)cfg->reg_base;

    fwk_assert(ctx->powered);
    fwk_assert(ctx->clocked);

    while (reg->FR & PL011_FR_BUSY) {
        continue;
    }
}

static int mod_pl011_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (pl011_ctx.initialized) {
        return FWK_SUCCESS;
    }

    mod_pl011_init_ctx();

    return FWK_SUCCESS;
}

static int mod_pl011_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    return FWK_SUCCESS;
}

static int mod_pl011_start(fwk_id_t id)
{
    int status = FWK_SUCCESS;

    const struct mod_pl011_element_cfg *cfg;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    cfg = fwk_module_get_data(id);

#ifdef BUILD_HAS_MOD_POWER_DOMAIN
    /*
     * Subscribe to power domain pre-state change notifications when a power
     * domain identifier is provided. We are waiting for the power domain to
     * come online so that we can mark it as powered.
     */

    if (!fwk_id_is_equal(cfg->pd_id, FWK_ID_NONE)) {
        status = fwk_notification_subscribe(
            mod_pd_notification_id_power_state_transition, cfg->pd_id, id);
        if (!fwk_expect(status == FWK_SUCCESS)) {
            return FWK_E_HANDLER;
        }

        status = fwk_notification_subscribe(
            mod_pd_notification_id_pre_shutdown,
            fwk_module_id_power_domain,
            id);
        if (!fwk_expect(status == FWK_SUCCESS)) {
            return FWK_E_HANDLER;
        }
    }
#endif

#ifdef BUILD_HAS_MOD_CLOCK
    /*
     * Subscribe to clock state change notifications if a clock domain
     * identifier is provided. We are waiting for the clock to start so that we
     * can mark it as clocked.
     */

    if (!fwk_id_is_equal(cfg->clock_id, FWK_ID_NONE)) {
        status = fwk_notification_subscribe(
            mod_clock_notification_id_state_changed, cfg->clock_id, id);
        if (!fwk_expect(status == FWK_SUCCESS)) {
            return FWK_E_HANDLER;
        }
    }
#endif

    (void)cfg;
    (void)status;

    return FWK_SUCCESS;
}

#ifdef BUILD_HAS_MOD_POWER_DOMAIN
static int mod_pl011_powering_down(fwk_id_t id)
{
    int status;

    struct mod_pl011_element_ctx *ctx =
        &pl011_ctx.elements[fwk_id_get_element_idx(id)];
    const struct mod_pl011_element_cfg *cfg = fwk_module_get_data(id);

    ctx->powered = false; /* This device has gone offline */

    /* We don't care about the pre-transition power on notification */
    status = fwk_notification_unsubscribe(
        mod_pd_notification_id_power_state_pre_transition, cfg->pd_id, id);
    if (!fwk_expect(status == FWK_SUCCESS)) {
        return FWK_E_HANDLER;
    }

    status = fwk_notification_subscribe(
        mod_pd_notification_id_power_state_transition, cfg->pd_id, id);
    if (!fwk_expect(status == FWK_SUCCESS)) {
        return FWK_E_HANDLER;
    }

    return FWK_SUCCESS;
}

static int mod_pl011_powered_on(fwk_id_t id)
{
    int status;

    struct mod_pl011_element_ctx *ctx =
        &pl011_ctx.elements[fwk_id_get_element_idx(id)];
    const struct mod_pl011_element_cfg *cfg = fwk_module_get_data(id);

    ctx->powered = true; /* This device is powered again */

    /* We need only the pre-transition in order to prepare for power down */
    status = fwk_notification_unsubscribe(
        mod_pd_notification_id_power_state_transition, cfg->pd_id, id);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_notification_subscribe(
        mod_pd_notification_id_power_state_pre_transition, cfg->pd_id, id);

    if (ctx->clocked) { /* Re-enable if no clock domain is configured */
        mod_pl011_enable(id);
    }

    return status;
}

static int mod_pl011_process_power_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status = FWK_SUCCESS;

    switch (fwk_id_get_notification_idx(event->id)) {
    case (unsigned int)MOD_PD_NOTIFICATION_IDX_POWER_STATE_PRE_TRANSITION: {
        struct mod_pd_power_state_pre_transition_notification_params
            *pd_pre_transition_params;
        struct mod_pd_power_state_pre_transition_notification_resp_params
            *pd_resp_params;

        pd_pre_transition_params =
            (struct mod_pd_power_state_pre_transition_notification_params *)
                event->params;
        pd_resp_params =
            (struct mod_pd_power_state_pre_transition_notification_resp_params
                 *)resp_event->params;

        switch (pd_pre_transition_params->target_state) {
#    ifdef BUILD_HAS_MOD_SYSTEM_POWER
        case (unsigned int)MOD_SYSTEM_POWER_POWER_STATE_SLEEP0:
            FWK_FALLTHROUGH;
#    endif

        case (unsigned int)MOD_PD_STATE_OFF:
            status = mod_pl011_powering_down(event->target_id);

            pd_resp_params->status = status; /* Inform the power domain */

            break;

        default:
            break;
        }

        break;
    }

    case (unsigned int)MOD_PD_NOTIFICATION_IDX_POWER_STATE_TRANSITION: {
        struct mod_pd_power_state_transition_notification_params *params =
            (struct mod_pd_power_state_transition_notification_params *)
                event->params;

        if (params->state != MOD_PD_STATE_ON) {
            status = FWK_SUCCESS;
        } else {
            status = mod_pl011_powered_on(event->target_id);
        }

        break;
    }

    case (unsigned int)MOD_PD_NOTIFICATION_IDX_PRE_SHUTDOWN: {
        struct mod_pd_pre_shutdown_notif_resp_params
            *pd_pre_shutdown_resp_params =
                (struct mod_pd_pre_shutdown_notif_resp_params *)
                    resp_event->params;

        status = mod_pl011_powering_down(event->target_id);

        pd_pre_shutdown_resp_params->status = status;

        break;
    }
    }

    return status;
}
#endif

#ifdef BUILD_HAS_MOD_CLOCK
static int mod_pl011_clock_change_pending(
    const struct fwk_event *event,
    const struct fwk_event *resp_event)
{
    int status;

    const struct clock_notification_params *event_params =
        (const void *)event->params;
    struct clock_state_change_pending_resp_params *resp_event_params =
        (void *)resp_event->params;

    struct mod_pl011_element_ctx *ctx =
        &pl011_ctx.elements[fwk_id_get_element_idx(event->target_id)];
    const struct mod_pl011_element_cfg *cfg =
        fwk_module_get_data(event->target_id);

    *resp_event_params = (struct clock_state_change_pending_resp_params){
        .status = FWK_SUCCESS,
    };

    fwk_assert(event_params->new_state == MOD_CLOCK_STATE_STOPPED);

    mod_pl011_flush(event->target_id);

    ctx->clocked = false;

    status = fwk_notification_unsubscribe(
        mod_clock_notification_id_state_change_pending,
        cfg->clock_id,
        event->target_id);
    if (!fwk_expect(status == FWK_SUCCESS)) {
        return FWK_E_OS;
    }

    status = fwk_notification_subscribe(
        mod_clock_notification_id_state_changed,
        cfg->clock_id,
        event->target_id);
    if (!fwk_expect(status == FWK_SUCCESS)) {
        return FWK_E_OS;
    }

    return FWK_SUCCESS;
}

static int mod_pl011_clock_changed(const struct fwk_event *event)
{
    int status;

    const struct clock_notification_params *event_params =
        (const void *)event->params;

    struct mod_pl011_element_ctx *ctx =
        &pl011_ctx.elements[fwk_id_get_element_idx(event->target_id)];
    const struct mod_pl011_element_cfg *cfg =
        fwk_module_get_data(event->target_id);

    fwk_assert(event_params->new_state == MOD_CLOCK_STATE_RUNNING);

    ctx->clocked = true;

    status = fwk_notification_unsubscribe(
        mod_clock_notification_id_state_changed,
        cfg->clock_id,
        event->target_id);
    if (status != FWK_SUCCESS) {
        return FWK_E_OS;
    }

    status = fwk_notification_subscribe(
        mod_clock_notification_id_state_change_pending,
        cfg->clock_id,
        event->target_id);
    if (status != FWK_SUCCESS) {
        return FWK_E_OS;
    }

    if (ctx->powered) { /* Re-enable if no clock domain is configured */
        mod_pl011_enable(event->target_id);
    }

    return FWK_SUCCESS;
}

static int mod_pl011_process_clock_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status = FWK_SUCCESS;

    switch (fwk_id_get_notification_idx(event->id)) {
    case (unsigned int)MOD_CLOCK_NOTIFICATION_IDX_STATE_CHANGED:
        status = mod_pl011_clock_changed(event);

        break;

    case (unsigned int)MOD_CLOCK_NOTIFICATION_IDX_STATE_CHANGE_PENDING:
        status = mod_pl011_clock_change_pending(event, resp_event);

        break;
    }

    return status;
}
#endif

static int mod_pl011_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    int status = FWK_E_PARAM;

    enum fwk_module_idx module_idx;

    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    module_idx = (enum fwk_module_idx)fwk_id_get_module_idx(event->id);

    switch (module_idx) {
#ifdef BUILD_HAS_MOD_POWER_DOMAIN
    case FWK_MODULE_IDX_POWER_DOMAIN:
        status = mod_pl011_process_power_notification(event, resp_event);

        break;
#endif

#ifdef BUILD_HAS_MOD_CLOCK
    case FWK_MODULE_IDX_CLOCK:
        status = mod_pl011_process_clock_notification(event, resp_event);

        break;
#endif

    default:
        break;
    }

    return status;
}

static int mod_pl011_io_open(const struct fwk_io_stream *stream)
{
    struct mod_pl011_element_ctx *ctx;

    if (!fwk_id_is_type(stream->id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_E_SUPPORT;
    }

    if (!pl011_ctx.initialized) {
        mod_pl011_init_ctx();
    }

    ctx = &pl011_ctx.elements[fwk_id_get_element_idx(stream->id)];
    if (ctx->open) { /* Refuse to open the same device twice */
        return FWK_E_BUSY;
    }

    ctx->open = true;

    if (ctx->powered && ctx->clocked) {
        mod_pl011_enable(stream->id); /* Enable the device if possible */
    }

    return FWK_SUCCESS;
}

static int mod_pl011_io_getch(
    const struct fwk_io_stream *restrict stream,
    char *restrict ch)
{
    const struct mod_pl011_element_ctx *ctx =
        &pl011_ctx.elements[fwk_id_get_element_idx(stream->id)];

    bool ok = true;

    fwk_assert(ctx->open);

    if (!ctx->powered || !ctx->clocked) {
        return FWK_E_PWRSTATE;
    }

    ok = mod_pl011_getch(stream->id, ch);
    if (!ok) {
        return FWK_PENDING;
    }

    return FWK_SUCCESS;
}

static int mod_pl011_io_putch(const struct fwk_io_stream *stream, char ch)
{
    const struct mod_pl011_element_ctx *ctx =
        &pl011_ctx.elements[fwk_id_get_element_idx(stream->id)];

    fwk_assert(ctx->open);

    if (!ctx->powered || !ctx->clocked) {
        return FWK_E_PWRSTATE;
    }

    if (!mod_pl011_putch(stream->id, ch)) {
        return FWK_E_BUSY;
    }

    return FWK_SUCCESS;
}

static int mod_pl011_close(const struct fwk_io_stream *stream)
{
    struct mod_pl011_element_ctx *ctx;

    fwk_assert(stream != NULL); /* Validated by the framework */
    fwk_assert(fwk_module_is_valid_element_id(stream->id));

    mod_pl011_flush(stream->id);

    ctx = &pl011_ctx.elements[fwk_id_get_element_idx(stream->id)];
    fwk_assert(ctx->open);

    ctx->open = false;

    return FWK_SUCCESS;
}

const struct fwk_module module_pl011 = {
    .type = FWK_MODULE_TYPE_DRIVER,

    .init = mod_pl011_init,
    .element_init = mod_pl011_element_init,
    .start = mod_pl011_start,

    .process_notification = mod_pl011_process_notification,

    .adapter =
        (struct fwk_io_adapter){
            .open = mod_pl011_io_open,
            .getch = mod_pl011_io_getch,
            .putch = mod_pl011_io_putch,
            .close = mod_pl011_close,
        },
};

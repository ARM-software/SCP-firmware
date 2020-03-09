/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <pl011.h>

#include <mod_clock.h>
#include <mod_log.h>
#include <mod_pl011.h>
#include <mod_power_domain.h>

#if BUILD_HAS_MOD_POWER_DOMAIN
#    include <mod_system_power.h>
#endif

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stddef.h>

struct pl011_device_ctx {
    /* Pointer to configuration data */
    const struct mod_pl011_device_config *config;

    /*
     * Flags indicating whether the device is available.
     * Such availability depends on the power domain.
     */
    bool available;
};

static struct pl011_device_ctx *dev_ctx_table;

static struct pl011_reg *get_device_reg(fwk_id_t device_id)
{
    struct pl011_device_ctx *dev_ctx;

    dev_ctx = &dev_ctx_table[fwk_id_get_element_idx(device_id)];

    return (struct pl011_reg *)dev_ctx->config->reg_base;
}

/*
 * For details on the constants and equations used to calculate the baud rate
 * settings, please consult the PL011 TRM.
 */
int mod_pl011_set_baud_rate(unsigned int baud_rate_bps, uint64_t clock_rate_hz,
    uintptr_t reg_ptr)
{
    uint32_t divisor_integer;
    uint32_t divisor_fractional;
    uint32_t divisor;
    uint32_t clock_rate_x4;

    struct pl011_reg *reg = (struct pl011_reg *)reg_ptr;

    assert(reg);

    if (baud_rate_bps == 0)
        return FWK_E_PARAM;

    if ((clock_rate_hz < PL011_UARTCLK_MIN) ||
        (clock_rate_hz > PL011_UARTCLK_MAX))
        return FWK_E_PARAM;

    /* The highest clock x4 should still fit in 32 bits */
    assert((PL011_UARTCLK_MAX * UINT64_C(4)) < UINT32_MAX);

    /* Ensure baud rate is not higher than the clock can support */
    clock_rate_x4 = clock_rate_hz * 4;
    if (baud_rate_bps > clock_rate_x4)
        return FWK_E_RANGE;

    /* Calculate integer and fractional divisors */
    divisor = clock_rate_x4 / baud_rate_bps;
    divisor_integer = divisor / 64;
    divisor_fractional = divisor % 64;

    /* The integer divisor must fit in 16 bits */
    if (divisor_integer > 0xffff)
        return FWK_E_RANGE;

    /* The fractional divisor must fit in 6 bits */
    if (divisor_fractional > 0x3f)
        return FWK_E_RANGE;

    /*
     * When the integer divisor is equals to 0xffff, the fractional divisor can
     * only be 0.
     */
    if ((divisor_integer == 0xffff) && (divisor_fractional != 0))
        return FWK_E_RANGE;

    /* Set registers */
    reg->IBRD = divisor_integer;
    reg->FBRD = divisor_fractional;

    return FWK_SUCCESS;
}

/*
 * Module log driver API
 */

static int do_putchar(fwk_id_t device_id, char c)
{
    struct pl011_reg *reg;
    struct pl011_device_ctx *dev_ctx;

    reg = get_device_reg(device_id);

    dev_ctx = &dev_ctx_table[fwk_id_get_element_idx(device_id)];

    if (!dev_ctx->available)
        return FWK_SUCCESS;

    while (reg->FR & PL011_FR_TXFF)
        continue;

    reg->DR = c;

    return FWK_SUCCESS;
}

static int do_flush(fwk_id_t device_id)
{
    struct pl011_reg *reg;
    struct pl011_device_ctx *dev_ctx;

    reg = get_device_reg(device_id);

    dev_ctx = &dev_ctx_table[fwk_id_get_element_idx(device_id)];

    if (!dev_ctx->available)
        return FWK_SUCCESS;

    while (reg->FR & PL011_FR_BUSY)
        continue;

    return FWK_SUCCESS;
}

static const struct mod_log_driver_api driver_api = {
    .flush = do_flush,
    .putchar = do_putchar,
};

/*
 * Framework handlers
 */

static int pl011_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    if (element_count == 0)
        return FWK_E_DATA;

    dev_ctx_table = fwk_mm_calloc(element_count, sizeof(dev_ctx_table[0]));

    return FWK_SUCCESS;
}

static int pl011_element_init(fwk_id_t element_id, unsigned int unused,
                              const void *data)
{
    struct pl011_reg *reg;
    const struct mod_pl011_device_config *config = data;
    int status;
    struct pl011_device_ctx *dev_ctx;

    reg = (struct pl011_reg *)config->reg_base;
    if (reg == NULL)
        return FWK_E_DATA;

    status = mod_pl011_set_baud_rate(config->baud_rate_bps,
                           config->clock_rate_hz,
                           config->reg_base);
    if (status != FWK_SUCCESS)
        return status;

    /*
     * Initialize PL011 device
     */
    reg->ECR = PL011_ECR_CLR;
    reg->LCR_H = PL011_LCR_H_WLEN_8BITS |
                 PL011_LCR_H_FEN;
    reg->CR = PL011_CR_UARTEN |
              PL011_CR_RXE |
              PL011_CR_TXE;

    dev_ctx = &dev_ctx_table[fwk_id_get_element_idx(element_id)];
    dev_ctx->config = config;

    dev_ctx->available = true;

    return FWK_SUCCESS;
}

static int pl011_process_bind_request(fwk_id_t requester_id, fwk_id_t target_id,
    fwk_id_t api_id, const void **api)
{
    *api = &driver_api;

    return FWK_SUCCESS;
}

static int pl011_start(fwk_id_t id)
{
    const struct mod_pl011_device_config *config;
    struct pl011_device_ctx *dev_ctx;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    dev_ctx = &dev_ctx_table[fwk_id_get_element_idx(id)];
    config = dev_ctx->config;

    /*
     * Subscribe to power domain pre-state change notifications when identifier
     * is provided.
     */
    #if BUILD_HAS_MOD_POWER_DOMAIN
    int status;
    if (!fwk_id_is_type(config->pd_id, FWK_ID_TYPE_NONE)) {
        status = fwk_notification_subscribe(
            mod_pd_notification_id_power_state_pre_transition,
            config->pd_id,
            id);
        if (status != FWK_SUCCESS)
            return status;

        status = fwk_notification_subscribe(
            mod_pd_notification_id_pre_shutdown,
            fwk_module_id_power_domain,
            id);
        if (status != FWK_SUCCESS)
            return status;
    }
    #endif

    /*
     * Subscribe to clock state change notifications if identifier is provided
     */
    if (fwk_id_is_type(config->clock_id, FWK_ID_TYPE_NONE))
        return FWK_SUCCESS;

    return fwk_notification_subscribe(
        mod_clock_notification_id_state_change_pending,
        config->clock_id,
        id);
}

static int process_clock_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params;
    struct clock_state_change_pending_resp_params *resp_params;

    resp_params =
        (struct clock_state_change_pending_resp_params *)resp_event->params;
    resp_params->status = FWK_SUCCESS;

    params = (struct clock_notification_params *)event->params;

    if (params->new_state == MOD_CLOCK_STATE_STOPPED)
        return do_flush(event->target_id);

    return FWK_SUCCESS;
}

#if BUILD_HAS_MOD_POWER_DOMAIN
static int pl011_powerdown(fwk_id_t id)
{
    int status;
    struct pl011_device_ctx *dev_ctx;

    dev_ctx = &dev_ctx_table[fwk_id_get_element_idx(id)];

    /* The device is silently made not available */
    dev_ctx->available = false;

    status = fwk_notification_unsubscribe(
        mod_pd_notification_id_power_state_pre_transition,
        dev_ctx->config->pd_id,
        id);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_notification_subscribe(
        mod_pd_notification_id_power_state_transition,
        dev_ctx->config->pd_id,
        id);
    return status;
}

static int pl011_powerup(fwk_id_t id)
{
    int status;
    struct pl011_device_ctx *dev_ctx;

    dev_ctx = &dev_ctx_table[fwk_id_get_element_idx(id)];

    /* The device is available back again */
    dev_ctx->available = true;

    status = fwk_notification_unsubscribe(
        mod_pd_notification_id_power_state_transition,
        dev_ctx->config->pd_id,
        id);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_notification_subscribe(
        mod_pd_notification_id_power_state_pre_transition,
        dev_ctx->config->pd_id,
        id);
    return status;
}
#endif

static int pl011_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    if (fwk_id_is_equal(
        event->id,
        mod_clock_notification_id_state_change_pending)) {
        /*
         * Clock notification
         */
        return process_clock_notification(event, resp_event);
    #if BUILD_HAS_MOD_POWER_DOMAIN
    } else if (fwk_id_is_equal(
        event->id,
        mod_pd_notification_id_power_state_pre_transition)) {
        /*
         * Power domain pre-transition notification
         */
        int status = FWK_E_PARAM;
        struct mod_pd_power_state_pre_transition_notification_params
            *pd_pre_transition_params;
        struct mod_pd_power_state_pre_transition_notification_resp_params
            *pd_resp_params;

        pd_pre_transition_params =
        (struct mod_pd_power_state_pre_transition_notification_params *)event
            ->params;
        pd_resp_params =
        (struct mod_pd_power_state_pre_transition_notification_resp_params *)
            resp_event->params;

        if ((pd_pre_transition_params->target_state == MOD_PD_STATE_OFF) ||
            (pd_pre_transition_params->target_state ==
                MOD_SYSTEM_POWER_POWER_STATE_SLEEP0)) {

            status = pl011_powerdown(event->target_id);
            pd_resp_params->status = status;
        }

        return status;
    } else if (fwk_id_is_equal(
        event->id,
        mod_pd_notification_id_power_state_transition)) {
        /*
         * Power domain post-transition notification
         */
        struct mod_pd_power_state_transition_notification_params *params =
            (struct mod_pd_power_state_transition_notification_params *)
                event->params;

        if (params->state != MOD_PD_STATE_ON)
            return FWK_SUCCESS;

        return pl011_powerup(event->target_id);
    } else if (fwk_id_is_equal(event->id,
        mod_pd_notification_id_pre_shutdown)) {
        int status = FWK_E_PARAM;

        status = pl011_powerdown(event->target_id);

        struct mod_pd_pre_shutdown_notif_resp_params
            *pd_pre_shutdown_resp_params =
        (struct mod_pd_pre_shutdown_notif_resp_params *)
            resp_event->params;

        pd_pre_shutdown_resp_params->status = status;
        return status;
    #endif
    } else
        return FWK_E_PARAM;
}

const struct fwk_module module_pl011 = {
    .name = "PL011",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = pl011_init,
    .element_init = pl011_element_init,
    .start = pl011_start,
    .process_bind_request = pl011_process_bind_request,
    .process_notification = pl011_process_notification
};

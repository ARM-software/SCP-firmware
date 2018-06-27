/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_assert.h>
#include <fwk_errno.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <mod_clock.h>
#include <mod_log.h>
#include <mod_pl011.h>
#include <mod_power_domain.h>
#include <pl011.h>

static const struct mod_pl011_device_config **device_config_table;

static struct pl011_reg *get_device_reg(fwk_id_t device_id)
{
    unsigned int device_idx;

    device_idx = fwk_id_get_element_idx(device_id);
    return (struct pl011_reg *)device_config_table[device_idx]->reg_base;
}

/*
 * For details on the constants and equations used to calculate the baud rate
 * settings, please consult the PL011 TRM.
 */
static int set_baud_rate(unsigned int baud_rate_bps, uint64_t clock_rate_hz,
    struct pl011_reg *reg)
{
    uint32_t divisor_integer;
    uint32_t divisor_fractional;
    uint32_t divisor;
    uint32_t clock_rate_x4;

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
    int status;
    struct pl011_reg *reg;

    status = fwk_module_check_call(device_id);
    if (status != FWK_SUCCESS)
        return status;

    reg = get_device_reg(device_id);

    while (reg->FR & PL011_FR_TXFF)
        continue;

    reg->DR = c;

    return FWK_SUCCESS;
}

static int do_flush(fwk_id_t device_id)
{
    int status;
    struct pl011_reg *reg;

    status = fwk_module_check_call(device_id);
    if (status != FWK_SUCCESS)
        return status;

    reg = get_device_reg(device_id);

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

    /*
     * Create an array of pointers used to store the configuration data pointer
     * of each element.
     */
    device_config_table = fwk_mm_calloc(element_count,
                                        sizeof(*device_config_table));
    if (device_config_table == NULL)
        return FWK_E_NOMEM;

    return FWK_SUCCESS;
}

static int pl011_element_init(fwk_id_t element_id, unsigned int unused,
                              const void *data)
{
    struct pl011_reg *reg;
    const struct mod_pl011_device_config *config = data;
    int status;

    reg = (struct pl011_reg *)config->reg_base;
    if (reg == NULL)
        return FWK_E_DATA;

    status = set_baud_rate(config->baud_rate_bps,
                           config->clock_rate_hz,
                           reg);
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

    device_config_table[fwk_id_get_element_idx(element_id)] = config;

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

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return FWK_SUCCESS;

    config = device_config_table[fwk_id_get_element_idx(id)];

    if (fwk_id_is_type(config->clock_id, FWK_ID_TYPE_NONE))
        return FWK_SUCCESS;

    return fwk_notification_subscribe(
        mod_clock_notification_id_state_change_pending,
        config->clock_id,
        id);
}

int pl011_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params;
    struct clock_state_change_pending_resp_params *resp_params;

    assert(
        fwk_id_is_equal(
            event->id,
            mod_clock_notification_id_state_change_pending));
    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    resp_params =
        (struct clock_state_change_pending_resp_params *)resp_event->params;
    resp_params->status = FWK_SUCCESS;

    params = (struct clock_notification_params *)event->params;

    if (params->new_state == MOD_CLOCK_STATE_STOPPED)
        return do_flush(event->target_id);

    return FWK_SUCCESS;
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

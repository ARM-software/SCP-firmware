/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <mod_log.h>

static const struct mod_log_config *log_config;
static struct mod_log_driver_api *log_driver;

#define ALL_GROUPS_MASK (MOD_LOG_GROUP_DEBUG | \
                         MOD_LOG_GROUP_ERROR | \
                         MOD_LOG_GROUP_INFO | \
                         MOD_LOG_GROUP_WARNING)

static int do_putchar(char c)
{
    int status;

    /* Include a 'carriage return' before the new line */
    if (c == '\n') {
        status = do_putchar('\r');
        if (status != FWK_SUCCESS)
            return status;
    }

    status = log_driver->putchar(log_config->device_id, c);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}

static int print_uint64(uint64_t value, unsigned int base, unsigned int fill)
{
    /* Just need enough space to store 64 bit decimal integer */
    unsigned char str[20];
    unsigned int i = 0;
    int status;

    /* Decimal or hexadecimal only */
    assert((base == 10) || (base == 16));

    do {
        str[i++] = "0123456789abcdef"[value % base];
    } while (value /= base);

    while (fill-- > i) {
        status = do_putchar('0');
        if (status != FWK_SUCCESS)
            return status;
    }

    while (i > 0) {
        status = do_putchar(str[--i]);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int print_int32(int32_t num, unsigned int fill)
{
    int status;
    uint64_t unum;

    if (num < 0) {
        status = do_putchar('-');
        if (status != FWK_SUCCESS)
            return status;
        unum = (uint64_t)-num;
    } else
        unum = (uint64_t)num;

    return print_uint64(unum, 10, fill);
}

static int print_string(const char *str)
{
    int status;

    while (*str) {
        status = do_putchar(*str++);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int do_print(const char *fmt, va_list *args)
{
    int status;
    int bit64;
    int64_t num;
    uint64_t unum;
    unsigned int fill;

    while (*fmt) {

        if (*fmt == '%') {
            fmt++;
            bit64 = 0;
            fill = 0;

next_symbol:
            /* Check the format specifier */
            switch (*fmt) {
            case 'i': /* Fall through to next one */
            case 'd':
                if (bit64)
                    return FWK_E_DATA;

                num = va_arg(*args, int32_t);

                status = print_int32(num, fill);
                if (status != FWK_SUCCESS)
                    return status;
                break;

            case 's':
                status = print_string(va_arg(*args, const char *));
                if (status != FWK_SUCCESS)
                    return status;
                break;

            case 'c':
                status = do_putchar(va_arg(*args, int));
                if (status != FWK_SUCCESS)
                    return status;
                break;

            case 'x':
                if (bit64)
                    unum = va_arg(*args, uint64_t);
                else
                    unum = va_arg(*args, uint32_t);
                status = print_uint64(unum, 16, fill);
                if (status != FWK_SUCCESS)
                    return status;
                break;

            case 'l':
                bit64 = 1;
                fmt++;
                goto next_symbol;

            case 'u':
                if (bit64)
                    unum = va_arg(*args, uint64_t);
                else
                    unum = va_arg(*args, uint32_t);

                status = print_uint64(unum, 10, fill);
                if (status != FWK_SUCCESS)
                    return status;
                break;

            case '0':
                fmt++;
                if (((*fmt) < '0') || ((*fmt) > '9'))
                    return FWK_E_DATA;
                fill = *(fmt++) - '0';
                goto next_symbol;

            default:
                /* Exit on any other format specifier */
                return FWK_E_DATA;
            }
            fmt++;
            continue;
        }
        status = do_putchar(*fmt++);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static bool is_valid_group(unsigned int group)
{
    /* Check if group is 'none' */
    if (group == MOD_LOG_GROUP_NONE)
        return false;

    /* Check if group is within the limits of valid groups */
    if (group & ~ALL_GROUPS_MASK)
        return false;

    /* Check if only one group was set */
    return !(group & (group - 1));
}

/*
 * Module API
 */

static int do_log(enum mod_log_group group, const char *fmt, ...)
{
    int status;
    va_list args;

    /* API called too early */
    if (log_driver == NULL)
        return FWK_E_STATE;

    status = fwk_module_check_call(FWK_ID_MODULE(FWK_MODULE_IDX_LOG));
    if (status != FWK_SUCCESS)
        return status;

    if (!is_valid_group(group))
        return FWK_E_PARAM;

    if (fmt == NULL)
        return FWK_E_PARAM;

    if (group & log_config->log_groups) {
        va_start(args, fmt);
        status = do_print(fmt, &args);
        va_end(args);

        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int do_flush(void)
{
    int status;

    /* API called too early */
    if (log_driver == NULL)
        return FWK_E_STATE;

    status = fwk_module_check_call(FWK_ID_MODULE(FWK_MODULE_IDX_LOG));
    if (status != FWK_SUCCESS)
        return status;

    status = log_driver->flush(log_config->device_id);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}

static const struct mod_log_api module_api = {
    .log = do_log,
    .flush = do_flush,
};

/*
 * Framework handlers
 */
static int log_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    const struct mod_log_config *config = data;

    /* Module does not support elements */
    if (element_count > 0)
        return FWK_E_DATA;

    /* Check for invalid groups in the 'log_groups' mask */
    if (config->log_groups & ~ALL_GROUPS_MASK)
        return FWK_E_PARAM;

    log_config = config;

    return FWK_SUCCESS;
}

static int log_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct mod_log_driver_api *driver = NULL;

    /* Skip second round */
    if (round == 1)
        return FWK_SUCCESS;

    /* Get the device driver's API */
    status = fwk_module_bind(log_config->device_id,
                             log_config->api_id,
                             &driver);
    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    /* Validate device driver's API */
    if ((driver == NULL) ||
        (driver->flush == NULL) ||
        (driver->putchar == NULL))
        return FWK_E_DATA;

    log_driver = driver;

    if (log_config->banner) {
        status = do_log(MOD_LOG_GROUP_INFO, log_config->banner);
        if (status != FWK_SUCCESS)
            return status;

        status = do_flush();
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int log_process_bind_request(fwk_id_t requester_id, fwk_id_t id,
    fwk_id_t api_id, const void **api)
{
    *api = &module_api;

    return FWK_SUCCESS;
}

/* Module descriptor */
const struct fwk_module module_log = {
    .name = "Log",
    .type = FWK_MODULE_TYPE_HAL,
    .api_count = 1,
    .init = log_init,
    .bind = log_bind,
    .process_bind_request = log_process_bind_request,
};

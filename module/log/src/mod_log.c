/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_log.h>

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static const struct mod_log_config *log_config;
static struct mod_log_driver_api *log_driver;

static int log_backend_print(char ch)
{
    int status;

    status = log_driver->putchar(log_config->device_id, ch);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}

static int log_backend_flush(void)
{
    int status;

    status = log_driver->flush(log_config->device_id);
    if (status != FWK_SUCCESS)
        return FWK_E_DEVICE;

    return FWK_SUCCESS;
}

static int log_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    const struct mod_log_config *config = data;

    /* Module does not support elements */
    if (element_count > 0)
        return FWK_E_DATA;

    log_config = config;

    return FWK_SUCCESS;
}

static int log_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct mod_log_driver_api *driver = NULL;

    const char *banner;

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

    banner = log_config->banner;

    while (banner != NULL) {
        FWK_LOG_INFO("%s", banner);

        banner = strchr(banner, '\n');
        if (banner != NULL)
            banner += 1;
    }

    return FWK_SUCCESS;
}

static int log_start(fwk_id_t id)
{
    static const struct fwk_log_backend backend = {
        .print = log_backend_print,
        .flush = log_backend_flush,
    };

    fwk_assert(fwk_id_is_type(id, FWK_ID_TYPE_MODULE));

    return fwk_log_register(&backend);
}

/* Module descriptor */
const struct fwk_module module_log = {
    .name = "Log",
    .type = FWK_MODULE_TYPE_HAL,
    .init = log_init,
    .bind = log_bind,
    .start = log_start,
};

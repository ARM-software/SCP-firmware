/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_log.h>

#include <fwk_module.h>
#include <fwk_status.h>

#include <stdint.h>
#include <stdio.h>

/*
 * Module driver API
 */
static int do_putchar(fwk_id_t device, char c)
{
    if (putchar(c) == EOF)
        return FWK_E_DEVICE;
    else
        return FWK_SUCCESS;
}

static int do_flush(fwk_id_t device_id)
{
    if (fflush(stdout) == EOF)
        return FWK_E_DEVICE;
    else
        return FWK_SUCCESS;
}

static const struct mod_log_driver_api driver_api = {
    .flush = do_flush,
    .putchar = do_putchar,
};

/*
 * Module API for the framework
 */
static int init(fwk_id_t module_id, unsigned int element_count,
                const void *specific_config)
{
    return FWK_SUCCESS;
}

static int process_bind_request(fwk_id_t requester_id, fwk_id_t target_id,
                                fwk_id_t api_id, const void **api)
{
    *api = &driver_api;
    return FWK_SUCCESS;
}

const struct fwk_module module_host_console = {
    .name = "Host Console",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = init,
    .process_bind_request = process_bind_request,
};

const struct fwk_module_config config_host_console = { 0 };

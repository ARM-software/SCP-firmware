/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     sgm776 System Support.
 */

#include <mod_power_domain.h>
#include <mod_sgm776_system.h>
#include <mod_system_power.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <fmw_cmsis.h>

/*
 * Functions fulfilling the framework's module interface
 */

static int sgm776_system_shutdown(enum mod_pd_system_shutdown system_shutdown)
{
    NVIC_SystemReset();

    return FWK_E_DEVICE;
}

static const struct mod_system_power_driver_api
    sgm776_system_system_power_driver_api = {
    .system_shutdown = sgm776_system_shutdown
};

/*
 * Functions fulfilling the framework's module interface
 */

static int sgm776_system_init(fwk_id_t module_id, unsigned int unused,
                             const void *unused2)
{
    return FWK_SUCCESS;
}

static int sgm776_system_process_bind_request(fwk_id_t source_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    *api = &sgm776_system_system_power_driver_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_sgm776_system = {
    .name = "sgm776_SYSTEM",
    .api_count = MOD_SGM776_SYSTEM_API_COUNT,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = sgm776_system_init,
    .process_bind_request = sgm776_system_process_bind_request,
};

/* No elements, no module configuration data */
struct fwk_module_config config_sgm776_system = { 0 };

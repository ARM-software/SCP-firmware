/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2018, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SGM775 System Support.
 */

#include <fmw_cmsis.h>
#include <fwk_module.h>
#include <mod_system_power.h>
#include <mod_sgm775_system.h>

/*
 * Functions fulfilling the framework's module interface
 */

static int sgm775_system_shutdown(enum mod_pd_system_shutdown system_shutdown)
{
    NVIC_SystemReset();

    return FWK_E_DEVICE;
}

static const struct mod_system_power_driver_api
    sgm775_system_system_power_driver_api = {
    .system_shutdown = sgm775_system_shutdown
};

/*
 * Functions fulfilling the framework's module interface
 */

static int sgm775_system_init(fwk_id_t module_id, unsigned int unused,
                              const void *unused2)
{
    return FWK_SUCCESS;
}

static int sgm775_system_process_bind_request(fwk_id_t source_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    *api = &sgm775_system_system_power_driver_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_sgm775_system = {
    .name = "SGM775_SYSTEM",
    .api_count = MOD_SGM775_SYSTEM_API_COUNT,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = sgm775_system_init,
    .process_bind_request = sgm775_system_process_bind_request,
};

/* No elements, no module configuration data */
struct fwk_module_config config_sgm775_system = { 0 };

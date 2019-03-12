/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <fwk_assert.h>
#include <fwk_errno.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <mod_juno_system.h>
#include <mod_system_power.h>

/*
 * Functions fulfilling the System Power's API
 */
static int juno_system_shutdown(enum mod_pd_system_shutdown system_shutdown)
{
    return FWK_E_SUPPORT;
}

static const struct mod_system_power_driver_api
    juno_system_system_power_driver_api = {
        .system_shutdown = juno_system_shutdown,
};

/*
 * Framework API
 */
static int juno_system_module_init(fwk_id_t module_id,
                                   unsigned int element_count,
                                   const void *data)
{
    fwk_assert(element_count == 0);

    return FWK_SUCCESS;
}

static int juno_system_process_bind_request(fwk_id_t source_id,
                                            fwk_id_t target_id,
                                            fwk_id_t api_id,
                                            const void **api)
{
    *api = &juno_system_system_power_driver_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_juno_system = {
    .name = "Juno SYSTEM",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = juno_system_module_init,
    .process_bind_request = juno_system_process_bind_request,
};

/* No elements, no module configuration data */
struct fwk_module_config config_juno_system = { 0 };

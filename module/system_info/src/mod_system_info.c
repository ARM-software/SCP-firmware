/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     System Information Hardware Abstraction module.
 */

#include <mod_system_info.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_status.h>

#include <stddef.h>

/* Pointer to the config data for module use. */
static const struct mod_system_info_config *config;

/* Pointer to the data provided by the driver module. */
static struct mod_system_info *system_info;

/* Pointer to the driver function which provides the system info data. */
static struct mod_system_info_get_driver_data_api *get_driver_data;

static int get_system_info(const struct mod_system_info **sys_info)
{
    if (system_info == NULL) {
        system_info = get_driver_data->get_driver_data();
        if (system_info == NULL)
            return FWK_E_SUPPORT;
    }

    *sys_info = system_info;
    return FWK_SUCCESS;
}

/*
 * API to be used by the modules that need a copy of the system ID information
 * data.
 */
static struct mod_system_info_get_info_api get_system_info_api = {
    .get_system_info = get_system_info,
};

/*
 * Framework handlers
 */
static int system_info_init(fwk_id_t module_id, unsigned int element_count,
        const void *data)
{
    fwk_assert(data != NULL);

    /* No elements support */
    if (element_count > 0)
        return FWK_E_DATA;

    config = data;
    return FWK_SUCCESS;
}

static int system_info_bind(fwk_id_t id, unsigned int round)
{
    int status = FWK_SUCCESS;

    if (round == 1) {
        if (!fwk_id_is_equal(config->system_info_driver_module_id,
                             FWK_ID_NONE)) {

            /* If module ID is provided, API ID shouldn't be NONE */
            fwk_assert(!fwk_id_is_equal(config->system_info_driver_data_api_id,
                                        FWK_ID_NONE));

            status = fwk_module_bind(config->system_info_driver_module_id,
                                     config->system_info_driver_data_api_id,
                                     &get_driver_data);
            if (status != FWK_SUCCESS)
                return FWK_E_PANIC;
        }
    }
    return status;
}

static int system_info_process_bind_request(fwk_id_t requester_id,
        fwk_id_t targer_id, fwk_id_t api_id, const void **api)
{
    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_SYSTEM_INFO_GET_API_IDX:
        *api = &get_system_info_api;
        break;
    default:
        return FWK_E_PARAM;
    }
    return FWK_SUCCESS;
}

const struct fwk_module module_system_info = {
    .name = "SYSTEM ID INFORMATION",
    .type = FWK_MODULE_TYPE_HAL,
    .init = system_info_init,
    .bind = system_info_bind,
    .process_bind_request = system_info_process_bind_request,
    .api_count = MOD_SYSTEM_INFO_API_COUNT,
};

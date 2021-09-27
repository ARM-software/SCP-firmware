/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/hsspi_api.h>

#include <mod_hsspi.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_status.h>

static struct mod_hsspi_api module_api = {
    .hsspi_init = HSSPI_init,
    .hsspi_exit = HSSPI_exit,
};

const struct mod_timer_api *hsspi_timer_api;

/*
 * Framework handlers
 */

static int hsspi_controller_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    return FWK_SUCCESS;
}

static int hsspi_controller_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round > 0) {
        return FWK_SUCCESS;
    }

    status = fwk_module_bind(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
        FWK_ID_API(FWK_MODULE_IDX_TIMER, MOD_TIMER_API_IDX_TIMER),
        &hsspi_timer_api);

    return status;
}

static int hsspi_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    return FWK_SUCCESS;
}

static int hsspi_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &module_api;

    return FWK_SUCCESS;
}

static int hsspi_start(fwk_id_t id)
{
    return FWK_SUCCESS;
}

const struct fwk_module module_hsspi = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = hsspi_controller_init,
    .bind = hsspi_controller_bind,
    .element_init = hsspi_element_init,
    .start = hsspi_start,
    .process_bind_request = hsspi_process_bind_request,
};

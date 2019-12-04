/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <i2c_api.h>

#include <mod_f_i2c.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_status.h>

static struct mod_f_i2c_api module_api = {
    .init = i2c_construction,
    .recv_data = f_i2c_api_recv_data,
    .send_data = f_i2c_api_send_data,
};

/*
 * Framework handlers
 */

static int f_i2c_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    return FWK_SUCCESS;
}

static int f_i2c_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    return FWK_SUCCESS;
}

static int f_i2c_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &module_api;

    return FWK_SUCCESS;
}

static int f_i2c_start(fwk_id_t id)
{
    return FWK_SUCCESS;
}

const struct fwk_module module_f_i2c = {
    .name = "f_i2c",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = f_i2c_init,
    .element_init = f_i2c_element_init,
    .start = f_i2c_start,
    .process_bind_request = f_i2c_process_bind_request,
};

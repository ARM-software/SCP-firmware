/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Performance, Monitor and instrumentation HAL implementation.
 *
 */

#include <mod_pmi.h>

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <string.h>

struct mod_pmi_ctx {
    /* Platform specific memory configuration data */
    struct mod_pmi_driver_config *driver_config;
    struct mod_pmi_driver_api *driver_api;
};

static struct mod_pmi_ctx pmi_ctx;

static int start_cycle_count(void)
{
    return pmi_ctx.driver_api->start_cycle_count();
}

static int stop_cycle_count(void)
{
    return pmi_ctx.driver_api->stop_cycle_count();
}

static int get_cycle_count(uint64_t *cycle_count)
{
    return pmi_ctx.driver_api->get_cycle_count(cycle_count);
}

static int set_cycle_count(uint64_t cycle_count)
{
    return pmi_ctx.driver_api->set_cycle_count(cycle_count);
}

static uint64_t cycle_count_diff(uint64_t start, uint64_t end)
{
    return pmi_ctx.driver_api->cycle_count_diff(start, end);
}

static uint64_t get_current_time(void)
{
    return pmi_ctx.driver_api->get_current_time();
}

static const struct mod_pmi_hal_api mod_pmi_api = {
    .start_cycle_count = start_cycle_count,
    .stop_cycle_count = stop_cycle_count,
    .get_cycle_count = get_cycle_count,
    .set_cycle_count = set_cycle_count,
    .cycle_count_diff = cycle_count_diff,
    .get_current_time = get_current_time,
};

static int pmi_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    pmi_ctx.driver_config = (struct mod_pmi_driver_config *)data;

    return FWK_SUCCESS;
}

static int pmi_bind(fwk_id_t id, unsigned int round)
{
    /* Bind to PMI driver */
    return fwk_module_bind(
        pmi_ctx.driver_config->driver_id,
        pmi_ctx.driver_config->driver_api_id,
        &pmi_ctx.driver_api);
}

static int pmi_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    /* Only allow binding to the module */
    if (!fwk_id_is_equal(target_id, fwk_module_id_pmi) ||
        !fwk_id_is_equal(api_id, mod_pmi_api_id_hal)) {
        return FWK_E_PARAM;
    }

    *api = &mod_pmi_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_pmi = {
    .type = FWK_MODULE_TYPE_HAL,
    .init = pmi_init,
    .bind = pmi_bind,
    .process_bind_request = pmi_bind_request,
    .api_count = MOD_PMI_API_IDX_COUNT,
};

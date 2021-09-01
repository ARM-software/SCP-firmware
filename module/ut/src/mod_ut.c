/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Performance, Monitor and instrumentation HAL implementation.
 *
 */

#include <mod_ut.h>

#include <fwk_assert.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <string.h>

extern int plat_execute_all_tests(void);

static int ut_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    plat_execute_all_tests();
    return FWK_SUCCESS;
}

static int ut_bind(fwk_id_t id, unsigned int round)
{
    return FWK_SUCCESS;
}

static int ut_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    return FWK_SUCCESS;
}

const struct fwk_module module_ut = {
    .type = FWK_MODULE_TYPE_HAL,
    .init = ut_init,
    .bind = ut_bind,
    .process_bind_request = ut_bind_request,
    .api_count = 1,
};

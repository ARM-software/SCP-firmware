/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Metrics Analyzer
 */

#include "mod_metrics_analyzer.h"

#include <fwk_module.h>

/*
 * Framework handlers
 */

static int metrics_analyzer_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{
    if (element_count == 0U) {
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int metrics_analyzer_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    int status = FWK_SUCCESS;

    return status;
}

static int metrics_analyzer_bind(fwk_id_t id, unsigned int round)
{
    return FWK_SUCCESS;
}

static int metrics_analyzer_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    return FWK_SUCCESS;
}

static int metrics_analyzer_start(fwk_id_t id)
{
    return FWK_SUCCESS;
}

/* metrics_analyzer module definition */
const struct fwk_module module_metrics_analyzer = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = MOD_METRICS_ANALYZER_API_IDX_COUNT,
    .init = metrics_analyzer_init,
    .element_init = metrics_analyzer_element_init,
    .bind = metrics_analyzer_bind,
    .start = metrics_analyzer_start,
    .process_bind_request = metrics_analyzer_process_bind_request,
};

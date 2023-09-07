/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI power capping and monitoring protocol completer.
 */
#include "fwk_mm.h"
#include "fwk_module_idx.h"
#include "internal/scmi_power_capping_protocol.h"
#include "mod_power_allocator.h"
#include "mod_power_meter.h"

#include <fwk_module.h>

struct mod_scmi_power_capping_power_apis power_management_apis;

static int scmi_power_capping_power_api_bind(
    struct mod_scmi_power_capping_power_apis *power_apis)
{
    int status;

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_ALLOCATOR),
        FWK_ID_API(
            FWK_MODULE_IDX_POWER_ALLOCATOR, MOD_POWER_ALLOCATOR_API_IDX_CAP),
        &(power_apis->power_allocator_api));

    if (status != FWK_SUCCESS) {
        return status;
    }

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_COORDINATOR),
        FWK_ID_API(
            FWK_MODULE_IDX_POWER_COORDINATOR,
            MOD_POWER_COORDINATOR_API_IDX_PERIOD),
        &(power_apis->power_coordinator_api));

    if (status != FWK_SUCCESS) {
        return status;
    }

    return fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_POWER_METER),
        FWK_ID_API(
            FWK_MODULE_IDX_POWER_METER, MOD_POWER_METER_API_IDX_MEASUREMENT),
        &(power_apis->power_meter_api));
}

static int scmi_power_capping_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    if (element_count == 0) {
        return FWK_E_SUPPORT;
    }

    struct mod_scmi_power_capping_context ctx;

    ctx.power_capping_domain_ctx_table = fwk_mm_calloc(
        element_count, sizeof(struct mod_scmi_power_capping_domain_context));
    ctx.domain_count = element_count;
    pcapping_protocol_init(&ctx);

    return FWK_SUCCESS;
}

static int scmi_power_capping_element_init(
    fwk_id_t element_id,
    unsigned int sub_element_count,
    const void *data)
{
    const struct mod_scmi_power_capping_domain_config *config;
    unsigned int domain_idx;
    int status;

    if (data == NULL) {
        return FWK_E_PARAM;
    }

    config = (const struct mod_scmi_power_capping_domain_config *)data;
    domain_idx = fwk_id_get_element_idx(element_id);

    status = pcapping_protocol_domain_init(domain_idx, config);
    if (status != FWK_SUCCESS) {
        return status;
    }

    return FWK_SUCCESS;
}

static int scmi_power_capping_bind(fwk_id_t id, unsigned int round)
{
    int status = FWK_E_INIT;

    if ((round == 1) || (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))) {
        return FWK_SUCCESS;
    }
    status = scmi_power_capping_power_api_bind(&power_management_apis);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = pcapping_protocol_bind();

    if (status != FWK_SUCCESS) {
        return status;
    }
    pcapping_protocol_set_power_apis(&power_management_apis);

    return status;
}

static int scmi_power_capping_start(fwk_id_t id)
{
    return pcapping_protocol_start(id);
}

static int scmi_power_capping_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    return pcapping_protocol_process_notification(event);
}

static int scmi_power_capping_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    if (fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI))) {
        return pcapping_protocol_process_bind_request(api_id, api);
    }

    return FWK_E_ACCESS;
}

const struct fwk_module module_scmi_power_capping = {
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .api_count = (unsigned int)MOD_SCMI_POWER_CAPPING_API_IDX_COUNT,
    .init = scmi_power_capping_init,
    .element_init = scmi_power_capping_element_init,
    .bind = scmi_power_capping_bind,
    .start = scmi_power_capping_start,
    .process_notification = scmi_power_capping_process_notification,
    .process_bind_request = scmi_power_capping_process_bind_request,
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "synquacer_ddr.h"

#include <ddr_init.h>

#include <internal/scmi.h>
#include <internal/scmi_vendor_ext.h>

#include <mod_scmi.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

struct scmi_vendor_ext_ctx {
    const struct mod_scmi_from_protocol_api *scmi_api;
    const struct mod_vendor_ext_api *vendor_ext_api;
    uint32_t vendor_ext_count;
};

static int scmi_vendor_ext_protocol_version_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_vendor_ext_protocol_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_vendor_ext_protocol_msg_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_vendor_ext_protocol_memory_info_get_handler(
    fwk_id_t service_id,
    const uint32_t *payload);

/*
 * Internal variables.
 */
static struct scmi_vendor_ext_ctx scmi_vendor_ext_ctx;

static int (*handler_table[])(fwk_id_t, const uint32_t *) = {
    [SCMI_PROTOCOL_VERSION] = scmi_vendor_ext_protocol_version_handler,
    [SCMI_PROTOCOL_ATTRIBUTES] = scmi_vendor_ext_protocol_attributes_handler,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        scmi_vendor_ext_protocol_msg_attributes_handler,
    [SCMI_VENDOR_EXT_MEMORY_INFO_GET] =
        scmi_vendor_ext_protocol_memory_info_get_handler,
};

static unsigned int payload_size_table[] = {
    [SCMI_PROTOCOL_VERSION] = 0,
    [SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        sizeof(struct scmi_protocol_message_attributes_a2p),
    [SCMI_VENDOR_EXT_MEMORY_INFO_GET] = 0,
};

/*
 * vendor_ext management protocol implementation
 */
static int scmi_vendor_ext_protocol_version_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_VENDOR_EXT,
    };

    scmi_vendor_ext_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));

    return FWK_SUCCESS;
}

static int scmi_vendor_ext_protocol_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_vendor_ext_protocol_attributes_p2a return_values = {
        .status = SCMI_SUCCESS,
        .attributes = scmi_vendor_ext_ctx.vendor_ext_count,
    };

    scmi_vendor_ext_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));

    return FWK_SUCCESS;
}

static int scmi_vendor_ext_protocol_msg_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_protocol_message_attributes_a2p *parameters;
    struct scmi_protocol_message_attributes_p2a return_values;

    parameters = (const struct scmi_protocol_message_attributes_a2p *)payload;

    if ((parameters->message_id < FWK_ARRAY_SIZE(handler_table)) &&
        (handler_table[parameters->message_id] != NULL)) {
        return_values = (struct scmi_protocol_message_attributes_p2a) {
            .status = SCMI_SUCCESS,
            /* All commands have an attributes value of 0 */
            .attributes = 0,
        };
    } else
        return_values.status = SCMI_NOT_FOUND;

    scmi_vendor_ext_ctx.scmi_api->respond(
        service_id,
        &return_values,
        (return_values.status == SCMI_SUCCESS) ? sizeof(return_values) :
                                                 sizeof(return_values.status));

    return FWK_SUCCESS;
}

static const uint64_t ddr_memorymap_start_addr[] = { DRAM_AREA_1_START_ADDR,
                                                     DRAM_AREA_2_START_ADDR,
                                                     DRAM_AREA_3_START_ADDR };

static const uint64_t ddr_memorymap_size[] = { DRAM_AREA_1_SIZE,
                                               DRAM_AREA_2_SIZE,
                                               DRAM_AREA_3_SIZE };

static void fill_available_dram_region(
    uint32_t *memory_array_count_num,
    uint64_t *area_start,
    uint64_t *area_size)
{
    uint32_t i;
    uint64_t dram_size;
    uint64_t allocated_size;

    dram_size = (uint64_t)fw_get_ddr4_sdram_total_size() * 1024 * 1024;

    for (i = 0; (i < SCMI_MEMORY_INFO_ARRAY_NUM_MAX) && (dram_size > 0); i++) {
        area_start[i] = ddr_memorymap_start_addr[i];
        allocated_size = dram_size > ddr_memorymap_size[i] ?
            ddr_memorymap_size[i] :
            dram_size;
        area_size[i] = allocated_size;

        dram_size -= allocated_size;
    }

    *memory_array_count_num = i;
}

static int get_memory_info(struct synquacer_memory_info *meminfo)
{
    uint32_t i;
    uint32_t memory_array_count_num = 0;
    uint64_t area_start[SCMI_MEMORY_INFO_ARRAY_NUM_MAX] = { 0 };
    uint64_t area_size[SCMI_MEMORY_INFO_ARRAY_NUM_MAX] = { 0 };

    fill_available_dram_region(&memory_array_count_num, area_start, area_size);

    if (ddr_is_secure_dram_enabled())
        area_size[0] -= DRAM_RESERVED_FOR_SECURE_APP_SIZE;

    for (i = 0, meminfo->array_num = 0; i < memory_array_count_num; i++) {
        meminfo->memory_info_array[i].start_offset_low =
            (uint32_t)(area_start[i] & 0xFFFFFFFFU);

        meminfo->memory_info_array[i].start_offset_high =
            (uint32_t)(area_start[i] >> 32);

        meminfo->memory_info_array[i].size_low =
            (uint32_t)(area_size[i] & 0xFFFFFFFFU);

        meminfo->memory_info_array[i].size_high =
            (uint32_t)(area_size[i] >> 32);

        meminfo->array_num++;
    }

    return FWK_SUCCESS;
}

static struct scmi_vendor_ext_memory_info_get_resp resp;

static int scmi_vendor_ext_protocol_memory_info_get_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    memset(&resp, 0, sizeof(struct scmi_vendor_ext_memory_info_get_resp));

    FWK_LOG_INFO("[scmi_vendor_ext] memory info get handler.");

    get_memory_info(&resp.meminfo);

    scmi_vendor_ext_ctx.scmi_api->respond(service_id, &resp, sizeof(resp));

    return FWK_SUCCESS;
}

/*
 * SCMI module -> SCMI vendor_ext module interface
 */
static int scmi_vendor_ext_get_scmi_protocol_id(
    fwk_id_t protocol_id,
    uint8_t *scmi_protocol_id)
{

    *scmi_protocol_id = SCMI_PROTOCOL_ID_VENDOR_EXT;

    return FWK_SUCCESS;
}

static int scmi_vendor_ext_message_handler(
    fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    int32_t return_value;

    FWK_LOG_INFO("[scmi_vendor_ext] message handler.");

    static_assert(
        FWK_ARRAY_SIZE(handler_table) == FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] vendor_ext management protocol table sizes not consistent");
    assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(handler_table)) {
        return_value = SCMI_NOT_SUPPORTED;
        goto error;
    }

    if (payload_size != payload_size_table[message_id]) {
        return_value = SCMI_PROTOCOL_ERROR;
        goto error;
    }

    return handler_table[message_id](service_id, payload);

error:
    scmi_vendor_ext_ctx.scmi_api->respond(
        service_id, &return_value, sizeof(return_value));
    return FWK_SUCCESS;
}

static struct mod_scmi_to_protocol_api
    scmi_vendor_ext_mod_scmi_to_protocol_api = {
        .get_scmi_protocol_id = scmi_vendor_ext_get_scmi_protocol_id,
        .message_handler = scmi_vendor_ext_message_handler
    };

/*
 * Framework interface
 */
static int scmi_vendor_ext_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{
    if (element_count != 0) {
        /* This module should not have any elements */
        assert(false);
        return FWK_E_SUPPORT;
    }

    return FWK_SUCCESS;
}

static int scmi_vendor_ext_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 1)
        return FWK_SUCCESS;

    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_vendor_ext_ctx.scmi_api);
    if (status != FWK_SUCCESS) {
        /* Failed to bind to SCMI module */
        assert(false);
        return status;
    }

    return FWK_SUCCESS;
}

static int scmi_vendor_ext_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    if (!fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI)))
        return FWK_E_ACCESS;

    FWK_LOG_INFO("[scmi_vendor_ext] process bind request.");

    *api = &scmi_vendor_ext_mod_scmi_to_protocol_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_scmi_vendor_ext = {
    .name = "SCMI vendor_ext management",
    .api_count = 1,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_vendor_ext_init,
    .bind = scmi_vendor_ext_bind,
    .process_bind_request = scmi_vendor_ext_process_bind_request,
};

/* No elements, no module configuration data */
struct fwk_module_config config_scmi_vendor_ext = { 0 };

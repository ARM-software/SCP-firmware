/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI Management Protocol Support.
 */

#include "n1sdp_scc_reg.h"
#include "n1sdp_scp_pik.h"

#include <internal/scmi.h>

#include <mod_scmi.h>
#include <mod_scmi_management.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>

struct scmi_management_ctx {
    /* SCMI module API */
    const struct mod_scmi_from_protocol_api *scmi_api;
};

static int scmi_management_protocol_version_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_management_protocol_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_management_protocol_message_attributes_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_management_clock_status_get_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_management_chipid_info_get_handler(fwk_id_t service_id,
    const uint32_t *payload);

/*
 * Internal variables.
 */
static struct scmi_management_ctx scmi_management_ctx;

static int (* const handler_table[])(fwk_id_t, const uint32_t *) = {
    [SCMI_PROTOCOL_VERSION] = scmi_management_protocol_version_handler,
    [SCMI_PROTOCOL_ATTRIBUTES] = scmi_management_protocol_attributes_handler,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        scmi_management_protocol_message_attributes_handler,
    [SCMI_MANAGEMENT_CLOCK_STATUS_GET] =
        scmi_management_clock_status_get_handler,
    [SCMI_MANAGEMENT_CHIPID_INFO_GET] =
        scmi_management_chipid_info_get_handler,
};

static const unsigned int payload_size_table[] = {
    [SCMI_PROTOCOL_VERSION] = 0,
    [SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] = 0,
    [SCMI_MANAGEMENT_CLOCK_STATUS_GET] = 0,
    [SCMI_MANAGEMENT_CHIPID_INFO_GET] = 0,
};

/*
 * Protocol Version
 */
static int scmi_management_protocol_version_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_MANAGEMENT,
    };

    scmi_management_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
    return FWK_SUCCESS;
}

/*
 * Protocol Attributes
 */
static int scmi_management_protocol_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_attributes_p2a return_values = {
        .status = SCMI_SUCCESS,
        .attributes = 0,
    };

    scmi_management_ctx.scmi_api->respond(
        service_id,
        &return_values,
        sizeof(return_values));

    return FWK_SUCCESS;
}

/*
 * Protocol Message Attributes
 */
static int scmi_management_protocol_message_attributes_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    size_t response_size;
    const struct scmi_protocol_message_attributes_a2p *parameters;
    unsigned int message_id;
    struct scmi_protocol_message_attributes_p2a return_values = {
        .status = SCMI_SUCCESS,
        .attributes = 0,
    };

    parameters = (const struct scmi_protocol_message_attributes_a2p *)
        payload;
    message_id = parameters->message_id;

    if ((message_id >= FWK_ARRAY_SIZE(handler_table)) ||
        (handler_table[message_id] == NULL))
        return_values.status = SCMI_NOT_FOUND;

    response_size = (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status);

    scmi_management_ctx.scmi_api->respond(
        service_id, &return_values, response_size);

    return FWK_SUCCESS;
}

/*
 * Clock Status Get
 */
static int scmi_management_clock_status_get_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status;
    unsigned int agent_id;
    enum scmi_agent_type agent_type;
    struct scmi_management_clock_status_get_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };

    status = scmi_management_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    status = scmi_management_ctx.scmi_api->get_agent_type(agent_id,
                                                          &agent_type);
    if (status != FWK_SUCCESS)
        goto exit;

    /* Only the Management agent may get the clock status */
    if (agent_type != SCMI_AGENT_TYPE_MANAGEMENT) {
        return_values.status = SCMI_DENIED;
        status = FWK_E_ACCESS;
        goto exit;
    }

    return_values.status = SCMI_SUCCESS;
    return_values.clock_status = 1;

exit:
    scmi_management_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
    return status;
}

/*
 * CHIP ID Information Get
 */
static int scmi_management_chipid_info_get_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status;
    uint32_t chip_info;
    unsigned int agent_id;
    enum scmi_agent_type agent_type;
    struct scmi_management_chipid_info_get_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };

    status = scmi_management_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    status = scmi_management_ctx.scmi_api->get_agent_type(agent_id,
                                                          &agent_type);
    if (status != FWK_SUCCESS)
        goto exit;

    /* Only the Management agent may get the CHIP ID information */
    if (agent_type != SCMI_AGENT_TYPE_MANAGEMENT) {
        return_values.status = SCMI_DENIED;
        status = FWK_E_ACCESS;
        goto exit;
    }

    chip_info = SCC->PLATFORM_CTRL;
    return_values.status = SCMI_SUCCESS;
    return_values.multi_chip_mode =
        ((chip_info & SCC_PLATFORM_CTRL_MULTI_CHIP_MODE_MASK) >>
         SCC_PLATFORM_CTRL_MULTI_CHIP_MODE_POS);
    return_values.chipid =
        ((chip_info & SCC_PLATFORM_CTRL_CHIPID_MASK) >>
         SCC_PLATFORM_CTRL_CHIPID_POS);

exit:
    scmi_management_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
    return status;
}

/*
 * SCMI module -> SCMI Management module interface
 */
static int scmi_management_get_scmi_protocol_id(fwk_id_t protocol_id,
    uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = SCMI_PROTOCOL_ID_MANAGEMENT;

    return FWK_SUCCESS;
}

static int scmi_management_message_handler(
    fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    int32_t return_value;

    static_assert(FWK_ARRAY_SIZE(handler_table) ==
        FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] Management protocol table sizes not consistent");
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
    scmi_management_ctx.scmi_api->respond(
        service_id,
        &return_value,
        sizeof(return_value));

    return FWK_SUCCESS;
}

static struct mod_scmi_to_protocol_api
    scmi_management_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = scmi_management_get_scmi_protocol_id,
    .message_handler = scmi_management_message_handler
};

/*
 * Framework handlers
 */

static int scmi_management_init(fwk_id_t module_id, unsigned int unused,
                           const void *data)
{
    return FWK_SUCCESS;
}

static int scmi_management_bind(fwk_id_t id, unsigned int round)
{
    if (round == 1)
        return FWK_SUCCESS;

    /* Bind to the SCMI module, storing an API pointer for later use. */
    return fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_management_ctx.scmi_api);
}

static int scmi_management_process_bind_request(fwk_id_t source_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    /* Only accept binding requests from the SCMI module. */
    if (!fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI)))
        return FWK_E_ACCESS;

    *api = &scmi_management_mod_scmi_to_protocol_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_scmi_management = {
    .name = "SCMI Management Protocol",
    .api_count = 1,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_management_init,
    .bind = scmi_management_bind,
    .process_bind_request = scmi_management_process_bind_request,
};

/* No elements, no module configuration data */
struct fwk_module_config config_scmi_management = { 0 };

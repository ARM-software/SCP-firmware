/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     System Control and Management Interface (SCMI) support.
 */

#include <internal/mod_scmi.h>
#include <internal/scmi.h>
#include <internal/scmi_base.h>

#include <mod_scmi.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#ifdef BUILD_HAS_MULTITHREADING
#    include <fwk_multi_thread.h>
#endif

#include <inttypes.h>
#include <string.h>

struct scmi_protocol {
    /* SCMI protocol message handler */
    mod_scmi_message_handler_t *message_handler;

    /* SCMI protocol framework identifier */
    fwk_id_t id;
};

struct scmi_ctx {
    /* SCMI module configuration data */
    struct mod_scmi_config *config;

    /* Table of bound protocols */
    struct scmi_protocol *protocol_table;

    /* Number of bound protocols */
    unsigned int protocol_count;

    /*
     * SCMI protocol identifier to the index of the entry in protocol_table[]
     * dedicated to the protocol.
     */
    uint8_t scmi_protocol_id_to_idx[SCMI_PROTOCOL_ID_MAX + 1];

    /* Table of service contexts */
    struct scmi_service_ctx *service_ctx_table;
};

/*
 * Entry zero (0) of the protocol table 'protocol_table' is not used, as index
 * 0 is the index of the unused entries of the 'scmi_protocol_id_to_idx[]'
 * table. Entry one (1) is reserved for the base protocol implemented in this
 * file.
 */
#define PROTOCOL_TABLE_BASE_PROTOCOL_IDX 1
#define PROTOCOL_TABLE_RESERVED_ENTRIES_COUNT 2

static int scmi_base_protocol_version_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_base_protocol_attributes_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_base_protocol_message_attributes_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_base_discover_vendor_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_base_discover_sub_vendor_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_base_discover_implementation_version_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_base_discover_list_protocols_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_base_discover_agent_handler(
    fwk_id_t service_id, const uint32_t *payload);

static int (* const base_handler_table[])(fwk_id_t, const uint32_t *) = {
    [SCMI_PROTOCOL_VERSION] =
        scmi_base_protocol_version_handler,
    [SCMI_PROTOCOL_ATTRIBUTES] =
        scmi_base_protocol_attributes_handler,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        scmi_base_protocol_message_attributes_handler,
    [SCMI_BASE_DISCOVER_VENDOR] =
        scmi_base_discover_vendor_handler,
    [SCMI_BASE_DISCOVER_SUB_VENDOR] =
        scmi_base_discover_sub_vendor_handler,
    [SCMI_BASE_DISCOVER_IMPLEMENTATION_VERSION] =
        scmi_base_discover_implementation_version_handler,
    [SCMI_BASE_DISCOVER_LIST_PROTOCOLS] =
        scmi_base_discover_list_protocols_handler,
    [SCMI_BASE_DISCOVER_AGENT] =
        scmi_base_discover_agent_handler,
};

static const unsigned int base_payload_size_table[] = {
    [SCMI_PROTOCOL_VERSION] = 0,
    [SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
                        sizeof(struct scmi_protocol_message_attributes_a2p),
    [SCMI_BASE_DISCOVER_VENDOR] = 0,
    [SCMI_BASE_DISCOVER_SUB_VENDOR] = 0,
    [SCMI_BASE_DISCOVER_IMPLEMENTATION_VERSION] = 0,
    [SCMI_BASE_DISCOVER_LIST_PROTOCOLS] =
                        sizeof(struct scmi_base_discover_list_protocols_a2p),
    [SCMI_BASE_DISCOVER_AGENT] =
                        sizeof(struct scmi_base_discover_agent_a2p),
};

static const char * const default_agent_names[] = {
    [SCMI_AGENT_TYPE_PSCI] = "PSCI",
    [SCMI_AGENT_TYPE_MANAGEMENT] = "MANAGEMENT",
    [SCMI_AGENT_TYPE_OSPM] = "OSPM",
    [SCMI_AGENT_TYPE_OTHER] = "OTHER",
};

static struct scmi_ctx scmi_ctx;

/*
 * Utility functions
 */
static uint16_t read_message_id(uint32_t message_header)
{
    return (message_header & SCMI_MESSAGE_HEADER_MESSAGE_ID_MASK) >>
        SCMI_MESSAGE_HEADER_MESSAGE_ID_POS;
}

static uint8_t read_protocol_id(uint32_t message_header)
{
    return (message_header & SCMI_MESSAGE_HEADER_PROTOCOL_ID_MASK) >>
        SCMI_MESSAGE_HEADER_PROTOCOL_ID_POS;
}

static uint16_t read_token(uint32_t message_header)
{
    return (message_header & SCMI_MESSAGE_HEADER_TOKEN_MASK) >>
        SCMI_MESSAGE_HEADER_TOKEN_POS;
}

/*
 * Transport entity -> SCMI module
 */
static int signal_error(fwk_id_t service_id)
{
    fwk_id_t transport_id;
    struct scmi_service_ctx *ctx;

    ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(service_id)];
    transport_id = ctx->transport_id;

    return ctx->respond(transport_id, &(int32_t){SCMI_PROTOCOL_ERROR},
                        sizeof(int32_t));
}

static int signal_message(fwk_id_t service_id)
{
    struct fwk_event event = (struct fwk_event) {
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_SCMI, 0),
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        .target_id = service_id,
    };

    return fwk_thread_put_event(&event);
}

static const struct mod_scmi_from_transport_api mod_scmi_from_transport_api = {
    .signal_error = signal_error,
    .signal_message = signal_message,
};

/*
 * SCMI protocol module -> SCMI module interface
 */

static int get_agent_id(fwk_id_t service_id, unsigned int *agent_id)
{
    struct scmi_service_ctx *ctx;

    if (agent_id == NULL)
        return FWK_E_PARAM;

    ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(service_id)];

    *agent_id = ctx->config->scmi_agent_id;

    return FWK_SUCCESS;
}

static int get_agent_type(uint32_t scmi_agent_id,
                          enum scmi_agent_type *agent_type)
{
    if ((agent_type == NULL) ||
        (scmi_agent_id > scmi_ctx.config->agent_count) ||
        (scmi_agent_id == SCMI_PLATFORM_ID))
        return FWK_E_PARAM;

    *agent_type = scmi_ctx.config->agent_table[scmi_agent_id].type;

    return FWK_SUCCESS;
}

static int get_max_payload_size(fwk_id_t service_id, size_t *size)
{
    struct scmi_service_ctx *ctx;

    if (size == NULL)
        return FWK_E_PARAM;

    ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(service_id)];

    return ctx->transport_api->get_max_payload_size(ctx->transport_id, size);
}

static int write_payload(fwk_id_t service_id, size_t offset,
                         const void *payload, size_t size)
{
    const struct scmi_service_ctx *ctx;

    ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(service_id)];

    return ctx->transport_api->write_payload(ctx->transport_id,
                                             offset, payload, size);
}

static void respond(fwk_id_t service_id, const void *payload, size_t size)
{
    int status;
    const struct scmi_service_ctx *ctx;
    const char *service_name;

    ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(service_id)];

    service_name = fwk_module_get_name(service_id);

    /*
     * Print to the error log if the message was not successfully processed.
     * We assume here that the first payload entry of the command response
     * holds an SCMI status code. This is the case for all the SCMI commands
     * specified so far, but it is not explicitly stated (yet) in the
     * specification it should be like that for all commands.
     */
    if ((payload != NULL) && (*((int32_t *)payload) < SCMI_SUCCESS)) {
        FWK_LOG_ERR(
            "[SCMI] %s: Message [%" PRIu16
            " (0x%x:0x%x)] returned with an error (%d)",
            service_name,
            ctx->scmi_token,
            ctx->scmi_protocol_id,
            ctx->scmi_message_id,
            *((int *)payload));
    } else {
        FWK_LOG_TRACE(
            "[SCMI] %s: Message [%" PRIu16
            " (0x%x:0x%x)] returned successfully",
            service_name,
            ctx->scmi_token,
            ctx->scmi_protocol_id,
            ctx->scmi_message_id);
    }

    status = ctx->respond(ctx->transport_id, payload, size);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            "[SCMI] %s: Message [%" PRIu16
            " (0x%x:0x%x)] failed to respond (%s)",
            service_name,
            ctx->scmi_token,
            ctx->scmi_protocol_id,
            ctx->scmi_message_id,
            fwk_status_str(status));
    }
}

static const struct mod_scmi_from_protocol_api mod_scmi_from_protocol_api = {
    .get_agent_id = get_agent_id,
    .get_agent_type = get_agent_type,
    .get_max_payload_size = get_max_payload_size,
    .write_payload = write_payload,
    .respond = respond,
};

/*
 * Base protocol implementation
 */
/*
 * Base Protocol - PROTOCOL_VERSION
 */
static int scmi_base_protocol_version_handler(fwk_id_t service_id,
                                              const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_BASE,
    };

    respond(service_id, &return_values, sizeof(return_values));

    return FWK_SUCCESS;
}

/*
 * Base Protocol - PROTOCOL_ATTRIBUTES
 */
static int scmi_base_protocol_attributes_handler(fwk_id_t service_id,
                                                 const uint32_t *payload)
{
    struct scmi_protocol_attributes_p2a return_values = {
        .status = SCMI_SUCCESS,
    };

    return_values.attributes =
        SCMI_BASE_PROTOCOL_ATTRIBUTES(scmi_ctx.protocol_count,
                                      scmi_ctx.config->agent_count);

    respond(service_id, &return_values, sizeof(return_values));

    return FWK_SUCCESS;
}

/*
 * Base Protocol - PROTOCOL_MESSAGE_ATTRIBUTES
 */
static int scmi_base_protocol_message_attributes_handler(fwk_id_t service_id,
                                                       const uint32_t *payload)
{
    struct scmi_protocol_message_attributes_a2p *parameters;
    unsigned int message_id;
    struct scmi_protocol_message_attributes_p2a return_values = {
        .status = SCMI_NOT_FOUND,
    };

    parameters = (struct scmi_protocol_message_attributes_a2p *)payload;
    message_id = parameters->message_id;

    if ((message_id < FWK_ARRAY_SIZE(base_handler_table)) &&
        (base_handler_table[message_id] != NULL))
        return_values.status = SCMI_SUCCESS;

    /* For this protocol, all commands have an attributes value of 0, which
     * has already been set by the initialization of "return_values".
     */

    respond(service_id, &return_values,
            (return_values.status == SCMI_SUCCESS) ?
            sizeof(return_values) : sizeof(return_values.status));

    return FWK_SUCCESS;
}

/*
 * Base Protocol - BASE_DISCOVER_VENDOR
 */
static int scmi_base_discover_vendor_handler(fwk_id_t service_id,
                                             const uint32_t *payload)
{
    struct scmi_base_discover_vendor_p2a return_values = {
        .status = SCMI_SUCCESS,
    };

    if (scmi_ctx.config->vendor_identifier != NULL)
        strncpy(return_values.vendor_identifier,
                scmi_ctx.config->vendor_identifier,
                sizeof(return_values.vendor_identifier) - 1);

    respond(service_id, &return_values, sizeof(return_values));

    return FWK_SUCCESS;
}

/*
 * BASE_DISCOVER_SUB_VENDOR
 */
static int scmi_base_discover_sub_vendor_handler(fwk_id_t service_id,
                                                 const uint32_t *payload)
{
    struct scmi_base_discover_sub_vendor_p2a return_values = {
        .status = SCMI_SUCCESS,
    };

    if (scmi_ctx.config->sub_vendor_identifier != NULL)
        strncpy(return_values.sub_vendor_identifier,
                scmi_ctx.config->sub_vendor_identifier,
                sizeof(return_values.sub_vendor_identifier) - 1);

    respond(service_id, &return_values, sizeof(return_values));

    return FWK_SUCCESS;
}

/*
 * BASE_DISCOVER_IMPLEMENTATION_VERSION
 */
static int scmi_base_discover_implementation_version_handler(
    fwk_id_t service_id, const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = SCMI_SUCCESS,
        .version = FWK_BUILD_VERSION
    };

    respond(service_id, &return_values, sizeof(return_values));

    return FWK_SUCCESS;
}

/*
 * BASE_DISCOVER_LIST_PROTOCOLS
 */
static int scmi_base_discover_list_protocols_handler(fwk_id_t service_id,
                                                     const uint32_t *payload)
{
    int status;
    const struct scmi_base_discover_list_protocols_a2p *parameters;
    struct scmi_base_discover_list_protocols_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
        .num_protocols = 0,
    };
    unsigned int skip;
    size_t max_payload_size;
    size_t payload_size;
    size_t entry_count;
    size_t protocol_count, protocol_count_max;
    unsigned int index;
    uint8_t protocol_id;

    status = get_max_payload_size(service_id, &max_payload_size);
    if (status != FWK_SUCCESS)
        goto error;

    if (max_payload_size <
        (sizeof(struct scmi_base_discover_list_protocols_p2a)
         + sizeof(return_values.protocols[0]))) {
        status = FWK_E_SIZE;
        goto error;
    }

    entry_count = max_payload_size -
                  sizeof(struct scmi_base_discover_list_protocols_p2a);

    parameters = (const struct scmi_base_discover_list_protocols_a2p *)payload;
    skip = parameters->skip;

    if (skip > scmi_ctx.protocol_count) {
        return_values.status = SCMI_INVALID_PARAMETERS;
        goto error;
    }

    protocol_count_max = (scmi_ctx.protocol_count < (skip + entry_count)) ?
                         scmi_ctx.protocol_count : (skip + entry_count);

    for (index = 0, protocol_count = 0,
         payload_size = sizeof(struct scmi_base_discover_list_protocols_p2a);
         (index < FWK_ARRAY_SIZE(scmi_ctx.scmi_protocol_id_to_idx)) &&
         (protocol_count < protocol_count_max);
         index++) {
        if ((scmi_ctx.scmi_protocol_id_to_idx[index] == 0) ||
            (index == SCMI_PROTOCOL_ID_BASE))
            continue;

        protocol_count++;
        if (protocol_count <= skip)
            continue;

        protocol_id = index;
        status = write_payload(service_id, payload_size, &protocol_id,
                               sizeof(protocol_id));
        if (status != FWK_SUCCESS)
            goto error;
        payload_size += sizeof(protocol_id);
    }

    return_values.status = SCMI_SUCCESS;
    return_values.num_protocols = protocol_count_max - skip;

    status = write_payload(service_id, 0,
                           &return_values, sizeof(return_values));
    if (status != FWK_SUCCESS)
        goto error;

    payload_size = FWK_ALIGN_NEXT(payload_size, sizeof(uint32_t));

    respond(service_id, NULL, payload_size);

    return status;

error:
    respond(service_id, &return_values,
            (return_values.status == SCMI_SUCCESS) ?
            sizeof(return_values) : sizeof(return_values.status));

    return status;
}

/*
 * BASE_DISCOVER_AGENT
 */
static int scmi_base_discover_agent_handler(fwk_id_t service_id,
                                            const uint32_t *payload)
{
    const struct scmi_base_discover_agent_a2p *parameters;
    struct scmi_base_discover_agent_p2a return_values = {
        .status = SCMI_NOT_FOUND,
    };
    const struct mod_scmi_agent *agent;

    parameters = (const struct scmi_base_discover_agent_a2p *)payload;

    if (parameters->agent_id > scmi_ctx.config->agent_count)
        goto exit;

    return_values.status = SCMI_SUCCESS;

    if (parameters->agent_id == SCMI_PLATFORM_ID) {
       static const char name[] = "platform";

       static_assert(sizeof(return_values.name) >= sizeof(name),
              "return_values.name is not large enough to contain name");

       memcpy(return_values.name, name, sizeof(name));
       goto exit;
    }

    agent = &scmi_ctx.config->agent_table[parameters->agent_id];

    strncpy(return_values.name,
            (agent->name != NULL) ? agent->name :
            default_agent_names[agent->type],
            sizeof(return_values.name) - 1);

exit:
    respond(service_id, &return_values,
            (return_values.status == SCMI_SUCCESS) ?
            sizeof(return_values) : sizeof(return_values.status));

    return FWK_SUCCESS;
}

static int scmi_base_message_handler(fwk_id_t protocol_id, fwk_id_t service_id,
    const uint32_t *payload, size_t payload_size, unsigned int message_id)
{
    int32_t return_value;

    static_assert(FWK_ARRAY_SIZE(base_handler_table) ==
                  FWK_ARRAY_SIZE(base_payload_size_table),
                  "[SCMI] Base protocol table sizes not consistent");
    assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(base_handler_table)) {
        return_value = SCMI_NOT_SUPPORTED;
        goto error;
    }

    if (payload_size != base_payload_size_table[message_id]) {
        return_value = SCMI_PROTOCOL_ERROR;
        goto error;
    }

    return base_handler_table[message_id](service_id, payload);

error:
    respond(service_id, &return_value, sizeof(return_value));

    return FWK_SUCCESS;
}

/*
 * Framework handlers
 */

static int scmi_init(fwk_id_t module_id, unsigned int service_count,
                     const void *data)
{
    struct mod_scmi_config *config = (struct mod_scmi_config *)data;
    unsigned int agent_idx;
    const struct mod_scmi_agent *agent;

    if (config == NULL)
        return FWK_E_PARAM;

    if ((config->agent_count == 0) ||
        (config->agent_count > SCMI_AGENT_ID_MAX))
        return FWK_E_PARAM;

    /*
     * Loop over the agent descriptors. The SCMI_PLATFORM_ID(0) entry of
     * the table - that would refer to the platform - is ignored.
     */
    for (agent_idx = SCMI_PLATFORM_ID + 1;
         agent_idx <= config->agent_count; agent_idx++) {
        agent = &config->agent_table[agent_idx];
        if (agent->type >= SCMI_AGENT_TYPE_COUNT)
            return FWK_E_PARAM;
    }

    scmi_ctx.protocol_table = fwk_mm_calloc(
        config->protocol_count_max + PROTOCOL_TABLE_RESERVED_ENTRIES_COUNT,
        sizeof(scmi_ctx.protocol_table[0]));

    scmi_ctx.service_ctx_table = fwk_mm_calloc(
        service_count, sizeof(scmi_ctx.service_ctx_table[0]));

    scmi_ctx.protocol_table[PROTOCOL_TABLE_BASE_PROTOCOL_IDX].message_handler =
        scmi_base_message_handler;
    scmi_ctx.scmi_protocol_id_to_idx[SCMI_PROTOCOL_ID_BASE] =
        PROTOCOL_TABLE_BASE_PROTOCOL_IDX;

    scmi_ctx.config = config;

    return FWK_SUCCESS;
}

static int scmi_service_init(fwk_id_t service_id, unsigned int unused,
                             const void *data)
{
    const struct mod_scmi_service_config *config =
        (struct mod_scmi_service_config *)data;
    struct scmi_service_ctx *ctx;

    if ((config->scmi_agent_id == SCMI_PLATFORM_ID) ||
        (config->scmi_agent_id > scmi_ctx.config->agent_count))
        return FWK_E_PARAM;

    ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(service_id)];
    ctx->config = config;

#ifdef BUILD_HAS_MULTITHREADING
    return fwk_thread_create(service_id);
#else
    return FWK_SUCCESS;
#endif
}

static int scmi_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct scmi_service_ctx *ctx;
    const struct mod_scmi_to_transport_api *transport_api = NULL;
    unsigned int protocol_idx;
    struct scmi_protocol *protocol;
    struct mod_scmi_to_protocol_api *protocol_api = NULL;
    uint8_t scmi_protocol_id;

    if (round == 0) {
        if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
            return FWK_SUCCESS;

        ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(id)];
        status = fwk_module_bind(ctx->config->transport_id,
                                 ctx->config->transport_api_id, &transport_api);
        if (status != FWK_SUCCESS)
            return status;

        if ((transport_api->get_secure == NULL) ||
            (transport_api->get_max_payload_size == NULL) ||
            (transport_api->get_message_header == NULL) ||
            (transport_api->get_payload == NULL) ||
            (transport_api->write_payload == NULL) ||
            (transport_api->respond == NULL))
            return FWK_E_DATA;

        ctx->transport_api = transport_api;
        ctx->transport_id = ctx->config->transport_id;
        ctx->respond = transport_api->respond;

        return FWK_SUCCESS;
    }

    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return FWK_SUCCESS;

    for (protocol_idx = 0;
         protocol_idx < scmi_ctx.protocol_count; protocol_idx++) {
        protocol = &scmi_ctx.protocol_table[
            PROTOCOL_TABLE_RESERVED_ENTRIES_COUNT + protocol_idx];

        status = fwk_module_bind(protocol->id,
            FWK_ID_API(fwk_id_get_module_idx(protocol->id), 0), &protocol_api);
        if (status != FWK_SUCCESS)
            return status;

        if ((protocol_api->get_scmi_protocol_id == NULL) ||
            (protocol_api->message_handler == NULL))
            return FWK_E_DATA;
        status = protocol_api->get_scmi_protocol_id(protocol->id,
                                                    &scmi_protocol_id);
        if (status != FWK_SUCCESS)
            return status;

        if (scmi_ctx.scmi_protocol_id_to_idx[scmi_protocol_id] != 0)
            return FWK_E_STATE;

        scmi_ctx.scmi_protocol_id_to_idx[scmi_protocol_id] =
            protocol_idx + PROTOCOL_TABLE_RESERVED_ENTRIES_COUNT;
        protocol->message_handler = protocol_api->message_handler;
    }

    return FWK_SUCCESS;
}

static int scmi_process_bind_request(fwk_id_t source_id, fwk_id_t target_id,
                                     fwk_id_t api_id, const void **api)
{
    unsigned int api_idx;
    struct scmi_service_ctx *ctx;

    api_idx = fwk_id_get_api_idx(api_id);

    switch (api_idx) {
    case MOD_SCMI_API_IDX_PROTOCOL:
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_MODULE))
            return FWK_E_SUPPORT;

        if (scmi_ctx.protocol_count >= scmi_ctx.config->protocol_count_max)
            return FWK_E_NOMEM;

        scmi_ctx.protocol_table[PROTOCOL_TABLE_RESERVED_ENTRIES_COUNT +
                                scmi_ctx.protocol_count++].id = source_id;
        *api = &mod_scmi_from_protocol_api;
        break;

    case MOD_SCMI_API_IDX_TRANSPORT:
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT))
            return FWK_E_SUPPORT;

        ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(target_id)];
        if (!fwk_id_is_equal(source_id, ctx->transport_id))
            return FWK_E_ACCESS;

        *api = &mod_scmi_from_transport_api;
        break;

    default:
        return FWK_E_SUPPORT;
    };

    return FWK_SUCCESS;
}

static int scmi_process_event(const struct fwk_event *event,
                              struct fwk_event *resp)
{
    int status;
    struct scmi_service_ctx *ctx;
    const struct mod_scmi_to_transport_api *transport_api;
    fwk_id_t transport_id;
    uint32_t message_header;
    const void *payload;
    size_t payload_size;
    unsigned int protocol_idx;
    struct scmi_protocol *protocol;
    const char *service_name;

    ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(event->target_id)];
    transport_api = ctx->transport_api;
    transport_id = ctx->transport_id;

    service_name = fwk_module_get_name(event->target_id);

    status = transport_api->get_message_header(transport_id, &message_header);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[SCMI] %s: Unable to read message header", service_name);
        return status;
    }

    status = transport_api->get_payload(transport_id, &payload, &payload_size);
    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR("[SCMI] %s: Unable to read message payload", service_name);
        return status;
    }

    ctx->scmi_protocol_id = read_protocol_id(message_header);
    ctx->scmi_message_id = read_message_id(message_header);
    ctx->scmi_token = read_token(message_header);

    FWK_LOG_TRACE(
        "[SCMI] %s: Message [%" PRIu16 " (0x%x:0x%x)] was received",
        service_name,
        ctx->scmi_token,
        ctx->scmi_protocol_id,
        ctx->scmi_message_id);

    protocol_idx = scmi_ctx.scmi_protocol_id_to_idx[ctx->scmi_protocol_id];

    if (protocol_idx == 0) {
        FWK_LOG_ERR(
            "[SCMI] %s: Message [%" PRIu16
            " (0x%x:0x%x)] requested an "
            "unsupported protocol",
            service_name,
            ctx->scmi_token,
            ctx->scmi_protocol_id,
            ctx->scmi_message_id);
        ctx->respond(transport_id, &(int32_t) { SCMI_NOT_SUPPORTED },
                     sizeof(int32_t));
        return FWK_SUCCESS;
    }

    protocol = &scmi_ctx.protocol_table[protocol_idx];
    status = protocol->message_handler(protocol->id, event->target_id,
        payload, payload_size, ctx->scmi_message_id);

    if (status != FWK_SUCCESS) {
        FWK_LOG_ERR(
            "[SCMI] %s: Message [%" PRIu16 " (0x%x:0x%x)] handler error (%s)",
            service_name,
            ctx->scmi_token,
            ctx->scmi_protocol_id,
            ctx->scmi_message_id,
            fwk_status_str(status));

        return FWK_SUCCESS;
    }

    return FWK_SUCCESS;
}

static int scmi_start(fwk_id_t id)
{
#ifdef BUILD_HAS_NOTIFICATION
    const struct mod_scmi_service_config *config;
    unsigned int notifications_sent;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    config = fwk_module_get_data(id);

    if (fwk_id_is_equal(config->transport_notification_init_id, FWK_ID_NONE)) {
        /* Notify that the service is immediately ready */
        struct fwk_event scmi_services_initialized_notification = {
            .id = mod_scmi_notification_id_initialized,
            .source_id = id,
        };

        return fwk_notification_notify(&scmi_services_initialized_notification,
            &notifications_sent);
    }

    return fwk_notification_subscribe(config->transport_notification_init_id,
                                      config->transport_id,
                                      id);
#else
    return FWK_SUCCESS;
#endif
}

#ifdef BUILD_HAS_NOTIFICATION
static int scmi_process_notification(const struct fwk_event *event,
                                     struct fwk_event *resp_event)
{
    const struct mod_scmi_service_config *config;
    unsigned int notifications_sent;

    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    config = fwk_module_get_data(event->target_id);
    if (config == NULL)
        return FWK_E_PARAM;

    fwk_assert(fwk_id_is_equal(event->id,
                               config->transport_notification_init_id));

    /* Notify that this service is ready */
    struct fwk_event scmi_services_initialized_notification = {
        .id = mod_scmi_notification_id_initialized,
        .source_id = FWK_ID_NONE
    };

    return fwk_notification_notify(&scmi_services_initialized_notification,
        &notifications_sent);
}
#endif

/* SCMI module definition */
const struct fwk_module module_scmi = {
    .name = "SCMI",
    .api_count = MOD_SCMI_API_IDX_COUNT,
    .event_count = 1,
    #ifdef BUILD_HAS_NOTIFICATION
    .notification_count = MOD_SCMI_NOTIFICATION_IDX_COUNT,
    #endif
    .type = FWK_MODULE_TYPE_SERVICE,
    .init = scmi_init,
    .element_init = scmi_service_init,
    .bind = scmi_bind,
    .start = scmi_start,
    .process_bind_request = scmi_process_bind_request,
    .process_event = scmi_process_event,
    #ifdef BUILD_HAS_NOTIFICATION
    .process_notification = scmi_process_notification,
    #endif
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     System Control and Management Interface (SCMI) support.
 */

#include <internal/mod_scmi.h>
#ifdef BUILD_HAS_BASE_PROTOCOL
#    include <internal/mod_scmi_base.h>
#endif
#include <internal/scmi.h>

#include <mod_scmi.h>
#include <mod_scmi_header.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_string.h>

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
#    include <mod_resource_perms.h>
#endif

#include <inttypes.h>

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS

/* Following macros are used for scmi notification related operations */
#    define MOD_SCMI_PROTOCOL_MAX_OPERATION_ID 0x20
#    define MOD_SCMI_PROTOCOL_OPERATION_IDX_INVALID 0xFF

struct scmi_notification_subscribers {
    unsigned int agent_count;
    unsigned int element_count;
    unsigned int operation_count;
    unsigned int operation_idx;
    uint8_t operation_id_to_idx[MOD_SCMI_PROTOCOL_MAX_OPERATION_ID];

    /*
     * Table of protocol operations for which SCMI notification is requested
     * An agent which is requesting a SCMI notification is identified using its
     * service_id and associated context as below.
     *
     * Usually, a notification is requested for
     * 1. A specific operation on the protocol.
     * 2. An element (e.g. domain_id)
     * 3. And a specific agent (e.g PSCI/OSPM)
     *
     * e.g. in Performance domain case,
     * 1. operation_id maps to
     *    either PERFORMANCE_LIMITS_CHANGED/PERFORMANCE_LEVEL_CHANGED
     * 2. element maps to a performance domain.
     * 3. And an agent maps to either a PSCI agent or an OSPM agent
     *
     * Thus, a service_id of a requesting agent can be indexed using above
     * information using a simple 3-dimentional array as below
     *
     *   agent_service_ids[operation_idx][element_idx][agent_idx]
     */
    fwk_id_t *agent_service_ids;
};

#endif

/*
 * Entry zero (0) of the protocol table 'protocol_table' is not used, as index
 * 0 is the index of the unused entries of the 'scmi_protocol_id_to_idx[]'
 * table. Entry one (1) is reserved for the base protocol implemented in this
 * file.
 */
#define PROTOCOL_TABLE_BASE_PROTOCOL_IDX 1
#define PROTOCOL_TABLE_RESERVED_ENTRIES_COUNT 2

static struct mod_scmi_ctx scmi_ctx;

/*
 * Utility functions
 */

/*
 * Return a packed 32-bit message header comprised of an 8-bit message
 * identifier, a 2-bit message type, an 8-bit protocol identifier,
 * and a 10-bit token.
 */
static uint32_t scmi_message_header(uint8_t message_id,
    uint8_t message_type, uint8_t protocol_id, uint8_t token)
{
    return (
        (((message_id) << SCMI_MESSAGE_HEADER_MESSAGE_ID_POS) &
         SCMI_MESSAGE_HEADER_MESSAGE_ID_MASK) |
        (((message_type) << SCMI_MESSAGE_HEADER_MESSAGE_TYPE_POS) &
         SCMI_MESSAGE_HEADER_MESSAGE_TYPE_MASK) |
        (((protocol_id) << SCMI_MESSAGE_HEADER_PROTOCOL_ID_POS) &
         SCMI_MESSAGE_HEADER_PROTOCOL_ID_MASK) |
        (((token) << SCMI_MESSAGE_HEADER_TOKEN_POS) &
         SCMI_MESSAGE_HEADER_TOKEN_MASK));
}

static uint16_t read_message_id(uint32_t message_header)
{
    return (uint16_t)(
        ((message_header & SCMI_MESSAGE_HEADER_MESSAGE_ID_MASK) >>
         SCMI_MESSAGE_HEADER_MESSAGE_ID_POS));
}

static uint8_t read_message_type(uint32_t message_header)
{
    return (uint8_t)(
        ((message_header & SCMI_MESSAGE_HEADER_MESSAGE_TYPE_MASK) >>
         SCMI_MESSAGE_HEADER_MESSAGE_TYPE_POS));
}

static uint8_t read_protocol_id(uint32_t message_header)
{
    return (uint8_t)(
        ((message_header & SCMI_MESSAGE_HEADER_PROTOCOL_ID_MASK) >>
         SCMI_MESSAGE_HEADER_PROTOCOL_ID_POS));
}

static uint16_t read_token(uint32_t message_header)
{
    return (uint16_t)(
        ((message_header & SCMI_MESSAGE_HEADER_TOKEN_MASK) >>
         SCMI_MESSAGE_HEADER_TOKEN_POS));
}

static const char *get_message_type_str(const struct scmi_service_ctx *ctx)
{
    enum mod_scmi_message_type message_type = ctx->scmi_message_type;
    switch (message_type) {
    case MOD_SCMI_MESSAGE_TYPE_COMMAND:
        if (ctx->config->scmi_entity_role == MOD_SCMI_ROLE_PLATFORM) {
            return "Cmd";
        } else if (ctx->config->scmi_entity_role == MOD_SCMI_ROLE_AGENT) {
            return "Cmd-Resp";
        } else {
            return "";
        }

    case MOD_SCMI_MESSAGE_TYPE_DELAYED_RESPONSE:
        return "Del-Resp";

    case MOD_SCMI_MESSAGE_TYPE_NOTIFICATION:
        return "Notif";

    default:
        return "Invalid message";
    }
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
    struct fwk_event_light event = (struct fwk_event_light){
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_SCMI, 0),
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        .target_id = service_id,
    };

    return fwk_put_event(&event);
}

static const struct mod_scmi_from_transport_api scmi_from_transport_api = {
    .signal_error = signal_error,
    .signal_message = signal_message,
};

/*
 * SCMI protocol module -> SCMI module interface
 */

static int get_agent_count(unsigned int *agent_count)
{
    if (agent_count == NULL) {
        return FWK_E_PARAM;
    }

    /* Include the platform in the count */
    *agent_count = scmi_ctx.config->agent_count + 1u;

    return FWK_SUCCESS;
}


static int get_agent_id(fwk_id_t service_id, unsigned int *agent_id)
{
    struct scmi_service_ctx *ctx;

    if (agent_id == NULL) {
        return FWK_E_PARAM;
    }

    ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(service_id)];

    *agent_id = ctx->config->scmi_agent_id;

    return FWK_SUCCESS;
}

static int get_agent_type(uint32_t scmi_agent_id,
                          enum scmi_agent_type *agent_type)
{
    if ((agent_type == NULL) ||
        (scmi_agent_id > scmi_ctx.config->agent_count) ||
        (scmi_agent_id == MOD_SCMI_PLATFORM_ID)) {
        return FWK_E_PARAM;
    }

    *agent_type = scmi_ctx.config->agent_table[scmi_agent_id].type;

    return FWK_SUCCESS;
}

static int get_max_payload_size(fwk_id_t service_id, size_t *size)
{
    struct scmi_service_ctx *ctx;

    if (size == NULL) {
        return FWK_E_PARAM;
    }

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

static int respond(fwk_id_t service_id, const void *payload, size_t size)
{
    int status;
    const struct scmi_service_ctx *ctx;
    const char *service_name;
    const char *message_type_name;

    ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(service_id)];

    service_name = fwk_module_get_element_name(service_id);
    message_type_name = get_message_type_str(ctx);

    /*
     * Print to the error log if the message was not successfully processed.
     * We assume here that the first payload entry of the command response
     * holds an SCMI status code. This is the case for all the SCMI commands
     * specified so far, but it is not explicitly stated (yet) in the
     * specification it should be like that for all commands.
     */
    if ((payload != NULL) && (*((int32_t *)payload) < SCMI_SUCCESS)) {
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
        FWK_LOG_ERR(
            "[SCMI] %s: %s [%" PRIu16 " (0x%x:0x%x)] returned error (%d)",
            service_name,
            message_type_name,
            ctx->scmi_token,
            ctx->scmi_protocol_id,
            ctx->scmi_message_id,
            *((int *)payload));
#else
        (void)service_name;
        (void)message_type_name;
#endif
    } else {
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_DEBUG
        FWK_LOG_DEBUG(
            "[SCMI] %s: %s [%" PRIu16 " (0x%x:0x%x)] returned successfully",
            service_name,
            message_type_name,
            ctx->scmi_token,
            ctx->scmi_protocol_id,
            ctx->scmi_message_id);
#endif
    }

    status = ctx->respond(ctx->transport_id, payload, size);
    if (status != FWK_SUCCESS) {
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
        FWK_LOG_ERR(
            "[SCMI] %s: %s [%" PRIu16
            " (0x%x:0x%x)] failed to respond (%s)",
            service_name,
            message_type_name,
            ctx->scmi_token,
            ctx->scmi_protocol_id,
            ctx->scmi_message_id,
            fwk_status_str(status));
#endif
    }
    return status;
}

static void scmi_notify(fwk_id_t id, int protocol_id, int message_id,
    const void *payload, size_t size)
{
    const struct scmi_service_ctx *ctx, *p2a_ctx;
    uint32_t message_header;
    int status;
    bool request_ack_by_interrupt;

    /*
     * The ID is the identifier of the service channel which
     * the agent used to request notificatiosn on. This ID is
     * linked to a P2A channel by the scmi_p2a_id.
     */
    if (fwk_id_is_equal(id, FWK_ID_NONE)) {
        return;
    }

    ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(id)];
    if (ctx == NULL) {
        return;
    }
    /* ctx is the original A2P service channel */
    if (fwk_id_is_equal(ctx->config->scmi_p2a_id, FWK_ID_NONE)) {
        return;
    }
    /* Get the P2A service channel for A2P ctx */
    p2a_ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(
        ctx->config->scmi_p2a_id)];
    if ((p2a_ctx == NULL) || (p2a_ctx->transmit == NULL)) {
        return; /* No notification service configured */
    }

    message_header = scmi_message_header(
        (uint8_t)message_id,
        (uint8_t)MOD_SCMI_MESSAGE_TYPE_NOTIFICATION,
        (uint8_t)protocol_id,
        0);

    request_ack_by_interrupt = false;
    status = p2a_ctx->transmit(
        p2a_ctx->transport_id,
        message_header,
        payload,
        size,
        request_ack_by_interrupt);
    if (status != FWK_SUCCESS) {
        FWK_LOG_DEBUG("[SCMI] %s @%d", __func__, __LINE__);
    }
}

int scmi_send_message(
    uint8_t message_id,
    uint8_t protocol_id,
    uint8_t token,
    fwk_id_t service_id,
    const void *payload,
    size_t payload_size,
    bool request_ack_by_interrupt)
{
    int status;

    /* All commands, synchronous or asynchronous, have a message type of 0 */
    uint8_t message_type = 0;

    /* Create the SCMI message header from the arguments */
    uint32_t message_header =
        scmi_message_header(message_id, message_type, protocol_id, token);

    if (fwk_id_is_equal(service_id, FWK_ID_NONE)) {
        return FWK_E_DATA;
    }

    /* Fetch scmi module context data using the service_id */
    const struct mod_scmi_to_transport_api *transport_api;
    const struct scmi_service_ctx *ctx;
    ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(service_id)];

    if (ctx == NULL) {
        return FWK_E_DATA;
    }

    /* Initalize the transport api pointer to TRANSPORT module api */
    transport_api = ctx->transport_api;

    /* Send the SCMI message using the transport_transmit() API  */
    status = transport_api->transmit(
        ctx->transport_id,
        message_header,
        payload,
        payload_size,
        request_ack_by_interrupt);

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
    if (status == FWK_SUCCESS) {
        FWK_LOG_DEBUG(
            "[SCMI] %s: Cmd [%" PRIu16 " (0x%x:0x%x)] was sent",
            fwk_module_get_element_name(service_id),
            token,
            protocol_id,
            message_id);
    } else {
        FWK_LOG_ERR(
            "[SCMI] %s: Cmd [%" PRIu16 " (0x%x:0x%x)] failed to respond (%s)",
            fwk_module_get_element_name(service_id),
            token,
            protocol_id,
            message_id,
            fwk_status_str(status));
    }
#endif

    return status;
};

int response_message_handler(fwk_id_t service_id)
{
    int status;
    const struct scmi_service_ctx *ctx;

    ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(service_id)];

    /* release the tranport channel lock */
    status =
        ctx->transport_api->release_transport_channel_lock(ctx->transport_id);

    return status;
}

static const struct mod_scmi_from_protocol_api scmi_from_protocol_api = {
    .get_agent_count = get_agent_count,
    .get_agent_id = get_agent_id,
    .get_agent_type = get_agent_type,
    .get_max_payload_size = get_max_payload_size,
    .write_payload = write_payload,
    .respond = respond,
    .notify = scmi_notify,
};

static const struct mod_scmi_from_protocol_req_api
    scmi_from_protocol_req_api = {
        .scmi_send_message = scmi_send_message,
        .response_message_handler = response_message_handler,
    };

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
static struct scmi_notification_subscribers *notification_subscribers(
    unsigned int protocol_id)
{
    unsigned int protocol_idx;
    protocol_idx = scmi_ctx.scmi_protocol_id_to_idx[protocol_id];
    /*
     * Couple of entries are reserved in scmi_protocol_id_to_idx,
     * out of these, one is reserved for the Base protocol. Since we may need
     * to enable notification for the Base protocol adjust the protocol_idx
     * accordingly.
     */
    protocol_idx -= (PROTOCOL_TABLE_RESERVED_ENTRIES_COUNT - 1);
    return &scmi_ctx.scmi_notif_subscribers[protocol_idx];
}

static int scmi_notification_init(
    unsigned int protocol_id,
    unsigned int agent_count,
    unsigned int element_count,
    unsigned int operation_count)
{
    int i;
    int total_count;
    struct scmi_notification_subscribers *subscribers =
        notification_subscribers(protocol_id);

    subscribers->agent_count = agent_count;
    subscribers->element_count = element_count;
    subscribers->operation_count = operation_count;

    total_count = (int)(operation_count * element_count * agent_count);

    subscribers->agent_service_ids =
        fwk_mm_calloc((size_t)total_count, sizeof(fwk_id_t));

    /*
     * Mark all operations_idx as invalid. This will be updated
     * whenever an agent subscribes to a notification for an operation.
     */
    fwk_str_memset(
        subscribers->operation_id_to_idx,
        MOD_SCMI_PROTOCOL_OPERATION_IDX_INVALID,
        MOD_SCMI_PROTOCOL_MAX_OPERATION_ID);

    for (i = 0; i < total_count; i++) {
        subscribers->agent_service_ids[i] = FWK_ID_NONE;
    }

    return FWK_SUCCESS;
}

static int scmi_notification_service_idx(
    unsigned int agent_idx,
    unsigned int element_idx,
    unsigned int operation_idx,
    unsigned int agent_count,
    unsigned int element_count)
{
    /*
     * The index is from a 3-dimentional array
     * [operation_idx][element_idx][agent_idx]
     * The calculation of service_id offset in above array is as below
     * 1. Get the offset of the service_id in the 3rd array .
     *    +
     * 2. Get the offset of the 3rd array within the 2nd array.
     *    +
     * 3. Get the offset of the 2nd array within the first array.
     */
    return (
        int)(agent_idx + element_idx * agent_count + operation_idx * element_count * agent_count);
}

static int scmi_notification_add_subscriber(
    unsigned int protocol_id,
    unsigned int element_idx,
    unsigned int operation_id,
    fwk_id_t service_id)
{
    int status;
    unsigned int service_id_idx;
    unsigned int agent_idx;

    struct scmi_notification_subscribers *subscribers =
        notification_subscribers(protocol_id);

    status = get_agent_id(service_id, &agent_idx);
    if (status != FWK_SUCCESS) {
        return status;
    }

    fwk_assert(operation_id < MOD_SCMI_PROTOCOL_MAX_OPERATION_ID);
    /*
     * Initialize only if the entry is
     * invalid (MOD_SCMI_PROTOCOL_OPERATION_IDX_INVALID)
     */
    if (subscribers->operation_id_to_idx[operation_id] ==
        MOD_SCMI_PROTOCOL_OPERATION_IDX_INVALID) {
        fwk_assert(subscribers->operation_idx < subscribers->operation_count);
        subscribers->operation_id_to_idx[operation_id] =
            (uint8_t)subscribers->operation_idx++;
    }

    service_id_idx = (unsigned int)scmi_notification_service_idx(
        agent_idx,
        element_idx,
        subscribers->operation_id_to_idx[operation_id],
        subscribers->agent_count,
        subscribers->element_count);

    subscribers->agent_service_ids[service_id_idx] = service_id;

    return FWK_SUCCESS;
}

static int scmi_notification_remove_subscriber(
    unsigned int protocol_id,
    unsigned int agent_idx,
    unsigned int element_idx,
    unsigned int operation_id)
{
    unsigned int operation_idx = 0;
    unsigned int service_id_idx;

    struct scmi_notification_subscribers *subscribers =
        notification_subscribers(protocol_id);

    fwk_assert(operation_id < MOD_SCMI_PROTOCOL_MAX_OPERATION_ID);

    operation_idx = subscribers->operation_id_to_idx[operation_id];

    service_id_idx = (unsigned int)scmi_notification_service_idx(
        agent_idx,
        element_idx,
        operation_idx,
        subscribers->agent_count,
        subscribers->element_count);

    subscribers->agent_service_ids[service_id_idx] = FWK_ID_NONE;

    return FWK_SUCCESS;
}

static int scmi_notification_notify(
    unsigned int protocol_id,
    unsigned int operation_id,
    unsigned int scmi_response_id,
    void *payload_p2a,
    size_t payload_size)
{
    unsigned int i, j;
    unsigned int operation_idx;
    fwk_id_t service_id;
    unsigned int service_id_idx;

    struct scmi_notification_subscribers *subscribers =
        notification_subscribers(protocol_id);

    fwk_assert(operation_id < MOD_SCMI_PROTOCOL_MAX_OPERATION_ID);
    operation_idx = subscribers->operation_id_to_idx[operation_id];

    /*
     * Silently return FWK_SUCCESS if no valid operation_idx is found
     * for a given operation_id. This is required in where(e.g. DVFS/perf)
     * scmi_notification_notify gets called before a call to
     * scmi_notification_add_subscriber.
     */
    if (operation_idx == MOD_SCMI_PROTOCOL_OPERATION_IDX_INVALID) {
        return FWK_SUCCESS;
    }

    for (i = 0; i < subscribers->element_count; i++) {
        /* Skip agent 0, platform agent */
        for (j = 1; j < subscribers->agent_count; j++) {
            service_id_idx = (unsigned int)scmi_notification_service_idx(
                j,
                i,
                operation_idx,
                subscribers->agent_count,
                subscribers->element_count);

            service_id = subscribers->agent_service_ids[service_id_idx];

            if (!fwk_id_is_equal(service_id, FWK_ID_NONE)) {
                scmi_notify(
                    service_id,
                    (int)protocol_id,
                    (int)scmi_response_id,
                    payload_p2a,
                    payload_size);
            }
        }
    }

    return FWK_SUCCESS;
}

static struct mod_scmi_notification_api mod_scmi_notif_api = {
    .scmi_notification_init = scmi_notification_init,
    .scmi_notification_add_subscriber = scmi_notification_add_subscriber,
    .scmi_notification_remove_subscriber = scmi_notification_remove_subscriber,
    .scmi_notification_notify = scmi_notification_notify,
};
#endif

/*
 * Framework handlers
 */

static int scmi_init(fwk_id_t module_id, unsigned int service_count,
                     const void *data)
{
    struct mod_scmi_config *config = (struct mod_scmi_config *)data;
    unsigned int agent_idx;
    const struct mod_scmi_agent *agent;

    if (config == NULL) {
        return FWK_E_PARAM;
    }

    if (config->agent_count > MOD_SCMI_AGENT_ID_MAX) {
        return FWK_E_PARAM;
    }

    /*
     * Loop over the agent descriptors. The MOD_SCMI_PLATFORM_ID(0) entry of
     * the table - that would refer to the platform - is ignored.
     */
    for (agent_idx = MOD_SCMI_PLATFORM_ID + 1;
         agent_idx <= config->agent_count;
         agent_idx++) {
        agent = &config->agent_table[agent_idx];
        if (agent->type >= SCMI_AGENT_TYPE_COUNT) {
            return FWK_E_PARAM;
        }
    }

    scmi_ctx.protocol_table = fwk_mm_calloc(
        config->protocol_count_max + PROTOCOL_TABLE_RESERVED_ENTRIES_COUNT,
        sizeof(scmi_ctx.protocol_table[0]));

    if (config->protocol_requester_count_max != 0) {
        scmi_ctx.protocol_requester_table = fwk_mm_calloc(
            config->protocol_requester_count_max,
            sizeof(scmi_ctx.protocol_requester_table[0]));
    } else {
        scmi_ctx.protocol_requester_table = NULL;
    }

    scmi_ctx.service_ctx_table = fwk_mm_calloc(
        service_count, sizeof(scmi_ctx.service_ctx_table[0]));

#ifdef BUILD_HAS_BASE_PROTOCOL
    scmi_ctx.protocol_table[PROTOCOL_TABLE_BASE_PROTOCOL_IDX].message_handler =
        scmi_base_message_handler;
    scmi_ctx.scmi_protocol_id_to_idx[MOD_SCMI_PROTOCOL_ID_BASE] =
        PROTOCOL_TABLE_BASE_PROTOCOL_IDX;
    scmi_base_set_api(&scmi_from_protocol_api);
    scmi_base_set_shared_ctx(&scmi_ctx);
#endif
    scmi_ctx.config = config;

    return FWK_SUCCESS;
}

static int scmi_service_init(fwk_id_t service_id, unsigned int unused,
                             const void *data)
{
    const struct mod_scmi_service_config *config =
        (struct mod_scmi_service_config *)data;
    struct scmi_service_ctx *ctx;

    if (((config->scmi_agent_id == MOD_SCMI_PLATFORM_ID) ||
         (config->scmi_agent_id > scmi_ctx.config->agent_count)) &&
        (config->scmi_entity_role == MOD_SCMI_ROLE_PLATFORM)) {
        return FWK_E_PARAM;
    }

    ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(service_id)];
    ctx->config = config;

    return FWK_SUCCESS;
}

static int scmi_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct scmi_service_ctx *ctx;
    const struct mod_scmi_to_transport_api *transport_api = NULL;
    unsigned int protocol_idx;
    struct scmi_protocol *protocol, *protocol_req;
    struct mod_scmi_to_protocol_api *protocol_api = NULL;
    uint8_t scmi_protocol_id;

    if (round == 0) {
        if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
            return FWK_SUCCESS;
        }

        ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(id)];
        status = fwk_module_bind(ctx->config->transport_id,
                                 ctx->config->transport_api_id, &transport_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        if ((transport_api->get_secure == NULL) ||
            (transport_api->get_max_payload_size == NULL) ||
            (transport_api->get_message_header == NULL) ||
            (transport_api->get_payload == NULL) ||
            (transport_api->write_payload == NULL)) {
            return FWK_E_DATA;
        }

        ctx->transport_api = transport_api;
        ctx->transport_id = ctx->config->transport_id;
        ctx->respond = transport_api->respond;
        ctx->transmit = transport_api->transmit;

        return FWK_SUCCESS;
    }

    if (fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_SUCCESS;
    }

    for (protocol_idx = 0;
         protocol_idx < scmi_ctx.protocol_count; protocol_idx++) {
        protocol = &scmi_ctx.protocol_table[
            PROTOCOL_TABLE_RESERVED_ENTRIES_COUNT + protocol_idx];

        status = fwk_module_bind(protocol->id,
            FWK_ID_API(fwk_id_get_module_idx(protocol->id), 0), &protocol_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        if ((protocol_api->get_scmi_protocol_id == NULL) ||
            (protocol_api->message_handler == NULL)) {
            return FWK_E_DATA;
        }
        status = protocol_api->get_scmi_protocol_id(protocol->id,
                                                    &scmi_protocol_id);
        if (status != FWK_SUCCESS) {
            return status;
        }

        if (scmi_ctx.scmi_protocol_id_to_idx[scmi_protocol_id] != 0) {
            return FWK_E_STATE;
        }

        scmi_ctx.scmi_protocol_id_to_idx[scmi_protocol_id] =
            (uint8_t)(protocol_idx + PROTOCOL_TABLE_RESERVED_ENTRIES_COUNT);
        protocol->message_handler = protocol_api->message_handler;
    }

    for (protocol_idx = 0; protocol_idx < scmi_ctx.protocol_requester_count;
         protocol_idx++) {
        protocol_req = &scmi_ctx.protocol_requester_table[protocol_idx];

        status = fwk_module_bind(
            protocol_req->id,
            FWK_ID_API(fwk_id_get_module_idx(protocol_req->id), 0),
            &protocol_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        if ((protocol_api->get_scmi_protocol_id == NULL) ||
            (protocol_api->message_handler == NULL)) {
            return FWK_E_DATA;
        }
        status = protocol_api->get_scmi_protocol_id(
            protocol_req->id, &scmi_protocol_id);
        if (status != FWK_SUCCESS) {
            return status;
        }

        scmi_ctx.scmi_protocol_requester_id_to_idx[scmi_protocol_id] =
            (uint8_t)(protocol_idx);
        protocol_req->message_handler = protocol_api->message_handler;
    }

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    status = fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_RESOURCE_PERMS),
        FWK_ID_API(FWK_MODULE_IDX_RESOURCE_PERMS, MOD_RES_PERM_RESOURCE_PERMS),
        &scmi_ctx.res_perms_api);
    if (status != FWK_SUCCESS) {
        return status;
    }
#endif

    return FWK_SUCCESS;
}

static int scmi_process_bind_request(fwk_id_t source_id, fwk_id_t target_id,
                                     fwk_id_t api_id, const void **api)
{
    unsigned int api_idx;
    struct scmi_service_ctx *ctx;
    enum mod_scmi_api_idx api_id_type;

    api_idx = fwk_id_get_api_idx(api_id);

    api_id_type = (enum mod_scmi_api_idx)api_idx;

    switch (api_id_type) {
    case MOD_SCMI_API_IDX_PROTOCOL:
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_MODULE)) {
            return FWK_E_SUPPORT;
        }

        if (scmi_ctx.protocol_count >= scmi_ctx.config->protocol_count_max) {
            return FWK_E_NOMEM;
        }

        scmi_ctx.protocol_table[PROTOCOL_TABLE_RESERVED_ENTRIES_COUNT +
                                scmi_ctx.protocol_count++].id = source_id;
        *api = &scmi_from_protocol_api;
        break;

    case MOD_SCMI_API_IDX_PROTOCOL_REQ:
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_MODULE)) {
            return FWK_E_SUPPORT;
        }
        if (scmi_ctx.protocol_requester_count >=
            scmi_ctx.config->protocol_requester_count_max) {
            return FWK_E_PARAM;
        }

        scmi_ctx.protocol_requester_table[scmi_ctx.protocol_requester_count++]
            .id = source_id;
        *api = &scmi_from_protocol_req_api;
        break;

    case MOD_SCMI_API_IDX_TRANSPORT:
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT)) {
            return FWK_E_SUPPORT;
        }

        ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(target_id)];
        if (!fwk_id_is_equal(source_id, ctx->transport_id)) {
            return FWK_E_ACCESS;
        }

        *api = &scmi_from_transport_api;
        break;

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    case MOD_SCMI_API_IDX_NOTIFICATION:
        if (!fwk_id_is_type(target_id, FWK_ID_TYPE_MODULE)) {
            return FWK_E_SUPPORT;
        }

        *api = &mod_scmi_notif_api;
        break;
#endif

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
#ifndef BUILD_HAS_MOD_RESOURCE_PERMS
    enum scmi_agent_type agent_type;
    unsigned int agent_id;
    unsigned int dis_protocol_list_psci_index;
#endif
    struct scmi_protocol *protocol;
    const char *service_name;
    const char *message_type_name;

    ctx = &scmi_ctx.service_ctx_table[fwk_id_get_element_idx(event->target_id)];
    transport_api = ctx->transport_api;
    transport_id = ctx->transport_id;

    service_name = fwk_module_get_element_name(event->target_id);

    status = transport_api->get_message_header(transport_id, &message_header);
    if (status != FWK_SUCCESS) {
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
        FWK_LOG_ERR("[SCMI] %s: Unable to read message header", service_name);
#else
        (void)service_name;
#endif
        return status;
    }

    status = transport_api->get_payload(transport_id, &payload, &payload_size);
    if (status != FWK_SUCCESS) {
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
        FWK_LOG_ERR("[SCMI] %s: Unable to read message payload", service_name);
#else
        (void)service_name;
#endif
        return status;
    }

    ctx->scmi_protocol_id = read_protocol_id(message_header);
    ctx->scmi_message_id = read_message_id(message_header);
    ctx->scmi_message_type =
        (enum mod_scmi_message_type)read_message_type(message_header);
    ctx->scmi_token = read_token(message_header);
    message_type_name = get_message_type_str(ctx);

#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_DEBUG
    FWK_LOG_DEBUG(
        "[SCMI] %s: %s [%" PRIu16 " (0x%x:0x%x)] was received",
        service_name,
        message_type_name,
        ctx->scmi_token,
        ctx->scmi_protocol_id,
        ctx->scmi_message_id);
#else
    (void)service_name;
    (void)message_type_name;
#endif
    if (ctx->config->scmi_entity_role == MOD_SCMI_ROLE_PLATFORM) {
        protocol_idx = scmi_ctx.scmi_protocol_id_to_idx[ctx->scmi_protocol_id];
        if (protocol_idx == 0) {
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
            FWK_LOG_ERR(
                "[SCMI] %s: %s [%" PRIu16
                "(0x%x:0x%x)] requested an unsupported protocol",
                service_name,
                message_type_name,
                ctx->scmi_token,
                ctx->scmi_protocol_id,
                ctx->scmi_message_id);
#endif
            status = ctx->respond(
                transport_id,
                &(int32_t){ SCMI_NOT_SUPPORTED },
                sizeof(int32_t));
            if (status != FWK_SUCCESS) {
                FWK_LOG_DEBUG("[SCMI] %s @%d", __func__, __LINE__);
            }
            return FWK_SUCCESS;
        }

#ifndef BUILD_HAS_MOD_RESOURCE_PERMS
    status = get_agent_id(event->target_id, &agent_id);
    if (status != FWK_SUCCESS) {
#    if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
        FWK_LOG_ERR("[SCMI] %s: Unable to get agent id", service_name);
#    endif
        return status;
    }

    status = get_agent_type(agent_id, &agent_type);
    if (status != FWK_SUCCESS) {
#    if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
        FWK_LOG_ERR("[SCMI] %s: Unable to get agent type", service_name);
#    endif
        return status;
    }

    if (agent_type == SCMI_AGENT_TYPE_PSCI) {
        /*
         * check if the current protocol is within the disabled protocols
         * list for PSCI agent.
         */
        for (dis_protocol_list_psci_index = 0; dis_protocol_list_psci_index <
             scmi_ctx.config->dis_protocol_count_psci;
             dis_protocol_list_psci_index++) {
            if (ctx->scmi_protocol_id ==
                scmi_ctx.config
                    ->dis_protocol_list_psci[dis_protocol_list_psci_index]) {
#    if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
                FWK_LOG_ERR(
                    "[SCMI] %s: %s [%" PRIu16
                    "(0x%x:0x%x)] requested a denied protocol",
                    service_name,
                    message_type_name,
                    ctx->scmi_token,
                    ctx->scmi_protocol_id,
                    ctx->scmi_message_id);
#    endif
                status = ctx->respond(
                    transport_id, &(int32_t){ SCMI_DENIED }, sizeof(int32_t));
                if (status != FWK_SUCCESS) {
                    FWK_LOG_DEBUG("[SCMI] %s @%d", __func__, __LINE__);
                }
                return FWK_SUCCESS;
            }
        }
    }
#endif
    protocol = &scmi_ctx.protocol_table[protocol_idx];
    } else if (ctx->config->scmi_entity_role == MOD_SCMI_ROLE_AGENT) {
        protocol_idx =
            scmi_ctx.scmi_protocol_requester_id_to_idx[ctx->scmi_protocol_id];
        protocol = &scmi_ctx.protocol_requester_table[protocol_idx];
    } else {
        return FWK_E_INIT;
    }

    status = protocol->message_handler(
        protocol->id,
        event->target_id,
        payload,
        payload_size,
        ctx->scmi_message_id);

    if (status != FWK_SUCCESS) {
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
        FWK_LOG_ERR(
            "[SCMI] %s: %s [%" PRIu16 " (0x%x:0x%x)] handler error (%s)",
            service_name,
            message_type_name,
            ctx->scmi_token,
            ctx->scmi_protocol_id,
            ctx->scmi_message_id,
            fwk_status_str(status));
#endif
        return FWK_SUCCESS;
    }

    /*
     * If we are receiving a command response, we need to set the transport
     * layer that we already received the response and are free
     */
    if (ctx->config->scmi_entity_role == MOD_SCMI_ROLE_AGENT) {
        status =
            ctx->transport_api->release_transport_channel_lock(transport_id);
        if (status != FWK_SUCCESS) {
#if FWK_LOG_LEVEL <= FWK_LOG_LEVEL_ERROR
            FWK_LOG_ERR(
                "[SCMI] %s: %s [%" PRIu16
                " (0x%x:0x%x)] confirm reception error (%s)",
                service_name,
                message_type_name,
                ctx->scmi_token,
                ctx->scmi_protocol_id,
                ctx->scmi_message_id,
                fwk_status_str(status));
#endif
            return FWK_SUCCESS;
        }
    }

    return FWK_SUCCESS;
}

static int scmi_start(fwk_id_t id)
{
#ifdef BUILD_HAS_NOTIFICATION
    const struct mod_scmi_service_config *config;
    unsigned int notifications_sent;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
#    ifdef BUILD_HAS_SCMI_NOTIFICATIONS
        /* scmi_ctx.protocol_count + 1 to include Base protocol */
        scmi_ctx.scmi_notif_subscribers = fwk_mm_calloc(
            scmi_ctx.protocol_count + 1,
            sizeof(struct scmi_notification_subscribers));
#    endif

        return FWK_SUCCESS;
    }

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
    if (config == NULL) {
        return FWK_E_PARAM;
    }

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
    .api_count = (unsigned int)MOD_SCMI_API_IDX_COUNT,
    .event_count = 1,
#ifdef BUILD_HAS_NOTIFICATION
    .notification_count = (unsigned int)MOD_SCMI_NOTIFICATION_IDX_COUNT,
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

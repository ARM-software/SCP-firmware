/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     System Control and Management Interface (SCMI) support.
 */

#ifndef MOD_INTERNAL_SCMI_H
#define MOD_INTERNAL_SCMI_H

#include <mod_scmi.h>
#include <mod_scmi_header.h>

#include <fwk_id.h>

#include <stddef.h>
#include <stdint.h>

/* SCMI service context */
struct scmi_service_ctx {
    /* Pointer to SCMI service configuration data */
    const struct mod_scmi_service_config *config;

    /*
     * Identifier of the transport entity used by the service to read/respond
     * to SCMI messages.
     */
    fwk_id_t transport_id;

    /* Pointer to the transport API used to read and respond to messages */
    const struct mod_scmi_to_transport_api *transport_api;

    /*
     * Copy of the pointer to the 'respond' function within the transport API.
     */
    int (*respond)(fwk_id_t transport_id, const void *payload, size_t size);

    /*
     * Copy of the pointer to the 'transmit' function within the transport API.
     */
    int (*transmit)(
        fwk_id_t transport_id,
        uint32_t message_header,
        const void *payload,
        size_t size,
        bool request_ack_by_interrupt);

    /* SCMI message token, used by the agent to identify individual messages */
    uint16_t scmi_token;

    /* SCMI identifier of the protocol processing the current message */
    unsigned int scmi_protocol_id;

    /* SCMI identifier of the message currently being processed */
    unsigned int scmi_message_id;

    /* SCMI type of the message currently being processed */
    enum mod_scmi_message_type scmi_message_type;
};

struct scmi_protocol {
    /* SCMI protocol message handler */
    mod_scmi_message_handler_t *message_handler;

    /* SCMI protocol framework identifier */
    fwk_id_t id;

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    /* SCMI protocol notification message handler */
    mod_scmi_notification_message_handler_t *notification_handler;
#endif
};

struct mod_scmi_ctx {
    /* SCMI module configuration data */
    struct mod_scmi_config *config;

    /* Table of bound protocols */
    struct scmi_protocol *protocol_table;

    /* Number of bound protocols */
    unsigned int protocol_count;

    /* Table of bound protocols as requesters */
    struct scmi_protocol *protocol_requester_table;

    /* Number of bound protocols as requesters */
    unsigned int protocol_requester_count;

    /*
     * SCMI protocol identifier to the index of the entry in protocol_table[]
     * dedicated to the protocol.
     */
    uint8_t scmi_protocol_id_to_idx[MOD_SCMI_PROTOCOL_ID_MAX + 1];

    /*
     * SCMI protocol identifier to the index of the entry in
     * protocol_requester_table[] dedicated to the protocol.
     */
    uint8_t scmi_protocol_requester_id_to_idx[MOD_SCMI_PROTOCOL_ID_MAX + 1];

    /* Table of service contexts */
    struct scmi_service_ctx *service_ctx_table;

#ifdef BUILD_HAS_MOD_RESOURCE_PERMS
    /* SCMI Resource Permissions API */
    const struct mod_res_permissions_api *res_perms_api;
#endif
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    /* Table of scmi notification subscribers */
    struct scmi_notification_subscribers *scmi_notif_subscribers;
#endif
};

#endif /* MOD_INTERNAL_SCMI_H */

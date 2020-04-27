/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
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

#define SCMI_VERSION 0x10000

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
    int (*transmit)(fwk_id_t transport_id, uint32_t message_header,
        const void *payload, size_t size);

    /* SCMI message token, used by the agent to identify individual messages */
    uint16_t scmi_token;

    /* SCMI identifier of the protocol processing the current message */
    unsigned int scmi_protocol_id;

    /* SCMI identifier of the message currently being processed */
    unsigned int scmi_message_id;

    /* SCMI type of the message currently being processed */
    enum mod_scmi_message_type scmi_message_type;
};

#endif /* MOD_INTERNAL_SCMI_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SCMI_EXTRA_H_
#define MOD_SCMI_EXTRA_H_

#include "fwk_id.h"

#include <mod_scmi.h>

int get_agent_count(unsigned int *agent_count);
int get_agent_id(fwk_id_t service_id, unsigned int *agent_id);
int respond(fwk_id_t service_id, const void *payload, size_t size);

int scmi_notification_init(
    unsigned int protocol_id,
    unsigned int agent_count,
    unsigned int element_count,
    unsigned int operation_count);

int scmi_notification_add_subscriber(
    unsigned int protocol_id,
    unsigned int element_idx,
    unsigned int operation_id,
    fwk_id_t service_id);

int scmi_notification_remove_subscriber(
    unsigned int protocol_id,
    unsigned int agent_idx,
    unsigned int element_idx,
    unsigned int operation_id);

int scmi_notification_notify(
    unsigned int protocol_id,
    unsigned int operation_id,
    unsigned int scmi_response_message_id,
    void *payload_p2a,
    size_t payload_size);

/*!
 * \brief Validate received protocol message.
 *
 * \param protocol_id Protocol identifier.
 * \param service_id Service identifier.
 * \param payload Payload data to write, or NULL if a payload has already
 *         been written.
 * \param payload_size Payload size.
 * \param message_id Message identifier.
 * \param payload_size_table Expected size of payload per message ID
 * \param command_count total number of commands per protocol
 * \param handler_table pointer to message handler
 */
int mod_scmi_from_protocol_api_scmi_frame_validation(
    uint8_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    size_t message_id,
    const size_t *payload_size_table,
    size_t command_count,
    const handler_table_t *handler_table);

#endif /* MOD_SCMI_EXTRA_H_ */

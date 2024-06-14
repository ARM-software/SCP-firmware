/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Power Domain unit test support.
 */

#ifndef MOD_SCMI_POWER_DOMAIN_EXTRA_H
#define MOD_SCMI_POWER_DOMAIN_EXTRA_H

#include "mod_power_domain.h"

#include <mod_scmi.h>

#include <fwk_id.h>

int get_domain_type(fwk_id_t pd_id, enum mod_pd_type *type);

int get_domain_parent_id(fwk_id_t pd_id, fwk_id_t *parent_pd_id);

int set_state(fwk_id_t pd_id, bool resp_requested, uint32_t state);

int get_state(fwk_id_t pd_id, unsigned int *state);

int reset(fwk_id_t pd_id, bool resp_requested);

int system_suspend(unsigned int state);

int system_shutdown(enum mod_pd_system_shutdown system_shsutdown);

int mod_scmi_from_protocol_api_get_agent_count(unsigned int *agent_count);

int mod_scmi_from_protocol_api_get_agent_id(
    fwk_id_t service_id,
    unsigned int *agent_id);

int mod_scmi_from_protocol_api_get_agent_type(
    uint32_t agent_id,
    enum scmi_agent_type *agent_type);

int mod_scmi_from_protocol_api_get_max_payload_size(
    fwk_id_t service_id,
    size_t *size);

int mod_scmi_from_protocol_api_write_payload(
    fwk_id_t service_id,
    size_t offset,
    const void *payload,
    size_t size);

int mod_scmi_from_protocol_api_respond(
    fwk_id_t service_id,
    const void *payload,
    size_t size);

void mod_scmi_from_protocol_api_notify(
    fwk_id_t service_id,
    int protocol_id,
    int message_id,
    const void *payload,
    size_t size);

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

#endif /* MOD_SCMI_POWER_DOMAIN_EXTRA_H */

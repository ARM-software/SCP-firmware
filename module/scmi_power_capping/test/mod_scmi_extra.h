/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SCMI_EXTRA_H_
#define MOD_SCMI_EXTRA_H_

#include "fwk_id.h"

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

#endif /* MOD_SCMI_EXTRA_H_ */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Power Requester unit test support.
 */
#include <mod_scmi_system_power_req.h>

/*! Send an SCMI message */
int scmi_send_message(
    uint8_t scmi_message_id,
    uint8_t scmi_protocol_id,
    uint8_t token,
    fwk_id_t service_id,
    const void *payload,
    size_t payload_size,
    bool request_ack_by_interrupt);

/*! Handle response SCMI message */
int response_message_handler(fwk_id_t service_id);

/*
 * used as a mock for message handlers
 */
int fake_message_handler(
    fwk_id_t service_id,
    const void *payload,
    size_t payload_size);

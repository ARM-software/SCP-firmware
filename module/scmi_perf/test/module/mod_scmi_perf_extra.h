/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) unit test support.
 */
#include <mod_dvfs.h>
#include <mod_scmi.h>
#include <mod_scmi_perf.h>

/*!
 * \brief Send an SCMI message
 *
 * \param scmi_message_id SCMI message identifier.
 * \param scmi_protocol_id SCMI message protocol identifier.
 * \param token SCMI message token.
 * \param service_id SCMI service identifier.
 * \param payload Payload data to write
 * \param payload_size size of the payload in bytes.
 * \param request_ack_by_interrupt flag to select whether acknowledgement
 * interrupt is required for this message.
 */
int mod_scmi_from_protocol_api_scmi_send_message(
    uint8_t scmi_message_id,
    uint8_t scmi_protocol_id,
    uint8_t token,
    fwk_id_t service_id,
    const void *payload,
    size_t payload_size,
    bool request_ack_by_interrupt);

/*!
 * \brief Handle response SCMI message
 *
 * \param service_id Service identifier.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 */
int mod_scmi_from_protocol_api_response_message_handler(fwk_id_t service_id);

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
    unsigned int message_id,
    const unsigned int *payload_size_table,
    unsigned int command_count,
    const handler_table_t *handler_table);

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) unit test support.
 */
#include <mod_scmi_sensor_req.h>
#include <mod_sensor.h>

/*!
 * \brief Inform the completion of a sensor reading.
 *
 * \param id Specific sensor device identifier.
 * \param[out] response The response data structure.
 */
void reading_complete(
    fwk_id_t id,
    struct mod_sensor_driver_resp_params *response);

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
int scmi_send_message(
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
int response_message_handler(fwk_id_t service_id);

/*
 * used as a mock for message handlers
 */
int fake_message_handler(
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size);

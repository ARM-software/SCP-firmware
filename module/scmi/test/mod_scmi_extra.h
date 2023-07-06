/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) unit test support.
 */
#include <mod_scmi.h>

/*!
 * \brief (UNIT Tests) SCMI module to transport entity API
 */

/*!
 * \brief Check whether a channel is secure or non-secure.
 *
 * \param channel_id Channel identifier.
 * \param[out] secure Channel security state. True if the channel
 *      is secure, or false if it is non-secure.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `channel_id` parameter was not a valid system entity
 *        identifier.
 *      - The `secure` parameter was a null pointer value.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int mod_scmi_to_transport_api_get_secure(fwk_id_t channel_id, bool *secure);

/*!
 * \brief Get the maximum permitted payload size of a channel.
 *
 * \param channel_id Channel identifier.
 * \param[out] size Maximum payload size in bytes.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `channel_id` parameter was not a valid system entity
 *        identifier.
 *      - The `size` parameter was a null pointer value.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int mod_scmi_to_transport_api_get_max_payload_size(
    fwk_id_t channel_id,
    size_t *size);

/*!
 * \brief Get the SCMI message header from a channel.
 *
 * \param channel_id Channel identifier.
 * \param[out] message_header SCMI message header.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `channel_id` parameter was not a valid system entity
 *        identifier.
 *      - The `message_header` parameter was a null pointer value.
 * \retval ::FWK_E_ACCESS No message is available to read.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int mod_scmi_to_transport_api_get_message_header(
    fwk_id_t channel_id,
    uint32_t *message_header);

/*!
 * \brief Get the SCMI payload from a channel.
 *
 * \param channel_id Channel identifier.
 * \param[out] payload Pointer to the payload.
 * \param[out] size Payload size. May be NULL, in which case the
 *      parameter should be ignored.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `channel_id` parameter was not a valid system entity
 *        identifier.
 *      - The `payload` parameter was a null pointer value.
 *      - The `size` parameter was a null pointer value.
 * \retval ::FWK_E_ACCESS No message is available to read.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int mod_scmi_to_transport_api_get_payload(
    fwk_id_t channel_id,
    const void **payload,
    size_t *size);

/*!
 * \brief Write part of a payload to a channel.
 *
 * \param channel_id Channel identifier.
 * \param offset Offset to begin writing at.
 * \param payload Payload data to write.
 * \param size Size of the payload data.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `channel_id` parameter was not a valid system entity
 *        identifier.
 *      - The `payload` parameter was a null pointer value.
 *      - The offset and size provided are not within the bounds of the
 *        payload area.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int mod_scmi_to_transport_api_write_payload(
    fwk_id_t channel_id,
    size_t offset,
    const void *payload,
    size_t size);

/*!
 * \brief Respond to an SCMI message on a channel.
 *
 * \param channel_id Channel identifier.
 * \param payload Payload data to write, or NULL if a payload has already
 *      been written.
 * \param size Size of the payload source.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `channel_id` parameter was not a valid system entity
 *        identifier.
 *      - The size given is less than the size of one paylout entry.
 * \retval ::FWK_E_ACCESS No message is available to respond to.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int mod_scmi_to_transport_api_respond(
    fwk_id_t channel_id,
    const void *payload,
    size_t size);

/*!
 * \brief Send a message on a channel.
 *
 * \param channel_id Channel identifier.
 * \param message_header Message ID.
 * \param payload Payload data to write.
 * \param size Size of the payload source.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `channel_id` parameter was not a valid system entity
 *        identifier.
 *      - The size given is less than the size of one paylout entry.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int mod_scmi_to_transport_api_transmit(
    fwk_id_t channel_id,
    uint32_t message_header,
    const void *payload,
    size_t size,
    bool request_ack_by_interrupt);

/*!
 * \brief Release the transport channel context lock.
 *
 * \param channel_id Transport channel identifier.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 */
int mod_scmi_to_transport_api_release_transport_channel_lock(
    fwk_id_t channel_id);

/*!
 * \brief Get the number of active agents.
 *
 * \param[out] agent_count Number of active agents.
 *
 * \retval ::FWK_SUCCESS The agent count was returned.
 * \retval ::FWK_E_PARAM The parameter `agent_count` is equal to `NULL`.
 */
int mod_scmi_from_protocol_get_agent_count(unsigned int *agent_count);

/*!
 * \brief Get the identifier of the agent associated with a service
 *
 * \param service_id Identifier of the service.
 * \param[out] agent_id Agent identifier.
 *
 * \retval ::FWK_SUCCESS The agent identifier was returned.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `service_id` parameter was not a valid system entity
 *        identifier.
 *      - The `agent_id` parameter was a null pointer value.
 * \retval ::FWK_E_INIT The service is not initialized.
 * \retval ::FWK_E_STATE The service is in an invalid state.
 */
int mod_scmi_from_protocol_get_agent_id(
    fwk_id_t service_id,
    unsigned int *agent_id);

/*!
 * \brief Get the type of the agent given its identifier.
 *
 * \details This API can be used by SCMI protocols to check the validity
 *          of an agent identifier.
 *
 * \param agent_id Identifier of the agent.
 * \param[out] agent_type Agent type.
 *
 * \retval ::FWK_SUCCESS The agent identifier was returned.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `agent_id` parameter was not a valid system entity
 *        identifier.
 *      - The `agent_type` parameter was a null pointer value.
 */
int mod_scmi_from_protocol_get_agent_type(
    uint32_t agent_id,
    enum scmi_agent_type *agent_type);

/*!
 * \brief Get the maximum permitted payload size of a channel associated
 *        with a service.
 *
 * \param service_id Service identifier.
 * \param[out] size Maximum payload size in bytes.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `service_id` parameter was not a valid system entity
 *        identifier.
 *      - The `size` parameter was a null pointer value.
 * \retval ::FWK_E_INIT The service is not initialized.
 * \retval ::FWK_E_STATE The service is in an invalid sate.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int mod_scmi_from_protocol_get_max_payload_size(
    fwk_id_t service_id,
    size_t *size);

/*!
 * \brief Write part of a payload through a service.
 *
 * \param service_id Service identifier.
 * \param offset Offset to begin writing at.
 * \param payload Payload data to write.
 * \param size Size of the payload data.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `service_id` parameter was not a valid system entity
 *        identifier.
 *      - The offset and size given were not within the bounds of the
 *        payload area.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int mod_scmi_from_protocol_write_payload(
    fwk_id_t service_id,
    size_t offset,
    const void *payload,
    size_t size);

/*!
 * \brief Respond to an SCMI message on a service.
 *
 * \param service_id Service identifier.
 * \param payload Payload data to write, or NULL if a payload has already
 *      been written.
 * \param size Size of the payload.
 */
int mod_scmi_from_protocol_respond(
    fwk_id_t service_id,
    const void *payload,
    size_t size);

/*!
 * \brief Send a notification to the agent on behalf on an SCMI service.
 *
 * \param service_id Service identifier.
 * \param protocol_id Protocol identifier.
 * \param message_id Message identifier.
 * \param payload Payload data to write, or NULL if a payload has already
 *         been written.
 * \param size Size of the payload in bytes.
 */
void mod_scmi_from_protocol_notify(
    fwk_id_t service_id,
    int protocol_id,
    int message_id,
    const void *payload,
    size_t size);

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
int mod_scmi_from_protocol_send_message(
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
int mod_scmi_from_protocol_response_message_handler(fwk_id_t service_id);

/*!
 * \brief Get the SCMI protocol identifier of the SCMI protocol implemented
 *      by the SCMI protocol module implementing this API.
 *
 * \param protocol_id Identifier of the protocol module.
 * \param scmi_protocol_id SCMI protocol identifier.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 *
 */
int test_get_scmi_protocol_id(fwk_id_t protocol_id, uint8_t *scmi_protocol_id);

int test_mod_scmi_message_handler(
    fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id);

int test_mod_scmi_notification_message_handler(
    fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size);

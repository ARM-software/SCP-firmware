/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) unit test support.
 */
#include <mod_scmi.h>
#include <mod_resource_perms.h>

/*!
 * \brief Get the number of active agents.
 *
 * \param[out] agent_count Number of active agents.
 *
 * \retval ::FWK_SUCCESS The agent count was returned.
 * \retval ::FWK_E_PARAM The parameter `agent_count` is equal to `NULL`.
 */
int mod_scmi_from_protocol_api_get_agent_count(unsigned int *agent_count);

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
int mod_scmi_from_protocol_api_get_agent_id(fwk_id_t service_id, unsigned int *agent_id);

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
int mod_scmi_from_protocol_api_get_agent_type(uint32_t agent_id, enum scmi_agent_type *agent_type);

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
int mod_scmi_from_protocol_api_get_max_payload_size(fwk_id_t service_id, size_t *size);

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
int mod_scmi_from_protocol_api_write_payload(fwk_id_t service_id, size_t offset,
                                             const void *payload, size_t size);

/*!
 * \brief Respond to an SCMI message on a service.
 *
 * \param service_id Service identifier.
 * \param payload Payload data to write, or NULL if a payload has already
 *      been written.
 * \param size Size of the payload.
 */
int mod_scmi_from_protocol_api_respond(fwk_id_t service_id, const void *payload, size_t size);

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
void mod_scmi_from_protocol_api_notify(fwk_id_t service_id, int protocol_id, int message_id,
    const void *payload, size_t size);

/*!
 * \brief Validate received protocol message.
 *
 * \param protocol_id Protocol identifier.
 * \param service_id Service identifier.
 * \param payload Payload data to write, or NULL if a payload has already
 *      been written.
 * \param payload_size Payload size.
 * \param message_id Message ID.
 * \param payload_size_table Expected size of payload per message ID
 * \param command_count total number of commands per protocol
 * \param handler_table pointer to message handler
 *
 * \retval ::FWK_SUCCESS Protocol frame is valid.
 * \retval ::SCMI_INVALID_PARAMETERS Payload is NULL or protocol ID error.
 * \retval ::SCMI_NOT_FOUND Message ID out of range
 * \retval ::SCMI_PROTOCOL_ERROR Payload size pegger than expected payload.
 * \retval ::SCMI_DENIED Agent has no permission for the protocol.
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
 * \brief Check whether the agent has permission to access a protocol.
 *
 * \param agent_id      identifier of the agent.
 * \param protocol_id   identifier of the protocol.
 *
 * \retval MOD_RES_PERMS_ACCESS_ALLOWED The agent has permissions to
 *      use the protocol.
 * \retval MOD_RES_PERMS_ACCESS_DENIED The agent does not have
 *      permissions to use the protocol.
 */
enum mod_res_perms_permissions mod_res_permissions_api_agent_has_protocol_permission(
    uint32_t agent_id,
    uint32_t protocol_id);

/*!
 * \brief Check whether the agent has permission to access a message.
 *
 * \param agent_id      identifier of the agent.
 * \param protocol_id   identifier of the protocol.
 * \param message_id    identifier of the message.
 *
 * \retval MOD_RES_PERMS_ACCESS_ALLOWED The agent has permissions to
 *      use the protocol.
 * \retval MOD_RES_PERMS_ACCESS_DENIED The agent does not have
 *      permissions to use the message.
 */
enum mod_res_perms_permissions mod_res_permissions_api_agent_has_message_permission(
    uint32_t agent_id,
    uint32_t protocol_id,
    uint32_t message_id);

/*!
 * \brief Check whether the agent has permission to access a resource.
 *
 * \param agent_id      identifier of the agent.
 * \param protocol_id   identifier of the protocol.
 * \param message_id    identifier of the message.
 * \param resource_id   identifier of the resource.
 *
 * \retval MOD_RES_PERMS_ACCESS_ALLOWED The agent has permissions to
 *      use the protocol.
 * \retval MOD_RES_PERMS_ACCESS_DENIED The agent does not have
 *      permissions to use the resource.
 */
enum mod_res_perms_permissions mod_res_permissions_api_agent_has_resource_permission(
    uint32_t agent_id,
    uint32_t protocol_id,
    uint32_t message_id,
    uint32_t resource_id);

/*!
 * \brief Set device permissions for an agent
 *
 * \param agent_id      identifier of the agent.
 * \param device_id     identifier of the device.
 * \param flags         permissions to set.
 *
 * \retval ::FWK_SUCCESS  The operation has completed successfully.
 * \retval ::FWK_E_ACCESS Unknown agent_id or device_id.
 * \retval ::FWK_E_PARAM  Invalid flags or protocol_ID.
 */
int mod_res_permissions_api_agent_set_device_permission(
    uint32_t agent_id,
    uint32_t device_id,
    uint32_t flags);

/*!
 * \brief Set device protocol permissions for an agent
 *
 * \param agent_id      identifier of the agent.
 * \param device_id     identifier of the device.
 * \param device_id     identifier of the protocol.
 * \param flags         permissions to set.
 *
 * \retval ::FWK_SUCCESS  The operation has completed successfully.
 * \retval ::FWK_E_ACCESS Unknown agent_id or device_id.
 * \retval ::FWK_E_PARAM  Invalid flags or protocol_ID.
 */
int mod_res_permissions_api_agent_set_device_protocol_permission(
    uint32_t agent_id,
    uint32_t device_id,
    uint32_t protocol_id,
    uint32_t flags);

/*!
 * \brief Reset permissions for an agent
 *
 * \param agent_id      identifier of the agent.
 * \param flags         permissions to set.
 *
 * \retval ::FWK_SUCCESS  The operation has completed successfully.
 * \retval ::FWK_E_ACCESS Unknown agent_id.
 * \retval ::FWK_E_PARAM  Invalid flags.
 */
int mod_res_permissions_api_agent_reset_config(uint32_t agent_id, uint32_t flags);

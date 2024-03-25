/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) support.
 */

#ifndef MOD_SCMI_H
#define MOD_SCMI_H

#include <internal/scmi.h>
#include <mod_scmi_std.h>

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * \{
 */

/*!
 * \defgroup GroupSCMI System Control & Management Interface (SCMI)
 * \{
 */

/*!
 * \brief Index of the interfaces exposed by the SCMI module.
 */
enum mod_scmi_api_idx {
    MOD_SCMI_API_IDX_PROTOCOL,
    MOD_SCMI_API_IDX_PROTOCOL_REQ,
    MOD_SCMI_API_IDX_TRANSPORT,
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    MOD_SCMI_API_IDX_NOTIFICATION,
#endif
    MOD_SCMI_API_IDX_COUNT,
};

/*!
 * \brief Message handler table perprotocol.
 */
typedef int (*handler_table_t)(fwk_id_t, const uint32_t *);

/*!
 * \brief Entity role.
 */
enum mod_scmi_entity_role { MOD_SCMI_ROLE_PLATFORM, MOD_SCMI_ROLE_AGENT };

/*!
 * \brief Agent descriptor
 */
struct mod_scmi_agent {
    /*! \brief Type of the agent. */
    enum scmi_agent_type type;

    /*!
     *  \brief Pointer to the agent's name. This pointer may be equal to NULL.
     *       In that case, the agent will be assigned a default name based on
     *       its type: "PSCI", "MANAGEMENT", "OSPM", or "OTHER".
     *
     *  \details The agent name of the BASE_DISCOVER_AGENT command can be at
     *       most 16 bytes in length (null terminator included). If the value
     *       assigned to this variable is longer than the limit then the agents
     *       in the system will be provided with a truncated version of it.
     */
    const char *name;
};

/*!
 * \brief SCMI module configuration data.
 */
struct mod_scmi_config {
    /*!
     *  \brief Maximum number of SCMI protocol modules that can bind to the
     *       SCMI module.
     */
    unsigned int protocol_count_max;

    /*!
     *  \brief Maximum number of SCMI protocol requester modules that can
     *       bind to the SCMI module.
     */
    unsigned int protocol_requester_count_max;

#ifndef BUILD_HAS_MOD_RESOURCE_PERMS
    /*!
     *  \brief Number of the disabled SCMI protocols for the PSCI agent.
     *         If set to zero then all protocols are allowed for a PSCI agent.
     */
    unsigned int dis_protocol_count_psci;

    /*!
     *  \brief list protocols disabled for the PSCI agent , limited
     *       by protocol_count_psci
     */
    const uint32_t *dis_protocol_list_psci;
#endif

    /*!
     *  \brief Number of agents in the system. Must be smaller than or equal to
     *       SCMI_AGENT_ID_MAX.
     */
    unsigned int agent_count;

    /*!
     *  \brief Table of descriptors of the agents in the system.
     *
     *  \details Entry 'i' in the table contains the descriptor of the agent
     *       with identifier 'i'. The agent identifiers are sequential and start
     *       from one, with the identifier 0 being reserved to identify the
     *       platform. The table must therefore have space for 'agent_count + 1'
     *       entries. The first entry is ignored.
     */
    const struct mod_scmi_agent *agent_table;

    /*!
     *  \brief Pointer to the vendor identifier.
     *
     *  \note The vendor identifier in the BASE_DISCOVER_VENDOR command is
     *       up to 16 bytes in length (null terminator included). The vendor
     *       identifier provided to agents will be truncated if it exceeds
     *       this limit.
     */
    const char *vendor_identifier;

    /*!
     *  \brief Pointer to the sub-vendor identifier.
     *
     *  \note The sub-vendor identifier in the BASE_DISCOVER_SUB_VENDOR
     *       command is up to 16 bytes in length (null terminator included).
     *       The sub-vendor identifier provided to agents will be truncated
     *       if it exceeds this limit.
     */
    const char *sub_vendor_identifier;
};

/*!
 * \brief Service configuration data.
 */
struct mod_scmi_service_config {
    /*!
     * \brief Identifier of the transport entity.
     */
    fwk_id_t transport_id;

    /*!
     * \brief Identifier of the API of the transport entity.
     */
    fwk_id_t transport_api_id;

    /*!
     * \brief Identifier of the notification indicating the transport has been
     *      initialized.
     */
    fwk_id_t transport_notification_init_id;

    /*!
     *  \brief Identifier of the agent.
     *
     *  \details An SCMI channel is the communication channel between an agent
     *        and the platform. This is the identifier assigned in the system
     *        to the agent using the transport channel identified by
     *        'scmi_channel_id'. The agent identifier must be greater than or
     *        equal to one (the identifier 0 is assigned to the platform) and
     *        lower than or equal to the number of agents declared in SCMI
     *        module configuration data.
     */
    unsigned int scmi_agent_id;

    /*!
     *  \brief Identifier of SCMI P2A service asssociated with this A2P service.
     *
     *  \details If a request for notifications is received on this service
     *        channel the notifications will be sent on the channel identified
     *        here.
     */
    fwk_id_t scmi_p2a_id;

    /*!
     * \brief Entity role.
     *
     * \details Determine if this entity is an agent or a platform.
     */
    enum mod_scmi_entity_role scmi_entity_role;
};

/*!
 * \brief SCMI module to transport entity API.
 */
struct mod_scmi_to_transport_api {
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
    int (*get_secure)(fwk_id_t channel_id, bool *secure);

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
    int (*get_max_payload_size)(fwk_id_t channel_id, size_t *size);

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
    int (*get_message_header)(fwk_id_t channel_id, uint32_t *message_header);

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
    int (*get_payload)(fwk_id_t channel_id, const void **payload,
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
    int (*write_payload)(fwk_id_t channel_id, size_t offset,
                         const void *payload, size_t size);

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
    int (*respond)(fwk_id_t channel_id, const void *payload, size_t size);

    /*!
     * \brief Send a message on a channel.
     *
     * \param channel_id Channel identifier.
     * \param message_header Message ID.
     * \param payload Payload data to write.
     * \param size Size of the payload source.
     * \param request_ack_by_interrupt flag to select whether acknowledgement
     * interrupt is required for this message.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered:
     *      - The `channel_id` parameter was not a valid system entity
     *        identifier.
     *      - The size given is less than the size of one paylout entry.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*transmit)(
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
    int (*release_transport_channel_lock)(fwk_id_t channel_id);
};

/*!
 * \brief Transport entity API to SCMI module API.
 */
struct mod_scmi_from_transport_api {
    /*!
     * \brief Signal to a SCMI service that a incoming message for it has
     *      incorrect length and payload size and so the incoming message has
     *      been dropped.
     *
     * \note Subscribed SCMI service should call the respond API to free the
     *       channel.
     *
     * \param service_id service identifier.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM The service_id parameter is invalid.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*signal_error)(fwk_id_t service_id);

    /*!
     * \brief Signal to a service that a message is incoming.
     *
     * \param service_id SCMI service identifier.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM The `service_id` parameter was not a valid system
     *      entity identifier.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*signal_message)(fwk_id_t service_id);
};

/*!
 * \brief SCMI protocol message handler prototype.
 *
 * \details Prototype of a message handler called by the SCMI module when it
 *      receives a message for a SCMI protocol module.
 *
 * \note A return value of FWK_SUCCESS indicates only that no internal error
 *      was encountered, not that the SCMI command has returned a successful
 *      result to the SCMI agent. In the case where the return value indicates
 *      an internal failure, the SCMI command is expected to return the status
 *      code SCMI_GENERIC_ERROR per the specification.
 *
 * \param protocol_id Identifier of the protocol module.
 * \param service_id Identifer of the SCMI service which received the message.
 * \param payload Pointer to the message payload.
 * \param payload_size Size in number of bytes of the message payload.
 * \param message_id Identifier of the message to be handled by the protocol
 *      handler.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \return One of the standard error codes for implementation-defined errors.
 *
 */
typedef int mod_scmi_message_handler_t(fwk_id_t protocol_id,
    fwk_id_t service_id, const uint32_t *payload, size_t payload_size,
    unsigned int message_id);

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
/*!
 * \brief SCMI protocol notification message handler prototype.
 *
 * \details Prototype of a notification message handler called
 *         by the SCMI module when it receives a notification
 *         for a SCMI protocol module.
 *
 * \note A return value of FWK_SUCCESS indicates only that no internal error
 *      was encountered, not that the SCMI command has returned a successful
 *      result to the SCMI agent. In the case where the return value indicates
 *      an internal failure, the SCMI command is expected to return the status
 *      code SCMI_GENERIC_ERROR per the specification.
 *
 * \param protocol_id Identifier of the protocol module.
 * \param service_id Identifer of the SCMI service which received the message.
 * \param payload Pointer to the notification payload.
 * \param payload_size Size in number of bytes of the notification payload.
 * \param message_id Identifier of the message to be handled by the protocol
 *      handler.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \return One of the standard error codes for implementation-defined errors.
 *
 */
typedef int mod_scmi_notification_message_handler_t(
    fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id);
#endif
/*!
 * \brief SCMI module to SCMI protocol module API.
 */
struct mod_scmi_to_protocol_api {
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
    int (*get_scmi_protocol_id)(fwk_id_t protocol_id,
                                uint8_t *scmi_protocol_id);

    /*! Protocol message handler. */
    mod_scmi_message_handler_t *message_handler;

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    /*! Protocol notification handler. */
    mod_scmi_notification_message_handler_t *notification_handler;
#endif
};

#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
/*!
 * \brief SCMI protocol SCMI module SCMI notification API.
 */
struct mod_scmi_notification_api {
    /*!
     * \brief Initialize notification context for a protocol.
     *
     * \param protocol_id Identifier of the protocol.
     * \param agent_count Number of agents supported by the protocol.
     * \param element_count Number of elements that support notification.
     * \param operation_count Number of notification SCMI messages supported.
     *
     * \retval ::FWK_SUCCESS Initialization successful.
     * \retval One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*scmi_notification_init)(
        unsigned int protocol_id,
        unsigned int agent_count,
        unsigned int element_count,
        unsigned int operation_count);

    /*!
     * \brief Add an agent to subscriber list that requested a notification.
     *
     * \param protocol_id Identifier of the protocol.
     * \param element_idx Index of the element within specified protocol
     *     context.
     * \param operation_id Identifier of the operation.
     * \param service_id  Identifier of the agent's SCMI service context.
     *
     * \retval ::FWK_SUCCESS Adding of subscriber agent to the list is
     *      successful.
     * \retval One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*scmi_notification_add_subscriber)(
        unsigned int protocol_id,
        unsigned int element_idx,
        unsigned int operation_id,
        fwk_id_t service_id);

    /*!
     * \brief Remove an agent from subscriber list.
     *
     * \param protocol_id Identifier of the protocol.
     * \param agent_idx Index of the agent within specified protocol context.
     * \param element_idx Index of the element within specified protocol
     *     context.
     * \param operation_id Identifier of the operation.
     *
     * \retval ::FWK_SUCCESS Removing of subscriber agent from the list is
     *     successful.
     * \retval One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*scmi_notification_remove_subscriber)(
        unsigned int protocol_id,
        unsigned int agent_idx,
        unsigned int element_idx,
        unsigned int operation_id);

    /*!
     * \brief Notifiy all agents which requested a specific notification.
     *
     * \param protocol_id Identifier of the protocol.
     * \param operation_id Identifier of the operation.
     * \param scmi_response_message_id SCMI message identifier that is sent as
     *     as a part of the notification.
     * \param payload_p2a Notification message payload from platform to
     *     agent.
     * \param payload_size Size of the message.
     *
     * \retval ::FWK_SUCCESS Notification to agents is successful.
     * \retval One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*scmi_notification_notify)(
        unsigned int protocol_id,
        unsigned int operation_id,
        unsigned int scmi_response_message_id,
        void *payload_p2a,
        size_t payload_size);
};
#endif

/*!
 * \brief SCMI protocol module to SCMI module API.
 */
struct mod_scmi_from_protocol_api {
    /*!
     * \brief Get the number of active agents.
     *
     * \param[out] agent_count Number of active agents.
     *
     * \retval ::FWK_SUCCESS The agent count was returned.
     * \retval ::FWK_E_PARAM The parameter `agent_count` is equal to `NULL`.
     */
    int (*get_agent_count)(unsigned int *agent_count);

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
    int (*get_agent_id)(fwk_id_t service_id, unsigned int *agent_id);

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
    int (*get_agent_type)(uint32_t agent_id, enum scmi_agent_type *agent_type);

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
    int (*get_max_payload_size)(fwk_id_t service_id, size_t *size);

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
    int (*write_payload)(fwk_id_t service_id, size_t offset,
                         const void *payload, size_t size);

    /*!
     * \brief Respond to an SCMI message on a service.
     *
     * \param service_id Service identifier.
     * \param payload Payload data to write, or NULL if a payload has already
     *      been written.
     * \param size Size of the payload.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_SUPPORT Transport-specific message not supported.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*respond)(fwk_id_t service_id, const void *payload, size_t size);

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
    int (*scmi_message_validation)(
        uint8_t protocol_id,
        fwk_id_t service_id,
        const uint32_t *payload,
        size_t payload_size,
        size_t message_id,
        const size_t *payload_size_table,
        size_t command_count,
        const handler_table_t *handler_table);

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
    void (*notify)(fwk_id_t service_id, int protocol_id, int message_id,
        const void *payload, size_t size);
};

/*!
 * \brief SCMI protocol requester module to SCMI module API.
 */
struct mod_scmi_from_protocol_req_api {
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
    int (*scmi_send_message)(
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
    int (*response_message_handler)(fwk_id_t service_id);
};

/*!
 * \brief SCMI notification indices.
 */
enum mod_scmi_notification_idx {
    /*! The SCMI service has been initialized */
    MOD_SCMI_NOTIFICATION_IDX_INITIALIZED,

    /*! Number of defined notifications */
    MOD_SCMI_NOTIFICATION_IDX_COUNT
};

/*!
 * \brief Identifier for the MOD_SCMI_NOTIFICATION_IDX_INITIALIZED
 *     notification.
 */
static const fwk_id_t mod_scmi_notification_id_initialized =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_SCMI,
        MOD_SCMI_NOTIFICATION_IDX_INITIALIZED);

/*!
 * \}
 */

/*!
 * \}
 */

#endif /* MOD_SCMI_H */

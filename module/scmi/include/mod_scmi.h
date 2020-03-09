/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) support.
 */

#ifndef MOD_SCMI_H
#define MOD_SCMI_H

#include <internal/scmi.h>

#include <fwk_id.h>
#include <fwk_module_idx.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \addtogroup GroupModules Modules
 * @{
 */

/*!
 * \defgroup GroupSCMI System Control & Management Interface (SCMI)
 * @{
 */

/*!
 * \brief Index of the interfaces exposed by the SCMI module.
 */
enum mod_scmi_api_idx {
    MOD_SCMI_API_IDX_PROTOCOL,
    MOD_SCMI_API_IDX_TRANSPORT,
    MOD_SCMI_API_IDX_COUNT,
};

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
     * is secure, or false if it is non-secure.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM The channel_id parameter is invalid.
     * \retval FWK_E_PARAM The secure parameter is NULL.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*get_secure)(fwk_id_t channel_id, bool *secure);

    /*!
     * \brief Get the maximum permitted payload size of a channel.
     *
     * \param channel_id Channel identifier.
     * \param[out] size Maximum payload size in bytes.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM The channel_id parameter is invalid.
     * \retval FWK_E_PARAM The size parameter is NULL.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*get_max_payload_size)(fwk_id_t channel_id, size_t *size);

    /*!
     * \brief Get the SCMI message header from a channel.
     *
     * \param channel_id Channel identifier.
     * \param[out] message_header SCMI message header.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM The channel_id parameter is invalid.
     * \retval FWK_E_PARAM The message_header parameter is NULL.
     * \retval FWK_E_ACCESS No message is available to read.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*get_message_header)(fwk_id_t channel_id, uint32_t *message_header);

    /*!
     * \brief Get the SCMI payload from a channel.
     *
     * \param channel_id Channel identifier.
     * \param[out] payload Pointer to the payload.
     * \param[out] size Payload size. May be NULL, in which case the
     * parameter should be ignored.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM The channel_id parameter is invalid.
     * \retval FWK_E_PARAM The payload parameter is NULL.
     * \retval FWK_E_ACCESS No message is available to read.
     * \return One of the standard error codes for implementation-defined
     * errors.
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
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM The payload parameter is NULL.
     * \retval FWK_E_PARAM The offset and size provided are not within the
     * bounds of the payload area.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*write_payload)(fwk_id_t channel_id, size_t offset,
                         const void *payload, size_t size);

    /*!
     * \brief Respond to an SCMI message on a channel.
     *
     * \param channel_id Channel identifier.
     * \param payload Payload data to write, or NULL if a payload has already
     * been written.
     * \param size Size of the payload source.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM The channel_id parameter is invalid.
     * \retval FWK_E_PARAM The size parameter is less than the size of one
     * payload entry.
     * \retval FWK_E_ACCESS No message is available to respond to.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*respond)(fwk_id_t channel_id, const void *payload, size_t size);
};

/*!
 * \brief Transport entity API to SCMI module API.
 */
struct mod_scmi_from_transport_api {
    /*!
     * \brief Signal to a service that a message is incoming.
     *
     * \param service_id SCMI service identifier.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM The channel_id parameter is invalid.
     * \retval FWK_E_PARAM The secure parameter is NULL.
     * \return One of the standard error codes for implementation-defined
     * errors.
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
 * \retval FWK_SUCCESS The operation succeeded.
 * \return One of the standard error codes for implementation-defined errors.
 *
 */
typedef int mod_scmi_message_handler_t(fwk_id_t protocol_id,
    fwk_id_t service_id, const uint32_t *payload, size_t payload_size,
    unsigned int message_id);

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
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     *
     */
    int (*get_scmi_protocol_id)(fwk_id_t protocol_id,
                                uint8_t *scmi_protocol_id);

    /*! Protocol message handler. */
    mod_scmi_message_handler_t *message_handler;
};

/*!
 * \brief SCMI protocol module to SCMI module API.
 */
struct mod_scmi_from_protocol_api {
    /*!
     * \brief Get the identifier of the agent associated with a service
     *
     * \param service_id Identifier of the service.
     * \param[out] agent_id Agent identifier.
     *
     * \retval FWK_SUCCESS The agent identifier was returned.
     * \retval FWK_E_PARAM The service identifier is not valid.
     * \retval FWK_E_PARAM The parameter 'agent_id' is equal to NULL.
     * \retval FWK_E_INIT The service is not initialized.
     * \retval FWK_E_STATE The service is in an invalid state.
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
     * \retval FWK_SUCCESS The agent identifier was returned.
     * \retval FWK_E_PARAM The agent identifier is not valid.
     * \retval FWK_E_PARAM The parameter 'agent_type' is equal to NULL.
     */
     int (*get_agent_type)(uint32_t agent_id,
                           enum scmi_agent_type *agent_type);

    /*!
     * \brief Get the maximum permitted payload size of a channel associated
     *        with a service.
     *
     * \param service_id Service identifier.
     * \param[out] size Maximum payload size in bytes.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM The service_id parameter is invalid.
     * \retval FWK_E_PARAM The size parameter is NULL.
     * \retval FWK_E_INIT The service is not initialized.
     * \retval FWK_E_STATE The service is in an invalid sate.
     * \return One of the standard error codes for implementation-defined
     * errors.
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
     * \retval FWK_SUCCESS The operation succeeded.
     * \retval FWK_E_PARAM The payload parameter is NULL.
     * \retval FWK_E_PARAM The offset and size provided are not within the
     *      bounds of the payload area.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*write_payload)(fwk_id_t service_id, size_t offset,
                         const void *payload, size_t size);

    /*!
     * \brief Respond to an SCMI message on a service.
     *
     * \param service_id Service identifier.
     * \param payload Payload data to write, or NULL if a payload has already
     * been written.
     * \param size Size of the payload.
     */
    void (*respond)(fwk_id_t service_id, const void *payload, size_t size);
};


/*!
 * \brief Identify if an SCMI entity is the communications master for a given
 * channel type.
 *
 * \param type Channel type.
 * \param role Entity role.
 *
 * \retval true The entity is the communications master of the channel.
 * \retval false The entity is the communications slave of the channel.
 */
static inline bool mod_scmi_is_master(enum scmi_channel_type type,
                                      enum scmi_role role)
{
    return (int)type == (int)role;
}

/*!
 * \brief Identify if an SCMI entity is the communications slave for a given
 * channel type.
 *
 * \param type Channel type.
 * \param role Entity role.
 *
 * \retval true The entity is the communications slave of the channel.
 * \retval false The entity is the communications master of the channel.
 */
static inline bool mod_scmi_is_slave(enum scmi_channel_type type,
                                     enum scmi_role role)
{
    return (int)type != (int)role;
}

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
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_SCMI_H */

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_SMT_H
#define MOD_SMT_H

#include <fwk_id.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
 * \name Channel policies
 *
 * \details These policies define attributes that affect how the channel is
 * treated by the SMT component.
 *
 * @{
 */

/*! No policies */
#define MOD_SMT_POLICY_NONE         ((uint32_t)0)

/*! This channel is secure */
#define MOD_SMT_POLICY_SECURE       ((uint32_t)(1 << 0))

/*! The mailbox for this channel requires initialization */
#define MOD_SMT_POLICY_INIT_MAILBOX ((uint32_t)(1 << 1))

/*!
 * @}
 */

/*!
 * \brief Channel type
 *
 * \details Defines the role of an entity in a channel
 */
enum mod_smt_channel_type {
    /*! Master channel */
    MOD_SMT_CHANNEL_TYPE_MASTER,

    /*! Slave channel */
    MOD_SMT_CHANNEL_TYPE_SLAVE,

    /*! Channel type count */
    MOD_SMT_CHANNEL_TYPE_COUNT,
};

/*!
 * \brief Channel config.
 */
struct mod_smt_channel_config {
    /*! Channel role (slave or master) */
    enum mod_smt_channel_type type;

    /*! Channel policies */
    uint32_t policies;

    /*! Shared mailbox address */
    uintptr_t mailbox_address;

    /*! Shared mailbox size in bytes */
    size_t mailbox_size;

    /*! Identifier of the driver */
    fwk_id_t driver_id;

    /*! Identifier of the driver API to bind to */
    fwk_id_t driver_api_id;

    /*! Identifier of the power domain that this channel depends on */
    fwk_id_t pd_source_id;
};

/*!
 * \brief SMT command configuration (used by SCMI agent).
 */
struct mod_smt_command_config {
    /*! SCMI protocol ID of the command */
    unsigned int protocol_id;

    /*! SCMI message ID of the command */
    unsigned int message_id;

    /*! Pointer to SCMI payload data */
    void *payload;

    /*! SCMI payload size in bytes */
    size_t size;
};

/*!
 * \brief Driver API
 */
struct mod_smt_driver_api {
    /*!
     * \brief Raise an interrupt on the receiver
     *
     * \param device_id Device identifier
     *
     * \retval FWK_SUCCESS The operation succeeded
     * \retval FWK_E_PARAM The device_id parameter is invalid
     * \return One of the standard error codes for implementation-defined
     * errors
     */
    int (*raise_interrupt)(fwk_id_t device_id);
};

/*!
 * \brief Driver input API (Implemented by SMT)
 *
 * \details Interface used for driver -> SMT communication.
 */
struct mod_smt_driver_input_api {
    /*!
     * \brief Signal an incoming message in the mailbox
     *
     * \param device_id Channel identifier
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*signal_message)(fwk_id_t channel_id);
};

/*!
 * \brief SCMI module to transport entity API.
 */
struct mod_scmi_to_transport_api {
    /*!
     * \brief Check whether a channel is secure or non-secure.
     *
     * \param channel_id Channel identifier.
     * \param[out] secure The channel security state. True
     * if the channel is secure, or false if it is non-secure.
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
     * \param[out] size The maximum payload size in bytes.
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
     * \param[out] message_header The SCMI message header.
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
     * \param[out] payload The pointer to the payload.
     * \param[out] size The payload size. May be NULL, in which case the
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
 * \brief SCMI Agent to Transport API
 *
 * \details Interface used for SCMI Agent -> SMT communication.
 */
struct mod_scmi_agent_to_transport_api {
    /*!
     * \brief Check if a channel is free to use or not.
     *
     * \param channel_id Channel identifier
     *
     * \retval true The channel is free and the mailbox can be written to.
     * \return false The channel is busy and the mailbox cannot be written.
     */
    bool (*is_channel_free)(fwk_id_t channel_id);

    /*!
     * \brief Send an SCMI message to platform
     *
     * \param channel_id Channel identifier
     * \param cmd Pointer to SMT command configuration
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*send)(fwk_id_t channel_id, struct mod_smt_command_config *cmd);

    /*!
     * \brief Get the SCMI payload from a channel.
     *
     * \param channel_id Channel identifier.
     * \param[out] payload The pointer to the payload.
     * \param[out] size The payload size. May be NULL, in which case the
     * parameter should be ignored.
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*get_payload)(fwk_id_t channel_id, const void **payload,
                       size_t *size);

    /*!
     * \brief Release the SMT channel
     *
     * \param channel_id Channel identifier
     *
     * \retval FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     * errors.
     */
    int (*put_channel)(fwk_id_t channel_id);
};

/*!
 * \brief Type of the interfaces exposed by the power domain module.
 */
enum mod_smt_api_idx {
    MOD_SMT_API_IDX_DRIVER_INPUT,
    MOD_SMT_API_IDX_SCMI_PLATFORM_TRANSPORT,
    MOD_SMT_API_IDX_SCMI_AGENT_TRANSPORT,
    MOD_SMT_API_IDX_COUNT,
};

#endif /* MOD_SMT_H */

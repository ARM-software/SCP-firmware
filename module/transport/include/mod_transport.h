/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Transport HAL module
 */

#ifndef MOD_TRANSPORT_H
#define MOD_TRANSPORT_H

#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_event.h>
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
 * \defgroup GroupTransport Transport
 *
 * \brief HAL Module used to provide transport for inter-processor messages
 *
 *
 * \details This module can bind to different transport driver modules like i2c,
 * mhu2, mhu3, etc and provides a transport agnostic HAL for the  other modules
 * to send and receive messages.
 * @{
 */

/*!
 * \brief structure used for sending & receiving messages
 */
struct mod_transport_buffer {
    /*! Reserved field, must be zero */
    uint32_t reserved0;
    /*! Channel status */
    volatile uint32_t status;
    /*! Implementation defined field */
    uint64_t reserved1;
    /*! Channel flags */
    uint32_t flags;
    /*! Length in bytes of the message header and payload */
    volatile uint32_t length;
    /*! Message header field */
    uint32_t message_header;
    /*! Message payload */
    uint32_t payload[];
};

/*! Interrupt mode enable flag position */
#define MOD_TRANSPORT_FLAGS_IENABLED_POS 0
/*! Interrupt mode enable bit mask */
#define MOD_TRANSPORT_FLAGS_IENABLED_MASK \
    (UINT32_C(0x1) << MOD_TRANSPORT_FLAGS_IENABLED_POS)

/*!
 * \name Channel policies
 *
 * \details These policies define attributes that affect how the channel is
 *      treated
 *
 * @{
 */

/*! No policies */
#define MOD_TRANSPORT_POLICY_NONE ((uint32_t)0)

/*! This channel is secure */
#define MOD_TRANSPORT_POLICY_SECURE ((uint32_t)(1U << 0))

/*!
 * The mailbox for this channel requires initialization. Only relevant for
 * out-band type transport channels.
 */
#define MOD_TRANSPORT_POLICY_INIT_MAILBOX ((uint32_t)(1U << 1))

/*!
 * @}
 */

/*!
 * \brief Channel type
 *
 * \details Defines the role of an entity in a channel
 */
enum mod_transport_channel_type {
    /*! Requester channel */
    MOD_TRANSPORT_CHANNEL_TYPE_REQUESTER,

    /*! Completer channel */
    MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,

    /*! Channel type count */
    MOD_TRANSPORT_CHANNEL_TYPE_COUNT,
};

/*!
 * \brief Channel transport type
 *
 * \details Defines the type of transport used by the channel
 */
enum mod_transport_channel_transport_type {
    /*! Out-band transport - SMT */
    MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_OUT_BAND,

#ifdef BUILD_HAS_INBAND_MSG_SUPPORT
    /*! In-band transport - MHUv2, MHUv3, I2C, etc*/
    MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_IN_BAND,
#endif

    /*! Channel transport type count */
    MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_COUNT,
};

/*!
 * \brief Channel config.
 */
struct mod_transport_channel_config {
    /*! Channel transport type (In-band, Out-band, etc.) */
    enum mod_transport_channel_transport_type transport_type;

    /*! Channel role (requester or completer) */
    enum mod_transport_channel_type channel_type;

    /*!
     * Out-band shared mailbox address. Only relevant for out-band
     * transport type.
     */
    uintptr_t out_band_mailbox_address;

    /*!
     * Out-band shared mailbox size in bytes. Only relevant for out-band
     * transport type.
     */
    size_t out_band_mailbox_size;

    /*!
     * Internal read & write mailbox size in bytes. Only relevant for
     * in-band transport type.
     */
    size_t in_band_mailbox_size;

    /*!
     * Identifier of the power domain that this channel depends on.
     * Applicable for out-band transport channels only.
     */
    fwk_id_t pd_source_id;

    /*! Channel policies */
    uint32_t policies;

    /*! Identifier of the driver */
    fwk_id_t driver_id;

    /*! Identifier of the driver API to bind to */
    fwk_id_t driver_api_id;
};

/*!
 * \brief Driver API (Implemented by the driver)
 *
 * \details Interface used for Transport -> driver communication
 */

struct mod_transport_driver_api {
#ifdef BUILD_HAS_INBAND_MSG_SUPPORT
    /*!
     * \brief Send in-band message using the driver
     *
     * \param message Pointer to the message struct
     * \param device_id Device identifier
     *
     * \retval ::FWK_SUCCESS The operation succeeded
     * \return One of the standard error codes for implementation-defined
     *      errors
     */
    int (*send_message)(
        struct mod_transport_buffer *message,
        fwk_id_t device_id);

    /*!
     * \brief Retrieve the in-band message from the driver
     *
     * \param[out] message Pointer to the message struct
     * \param device_id Device identifier
     *
     * \retval ::FWK_SUCCESS The operation succeeded
     * \return One of the standard error codes for implementation-defined
     *      errors
     */
    int (
        *get_message)(struct mod_transport_buffer *message, fwk_id_t device_id);
#endif

    /*!
     * \brief Raise an interrupt on the receiver
     *
     * \param device_id Device identifier
     *
     * \retval ::FWK_SUCCESS The operation succeeded
     * \return One of the standard error codes for implementation-defined
     *      errors
     */
    int (*trigger_event)(fwk_id_t device_id);
};

/*!
 * \brief Driver input API (Implemented by the transport module)
 *
 * \details Interface used for driver -> Transport communication.
 */
struct mod_transport_driver_input_api {
    /*!
     * \brief Signal an incoming message
     *
     * \param channel_id Channel identifier
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*signal_message)(fwk_id_t channel_id);
};

/*!
 * \brief Type of the interfaces exposed by the transport module.
 */
enum mod_transport_api_idx {
    /*! Interface for driver module */
    MOD_TRANSPORT_API_IDX_DRIVER_INPUT,
    /*! Interface for scmi module */
    MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT,
    /*! Number of defined interfaces */
    MOD_TRANSPORT_API_IDX_COUNT,
};

/*!
 * \brief Transport notification indices.
 */
enum mod_transport_notification_idx {
    /*! The out_band mailbox/channel has been initialized */
    MOD_TRANSPORT_NOTIFICATION_IDX_INITIALIZED,

    /*! Number of defined notifications */
    MOD_TRANSPORT_NOTIFICATION_IDX_COUNT
};

/*!
 * \brief Identifier for the MOD_TRANSPORT_NOTIFICATION_IDX_INITIALIZED
 *     notification.
 */
static const fwk_id_t mod_transport_notification_id_initialized =
    FWK_ID_NOTIFICATION_INIT(
        FWK_MODULE_IDX_TRANSPORT,
        MOD_TRANSPORT_NOTIFICATION_IDX_INITIALIZED);

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* MOD_TRANSPORT_H */

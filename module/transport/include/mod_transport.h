/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
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
#ifdef BUILD_HAS_OUTBAND_MSG_SUPPORT
    /*! Out-band transport - SMT */
    MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_OUT_BAND,
#endif

#ifdef BUILD_HAS_INBAND_MSG_SUPPORT
    /*! In-band transport - MHUv2, MHUv3, I2C, etc*/
    MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_IN_BAND,
#endif

    /*! Trigger interrupt only */
    MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_NONE,

#ifdef BUILD_HAS_FAST_CHANNELS
    /*! Fast Channel transport */
    MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS,
#endif

    /*! Channel transport type count */
    MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_COUNT,
};

/*!
 * \brief Fast channel interrupt type.
 *
 * \note On platforms, where hardware does not support fast channel interrupts,
 *     a TIMER based interrupt need to be enabled by the driver of the
 *     transport module that supports fast channel. However this restricts
 *     registration of a different callback per fast channel. On such platforms
 *     only one call back is allowed to be registered and that callback should
 *     take action(if any) for all the fast channels on that platform.
 *     On the platform(such as platform which implements MHUv3) where hardware
 *     interrupts can be enabled for each update on respective fast channel,
 *     client modules are allowed to register same or different callbacks per
 *     fast channel.
 *     Client modules must obtain this information using call to
 *     `get_fch(interrupt_type)`.
 */
enum mod_transport_fch_interrupt_type {
    /*! Fast channel interrupts are emulated using timer interrupt */
    MOD_TRANSPORT_FCH_INTERRUPT_TYPE_TIMER,

    /*! Fast channel hardware supported interrupts */
    MOD_TRANSPORT_FCH_INTERRUPT_TYPE_HW,

    /*! Fast channel transport interrupt type count */
    MOD_TRANSPORT_FCH_INTERRUPT_TYPE_COUNT,
};

/*!
 * \brief Fast channel doorbell information.
 */
struct mod_transport_fch_doorbell_info {
    /*! Doorbell register width */
    uint32_t doorbell_register_width;

    /*! Doorbell support */
    bool doorbell_support;
};

/*!
 * \brief Platform notification source and notification id
 *
 * \details On platforms that require platform configuration (in addition to
 *      the clock configuration) to access the shared memory regions used by
 *      mailbox, the platform notification can be subscribed. This is optional
 *      for a channel and if provided as module configuration data, the channel
 *      will be setup only after this notification is processed.
 */
struct mod_transport_platform_notification {
    /*! Identifier of the notification id */
    const fwk_id_t notification_id;

    /*! Identifier of the module sending the notification */
    const fwk_id_t source_id;
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

    /*!
     * Platform notification source and notification id (optional)
     */
    struct mod_transport_platform_notification platform_notification;

    /*!
     * Identifier of the API to bind to signal message/error for Firmware
     * channels. This field is irrelevant for channels that are used for
     * SCMI messages.
     */
    fwk_id_t signal_api_id;

    /*! Channel policies */
    uint32_t policies;

    /*! Identifier of the driver */
    fwk_id_t driver_id;

    /*! Identifier of the driver API to bind to */
    fwk_id_t driver_api_id;
};

/*!
 * \brief Data structure encapsulating address(es) and length of a fast channel
 *    between two components
 */
struct mod_transport_fast_channel_addr {
    /*! Address as seen by the firmware running on current processor */
    uintptr_t local_view_address;
    /*! Address as seen by the firmware/OS running on target processor */
    uintptr_t target_view_address;
    /*! Length of the fast channel in bytes */
    size_t length;
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

#ifdef BUILD_HAS_FAST_CHANNELS
    /*!
     * \brief Get fast channel address information.
     *
     * \param fch_id Fast channel identifier.
     * \param[out] fch_address Holds requested fast channel address.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     */
    int (*get_fch_address)(
        fwk_id_t fch_id,
        struct mod_transport_fast_channel_addr *fch_address);

    /*!
     * \brief Get fast channel interrupt type.
     *
     * \param fch_id Fast channel identifier
     * \param[out] fch_interrupt_type Holds requested fast channel interrupt
     *     type information. This will be either
     *     ::MOD_TRANSPORT_FCH_INTERRUPT_TYPE_TIMER or
     *     ::MOD_TRANSPORT_FCH_INTERRUPT_TYPE_HW
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     */
    int (*get_fch_interrupt_type)(
        fwk_id_t fch_id,
        enum mod_transport_fch_interrupt_type *fch_interrupt_type);

    /*!
     * \brief Get fast channel doorbell information.
     *
     * \param fch_id Fast channel identifier
     * \param[out] doorbell_info Holds requested doorbell information.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     */
    int (*get_fch_doorbell_info)(
        fwk_id_t fch_id,
        struct mod_transport_fch_doorbell_info *doorbell_info);

    /*!
     * \brief Get fast channel rate limit.
     *
     * \param fch_id Fast channel identifier
     * \param[out] rate_limit Holds requested fast channel rate limit.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     */
    int (*get_fch_rate_limit)(fwk_id_t fch_id, uint32_t *fch_rate_limit);

    /*!
     * \brief Register a callback function in the driver
     *
     * \param fch_id fast channel identifier
     * \param param Context-specific value
     * \param fch_callback Pointer to the callback function
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     */
    int (*fch_register_callback)(
        fwk_id_t fch_id,
        uintptr_t param,
        void (*fch_callback)(uintptr_t param));
#endif
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
 * \brief Firmware transport API
 *
 * \details Interface used for Firmware messages.
 */
struct mod_transport_firmware_api {
    /*!
     * \brief Check whether a channel is secure or non-secure.
     *
     * \param channel_id Channel identifier.
     * \param[out] secure Channel security state. True if the channel
     *      is secure, or false if it is non-secure.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered:
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
     *      - The `size` parameter was a null pointer value.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */

    int (*get_max_payload_size)(fwk_id_t channel_id, size_t *size);

    /*!
     * \brief Get the message header from a channel.
     *
     * \param channel_id Channel identifier.
     * \param[out] message_header message header.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered:
     *      - The `message_header` parameter was a null pointer value.
     * \retval ::FWK_E_ACCESS No message is available to read.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*get_message_header)(fwk_id_t channel_id, uint32_t *message_header);

    /*!
     * \brief Get the payload from a channel.
     *
     * \param channel_id Channel identifier.
     * \param[out] payload Pointer to the payload.
     * \param[out] size Payload size. May be NULL, in which case the
     *      parameter should be ignored.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM An invalid parameter was encountered:
     *      - The `payload` parameter was a null pointer value.
     *      - The `size` parameter was a null pointer value.
     * \retval ::FWK_E_ACCESS No message is available to read.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*get_payload)(fwk_id_t channel_id, const void **payload, size_t *size);

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
     *      - The `payload` parameter was a null pointer value.
     *      - The offset and size provided are not within the bounds of the
     *        payload area.
     * \retval ::FWK_E_ACCESS No message available to respond to.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*write_payload)(
        fwk_id_t channel_id,
        size_t offset,
        const void *payload,
        size_t size);

    /*!
     * \brief Respond to message on a channel.
     *
     * \param channel_id Channel identifier.
     * \param payload Payload data to write, or NULL if a payload has already
     *      been written.
     * \param size Size of the payload source.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_SUPPORT In-band message not supported.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*respond)(fwk_id_t channel_id, const void *payload, size_t size);

    /*!
     * \brief Send a message on a channel.
     *
     * \param channel_id Channel identifier.
     * \param message_header Message header.
     * \param payload Payload data to write.
     * \param size Size of the payload source.
     * \param request_ack_by_interrupt flag to select whether acknowledgement
     * interrupt is required for this message.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_BUSY Previous message was not read by agent/platform
     * \retval ::FWK_E_SUPPORT In-band message not supported.
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

    /*!
     * \brief Trigger interrupt on receiver using the driver.
     *
     * \param channel_id Transport channel identifier.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \return One of the standard error codes for implementation-defined
     *      errors
     */
    int (*trigger_interrupt)(fwk_id_t channel_id);
};

/*!
 * \brief Firmware Signal API
 *
 * \details Interface used to signal Firmware messages/errors.
 */
struct mod_transport_firmware_signal_api {
    /*!
     * \brief Signal to a service that a incoming message for it has
     *      incorrect length and payload size and so the incoming message has
     *      been dropped.
     *
     * \note Subscribed service should call the release_transport_channel_lock()
     *       to free the channel.
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
     * \param service_id Service identifier.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     * \retval ::FWK_E_PARAM The `service_id` parameter was not a valid system
     *      entity identifier.
     * \return One of the standard error codes for implementation-defined
     *      errors.
     */
    int (*signal_message)(fwk_id_t service_id);
};

#ifdef BUILD_HAS_FAST_CHANNELS

/*!
 * \brief Fast Channels API
 *
 * \details Interface used for Fast Channels
 */
struct mod_transport_fast_channels_api {
    /*!
     * \brief Get fast channel address information.
     *
     * \param fch_id Fast channel identifier.
     * \param[out] fch_address Holds requested fast channel address.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     */
    int (*transport_get_fch_address)(
        fwk_id_t fch_id,
        struct mod_transport_fast_channel_addr *fch_address);

    /*!
     * \brief Get fast channel interrupt type information.
     *
     * \param fch_id Fast channel identifier
     * \param[out] fch_interrupt_type Holds requested fast channel interrupt
     *     type information. This will be either
     *     ::MOD_TRANSPORT_FCH_INTERRUPT_TYPE_TIMER or
     *     ::MOD_TRANSPORT_FCH_INTERRUPT_TYPE_HW
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     */
    int (*transport_get_fch_interrupt_type)(
        fwk_id_t fch_id,
        enum mod_transport_fch_interrupt_type *fch_interrupt_type);

    /*!
     * \brief Get fast channel doorbell information.
     *
     * \param fch_id Fast channel identifier
     * \param[out] doorbell_info Holds requested doorbell information.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     */
    int (*transport_get_fch_doorbell_info)(
        fwk_id_t fch_id,
        struct mod_transport_fch_doorbell_info *doorbell_info);

    /*!
     * \brief Get fast channel rate limit.
     *
     * \param fch_id Fast channel identifier
     * \param[out] rate_limit Holds requested fast channel rate limit.
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     */
    int (*transport_get_fch_rate_limit)(fwk_id_t fch_id, uint32_t *rate_limit);

    /*!
     * \brief Register a callback function.
     *
     * \param fch_id Fast channel identifier
     * \param param Context-specific value
     * \param fch_callback Pointer to the callback function
     *
     * \retval ::FWK_SUCCESS The operation succeeded.
     */
    int (*transport_fch_register_callback)(
        fwk_id_t fch_id,
        uintptr_t param,
        void (*fch_callback)(uintptr_t param));
};
#endif

/*!
 * \brief Type of the interfaces exposed by the transport module.
 */
enum mod_transport_api_idx {
    /*! Interface for driver module */
    MOD_TRANSPORT_API_IDX_DRIVER_INPUT,
#ifdef BUILD_HAS_MOD_SCMI
    /*! Interface for scmi module */
    MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT,
#endif
    /*! Interface for MSCP Firmware communication */
    MOD_TRANSPORT_API_IDX_FIRMWARE,
#ifdef BUILD_HAS_FAST_CHANNELS
    /*! Interface for scmi protocol modules that use SCMI Fast Channels */
    MOD_TRANSPORT_API_IDX_FAST_CHANNELS,
#endif
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

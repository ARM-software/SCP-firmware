/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Transport HAL module for interfacing with transport driver modules.
 */

#include <internal/transport.h>

#ifdef BUILD_HAS_MOD_POWER_DOMAIN
#    include <mod_power_domain.h>
#endif

#ifdef BUILD_HAS_MOD_SCMI
#    include <mod_scmi.h>
#endif

#include <mod_transport.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>
#include <fwk_string.h>

#include <stdbool.h>

#define MOD_NAME "[TRANSPORT]"

struct transport_channel_ctx {
    /* Channel identifier */
    fwk_id_t id;

    /* Channel configuration data */
    struct mod_transport_channel_config *config;

    /* Channel read and write buffer areas */
    struct mod_transport_buffer *in, *out;

    /* Flag to indicate message processing in progress */
    volatile bool locked;

    /* Maximum payload size of the channel */
    size_t max_payload_size;

    /* Service bound to the channel */
    fwk_id_t service_id;

    /* Driver API */
    struct mod_transport_driver_api *driver_api;

    /* Service APIs to signal incoming messages or errors */
    union mod_transport_signal_api {
#ifdef BUILD_HAS_MOD_SCMI
        /* For SCMI messages or errors */
        struct mod_scmi_from_transport_api *scmi_signal_api;
#endif
        /* For Firmware messages or errors */
        struct mod_transport_firmware_signal_api *firmware_signal_api;
    } transport_signal;

    /* Flag indicating the service bound to the channel is of type SCMI */
    bool is_scmi;

    /* Flag indicating that the out-band mailbox is ready */
    bool out_band_mailbox_ready;
};

struct transport_context {
    /* Table of channel contexts */
    struct transport_channel_ctx *channel_ctx_table;

    /* Number of channels */
    unsigned int channel_count;
};

static struct transport_context transport_ctx;

/*
 * SCMI module Transport API
 */
static int transport_get_secure(fwk_id_t channel_id, bool *secure)
{
    struct transport_channel_ctx *channel_ctx;

    if (secure == NULL) {
        fwk_unexpected();
        return FWK_E_PARAM;
    }

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    *secure =
        ((channel_ctx->config->policies & MOD_TRANSPORT_POLICY_SECURE) !=
         (uint32_t)0);

    return FWK_SUCCESS;
}

static int transport_get_max_payload_size(fwk_id_t channel_id, size_t *size)
{
    struct transport_channel_ctx *channel_ctx;

    if (size == NULL) {
        fwk_unexpected();
        return FWK_E_PARAM;
    }

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    *size = channel_ctx->max_payload_size;

    return FWK_SUCCESS;
}

static int transport_get_message_header(fwk_id_t channel_id, uint32_t *header)
{
    struct transport_channel_ctx *channel_ctx;

    if (header == NULL) {
        fwk_unexpected();
        return FWK_E_PARAM;
    }

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    fwk_assert(
        channel_ctx->config->transport_type !=
        MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_NONE);

    if (!channel_ctx->locked) {
        return FWK_E_ACCESS;
    }

    *header = channel_ctx->in->message_header;

    return FWK_SUCCESS;
}

static int transport_get_payload(
    fwk_id_t channel_id,
    const void **payload,
    size_t *size)
{
    struct transport_channel_ctx *channel_ctx;

    if (payload == NULL || size == NULL) {
        fwk_unexpected();
        return FWK_E_PARAM;
    }

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    fwk_assert(
        channel_ctx->config->transport_type !=
        MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_NONE);

    if (!channel_ctx->locked) {
        return FWK_E_ACCESS;
    }

    *payload = channel_ctx->in->payload;

    *size = channel_ctx->in->length - sizeof(channel_ctx->in->message_header);

    return FWK_SUCCESS;
}

static int transport_write_payload(
    fwk_id_t channel_id,
    size_t offset,
    const void *payload,
    size_t size)
{
    struct transport_channel_ctx *channel_ctx;

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    fwk_assert(
        channel_ctx->config->transport_type !=
        MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_NONE);

    if ((payload == NULL) ||
        ((offset + size) > channel_ctx->max_payload_size)) {
        fwk_unexpected();
        return FWK_E_PARAM;
    }

    if (!channel_ctx->locked) {
        return FWK_E_ACCESS;
    }

    fwk_str_memcpy(
        ((uint8_t *)channel_ctx->out->payload) + offset, payload, size);

    return FWK_SUCCESS;
}

static int transport_respond(
    fwk_id_t channel_id,
    const void *payload,
    size_t size)
{
    struct transport_channel_ctx *channel_ctx;
    struct mod_transport_buffer *buffer;
    enum mod_transport_channel_transport_type transport_type;
    int status = FWK_SUCCESS;
    unsigned int flags;

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    transport_type = channel_ctx->config->transport_type;

    fwk_assert(transport_type != MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_NONE);

    if (transport_type == MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_OUT_BAND) {
        /* Use shared mailbox for out-band messages */
        buffer = ((struct mod_transport_buffer *)
                      channel_ctx->config->out_band_mailbox_address);

        /* Copy the header and other fields from the write buffer */
        fwk_str_memcpy(
            buffer, channel_ctx->out, sizeof(struct mod_transport_buffer));

        /*
         * Copy the payload from either the write buffer or the payload
         * parameter.
         */
        fwk_str_memcpy(
            buffer->payload,
            (payload == NULL ? channel_ctx->out->payload : payload),
            size);
    } else {
#ifdef BUILD_HAS_INBAND_MSG_SUPPORT
        /* Use internal write buffer for in-band messages */
        buffer = channel_ctx->out;

        /* Copy the payload from the payload parameter */
        if (payload != NULL) {
            fwk_str_memcpy(buffer->payload, payload, size);
        }
#else
        FWK_LOG_ERR("%s ERROR. IN-BAND MESSAGE NOT SUPPORTED!", MOD_NAME);
        return FWK_E_SUPPORT;
#endif
    }

    /*
     * NOTE: Disable interrupts for a brief period to ensure interrupts are not
     * erroneously accepted in between unlocking the context, and setting
     * the mailbox free bit. The agent should not interrupt during this
     * period anyway, but this guard is included to protect against a
     * misbehaving agent.
     */
    flags = fwk_interrupt_global_disable();

    channel_ctx->locked = false;
    buffer->length = (volatile uint32_t)(sizeof(buffer->message_header) + size);
    /* The mailbox status is relevant for out-band transport only */
    buffer->status |= MOD_TRANSPORT_MAILBOX_STATUS_FREE_MASK;

    fwk_interrupt_global_enable(flags);

#ifdef BUILD_HAS_INBAND_MSG_SUPPORT
    if (transport_type == MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_IN_BAND) {
        /* Send the response message using driver module API */
        status = channel_ctx->driver_api->send_message(
            buffer, channel_ctx->config->driver_id);

        if (status != FWK_SUCCESS) {
            return status;
        }
    }
#endif

    if (buffer->flags & MOD_TRANSPORT_MAILBOX_FLAGS_IENABLED_MASK) {
        status = channel_ctx->driver_api->trigger_event(
            channel_ctx->config->driver_id);
    }

    return status;
}

static int transport_transmit(
    fwk_id_t channel_id,
    uint32_t message_header,
    const void *payload,
    size_t size,
    bool request_ack_by_interrupt)
{
    struct transport_channel_ctx *channel_ctx;
    struct mod_transport_buffer *buffer;
#ifdef BUILD_HAS_INBAND_MSG_SUPPORT
    int status;
#endif
    enum mod_transport_channel_transport_type transport_type;

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    transport_type = channel_ctx->config->transport_type;

    fwk_assert(transport_type != MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_NONE);

    if (transport_type == MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_OUT_BAND) {
        /* Use shared mailbox for out-band messages */
        buffer = ((struct mod_transport_buffer *)
                      channel_ctx->config->out_band_mailbox_address);
        /*
         * If the agent/platform has not yet read the previous message we
         * abandon this transmission. We don't want to poll on the BUSY/FREE
         * bit, and while it is probably safe to just overwrite the data
         * the agent/platform could be in the process of reading.
         */
        if ((buffer->status & MOD_TRANSPORT_MAILBOX_STATUS_FREE_MASK) ==
            (uint32_t)0) {
            return FWK_E_BUSY;
        }
    } else {
#ifdef BUILD_HAS_INBAND_MSG_SUPPORT
        /* Use internal write buffer for in-band messages */
        buffer = channel_ctx->out;
        /* reserved fields must be set to zero */
        buffer->reserved0 = 0;
        buffer->reserved1 = 0;
#else
        FWK_LOG_ERR("%s ERROR. IN-BAND MESSAGES NOT SUPPORTED!", MOD_NAME);
        return FWK_E_SUPPORT;
#endif
    }

    buffer->message_header = message_header;

    if (request_ack_by_interrupt) {
        buffer->flags |= MOD_TRANSPORT_MAILBOX_FLAGS_IENABLED_MASK;
    } else {
        buffer->flags &= ~MOD_TRANSPORT_MAILBOX_FLAGS_IENABLED_MASK;
    }

    /* Copy the payload */
    if (payload != NULL && size != 0) {
        fwk_str_memcpy(buffer->payload, payload, size);
    }

    buffer->length = (volatile uint32_t)(sizeof(buffer->message_header) + size);
    /* The mailbox status is relevant for out-band transport only */
    buffer->status &= ~MOD_TRANSPORT_MAILBOX_STATUS_FREE_MASK;

#ifdef BUILD_HAS_INBAND_MSG_SUPPORT
    if (transport_type == MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_IN_BAND) {
        /* Send the SCMI message using driver module API */
        status = channel_ctx->driver_api->send_message(
            channel_ctx->out, channel_ctx->config->driver_id);

        if (status != FWK_SUCCESS) {
            return status;
        }
    }
#endif

    /* Notify the agent/platform and return */
    return channel_ctx->driver_api->trigger_event(
        channel_ctx->config->driver_id);
}

static int transport_release_channel_lock(fwk_id_t channel_id)
{
    struct transport_channel_ctx *channel_ctx;

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    /*
     * If the received message is a response message, then release
     * the channel lock so that we can process the next message.
     *
     * If this is not done, then for messages that don't require client module
     * to call the transport_respond() function will lead to situation
     * where the channel context is locked and never released since it is the
     * transport_respond() function that releases the channel context.
     */
    channel_ctx->locked = false;
    return FWK_SUCCESS;
}

static int transport_trigger_interrupt(fwk_id_t channel_id)
{
    struct transport_channel_ctx *channel_ctx;

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    fwk_assert(
        channel_ctx->config->transport_type ==
        MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_NONE);

    return channel_ctx->driver_api->trigger_event(
        channel_ctx->config->driver_id);
}

#ifdef BUILD_HAS_MOD_SCMI
static const struct mod_scmi_to_transport_api
    transport_mod_scmi_to_transport_api = {
        .get_secure = transport_get_secure,
        .get_max_payload_size = transport_get_max_payload_size,
        .get_message_header = transport_get_message_header,
        .get_payload = transport_get_payload,
        .write_payload = transport_write_payload,
        .respond = transport_respond,
        .transmit = transport_transmit,
        .release_transport_channel_lock = transport_release_channel_lock,
    };
#endif

static const struct mod_transport_firmware_api transport_firmware_api = {
    .get_secure = transport_get_secure,
    .get_max_payload_size = transport_get_max_payload_size,
    .get_message_header = transport_get_message_header,
    .get_payload = transport_get_payload,
    .write_payload = transport_write_payload,
    .respond = transport_respond,
    .transmit = transport_transmit,
    .release_transport_channel_lock = transport_release_channel_lock,
    .trigger_interrupt = transport_trigger_interrupt,
};

#ifdef BUILD_HAS_FAST_CHANNELS

static int transport_get_fch_address(
    fwk_id_t fch_id,
    struct mod_transport_fast_channel_addr *fch_addr)
{
    struct transport_channel_ctx *channel_ctx;

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(fch_id)];

    fwk_assert(
        channel_ctx->config->transport_type ==
        MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS);

    return channel_ctx->driver_api->get_fch_address(
        channel_ctx->config->driver_id, fch_addr);
}

static int transport_get_fch_interrupt_type(
    fwk_id_t fch_id,
    enum mod_transport_fch_interrupt_type *fch_interrupt_type)
{
    struct transport_channel_ctx *channel_ctx;

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(fch_id)];

    fwk_assert(
        channel_ctx->config->transport_type ==
        MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS);

    return channel_ctx->driver_api->get_fch_interrupt_type(
        channel_ctx->config->driver_id, fch_interrupt_type);
}

/*!
 * \brief Get fast channel doorbell information.
 *
 * \param fch_id Fast channel identifier
 * \param[out] doorbell_info Holds requested doorbell information.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 */
static int transport_get_fch_doorbell_info(
    fwk_id_t fch_id,
    struct mod_transport_fch_doorbell_info *doorbell_info)
{
    struct transport_channel_ctx *channel_ctx;

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(fch_id)];

    fwk_assert(
        channel_ctx->config->transport_type ==
        MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS);

    return channel_ctx->driver_api->get_fch_doorbell_info(
        channel_ctx->config->driver_id, doorbell_info);
}

/*!
 * \brief Get fast channel rate limit information.
 *
 * \param fch_id Fast channel identifier
 * \param[out] rate_limit Holds requested rate limit information.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 */
static int transport_get_fch_rate_limit(fwk_id_t fch_id, uint32_t *rate_limit)
{
    struct transport_channel_ctx *channel_ctx;

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(fch_id)];

    fwk_assert(
        channel_ctx->config->transport_type ==
        MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS);

    return channel_ctx->driver_api->get_fch_rate_limit(
        channel_ctx->config->driver_id, rate_limit);
}

static int transport_fch_register_callback(
    fwk_id_t fch_id,
    uintptr_t param,
    void (*fch_callback)(uintptr_t param))
{
    struct transport_channel_ctx *channel_ctx;

    if (fch_callback == NULL) {
        return FWK_E_PARAM;
    }

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(fch_id)];

    fwk_assert(
        channel_ctx->config->transport_type ==
        MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS);

    /* Provide callback function pointer to driver and get it registered */
    return channel_ctx->driver_api->fch_register_callback(
        channel_ctx->config->driver_id, param, fch_callback);
}

static const struct mod_transport_fast_channels_api
    transport_fast_channels_api = {
        .transport_get_fch_address = transport_get_fch_address,
        .transport_get_fch_interrupt_type = transport_get_fch_interrupt_type,
        .transport_get_fch_doorbell_info = transport_get_fch_doorbell_info,
        .transport_get_fch_rate_limit = transport_get_fch_rate_limit,
        .transport_fch_register_callback = transport_fch_register_callback,
    };
#endif

static int transport_message_handler(struct transport_channel_ctx *channel_ctx)
{
    struct mod_transport_buffer *in, *out, *shared_memory;
    enum mod_transport_channel_transport_type transport_type;
    size_t payload_size;
    int status;

    transport_type = channel_ctx->config->transport_type;

    /* Check if we are already processing */
    if (channel_ctx->locked) {
        return FWK_E_STATE;
    }

    in = channel_ctx->in;
    out = channel_ctx->out;

    if (transport_type == MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_OUT_BAND) {
        shared_memory = ((struct mod_transport_buffer *)
                             channel_ctx->config->out_band_mailbox_address);

        if (channel_ctx->config->channel_type ==
            MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER) {
            /*
             * The completer type channel is used for handling the requests.
             *
             * Check if we have the ownership of shared mailbox. We don't
             * have the mailbox ownership if the mailbox status is 1 (free).
             */
            if (shared_memory->status &
                MOD_TRANSPORT_MAILBOX_STATUS_FREE_MASK) {
                FWK_LOG_ERR(
                    "%s Mailbox ownership error on completer channel %u",
                    MOD_NAME,
                    fwk_id_get_element_idx(channel_ctx->id));

                return FWK_E_STATE;
            }
        } else if (
            channel_ctx->config->channel_type ==
            MOD_TRANSPORT_CHANNEL_TYPE_REQUESTER) {
            /*
             * The requester type channel is used for handling the responses.
             *
             * Check if we have the ownership of shared mailbox. We don't
             * have the mailbox ownership if the mailbox status is 0 (busy).
             */
            if ((shared_memory->status &
                 MOD_TRANSPORT_MAILBOX_STATUS_FREE_MASK) == 0) {
                FWK_LOG_ERR(
                    "%s Mailbox ownership error on requester channel %u",
                    MOD_NAME,
                    fwk_id_get_element_idx(channel_ctx->id));

                return FWK_E_STATE;
            }
        }
        /*
         * Copy the contents from shared mailbox to internal read buffer.
         * note: payload is not copied yet.
         */
        fwk_str_memcpy(in, shared_memory, sizeof(struct mod_transport_buffer));
    }
    /*
     * Set the channel context as locked until the bound service completes
     * processing the message.
     */
    channel_ctx->locked = true;

#ifdef BUILD_HAS_INBAND_MSG_SUPPORT
    if (transport_type == MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_IN_BAND) {
        /* get the message from the driver */
        channel_ctx->driver_api->get_message(
            in, channel_ctx->config->driver_id);
    }
#endif
    /* mirror contents in the read & write buffers (Payload not copied) */
    fwk_str_memcpy(out, in, sizeof(struct mod_transport_buffer));

    /* Ensure error bit is not set */
    out->status &= ~MOD_TRANSPORT_MAILBOX_STATUS_ERROR_MASK;

    /*
     * Verify:
     * 1. The length is at least as large as the message header
     * 2. The length, minus the size of the message header, is less than or
     *         equal to the maximum payload size
     *
     * Note: the payload size is permitted to be of size zero.
     */
    if ((in->length < sizeof(in->message_header)) ||
        ((in->length - sizeof(in->message_header)) >
         channel_ctx->max_payload_size)) {
        out->status |= MOD_TRANSPORT_MAILBOX_STATUS_ERROR_MASK;

        if (channel_ctx->is_scmi) {
#ifdef BUILD_HAS_MOD_SCMI
            status =
                channel_ctx->transport_signal.scmi_signal_api->signal_error(
                    channel_ctx->service_id);
#else
            FWK_LOG_INFO(
                "%s Error! SCMI module not included in the build", MOD_NAME);
            return FWK_E_SUPPORT;
#endif
        } else {
            status =
                channel_ctx->transport_signal.firmware_signal_api->signal_error(
                    channel_ctx->service_id);
        }
    }

    if (transport_type == MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_OUT_BAND) {
        shared_memory = ((struct mod_transport_buffer *)
                             channel_ctx->config->out_band_mailbox_address);

        payload_size = in->length - sizeof(in->message_header);
        if (payload_size != 0) {
            /* Copy payload from shared memory to read buffer */
            fwk_str_memcpy(in->payload, shared_memory->payload, payload_size);
        }
    }

    /* Let the subscribed service handle the message */
    if (channel_ctx->is_scmi) {
#ifdef BUILD_HAS_MOD_SCMI
        /* Signal the SCMI service */
        status = channel_ctx->transport_signal.scmi_signal_api->signal_message(
            channel_ctx->service_id);
#else
        FWK_LOG_INFO(
            "%s Error! SCMI module not included in the build", MOD_NAME);
        return FWK_E_SUPPORT;
#endif
    } else {
        /* Signal the service */
        status =
            channel_ctx->transport_signal.firmware_signal_api->signal_message(
                channel_ctx->service_id);
    }

    if (status != FWK_SUCCESS) {
        return FWK_E_HANDLER;
    }

    return status;
}

/*
 *  Driver module API
 */
static int transport_signal_message(fwk_id_t channel_id)
{
    int status;
    struct transport_channel_ctx *channel_ctx;

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    /*
     * If the channel is used for events only, then signal the module.
     * since, there wouldn't be any messages for this channel.
     */
    if (channel_ctx->config->transport_type ==
        MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_NONE) {
        /* Signal the service */
        status =
            channel_ctx->transport_signal.firmware_signal_api->signal_message(
                channel_ctx->service_id);

        return status;
    }

    if (channel_ctx->config->transport_type ==
        MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_OUT_BAND) {
        if (!channel_ctx->out_band_mailbox_ready) {
            /* Discard any message in the mailbox when not ready */
            FWK_LOG_ERR("%s Out-band message not valid", MOD_NAME);

            return FWK_SUCCESS;
        }
    }
    return transport_message_handler(channel_ctx);
}

static const struct mod_transport_driver_input_api driver_input_api = {
    .signal_message = transport_signal_message,
};

static int transport_mailbox_init(struct transport_channel_ctx *channel_ctx)
{
    int status = FWK_SUCCESS;

    if ((channel_ctx->config->policies & MOD_TRANSPORT_POLICY_INIT_MAILBOX) !=
        (uint32_t)0) {
        unsigned int notifications_sent;

        /* Only the completer channel should initialize the shared mailbox */
        if (channel_ctx->config->channel_type ==
            MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER) {
            /* Initialize mailbox such that the requester has ownership */
            *((struct mod_transport_buffer *)
                  channel_ctx->config->out_band_mailbox_address) =
                (struct mod_transport_buffer){
                    .status = (1U << MOD_TRANSPORT_MAILBOX_STATUS_FREE_POS)
                };
        }
        /* Notify that this mailbox is initialized */
        struct fwk_event transport_channel_initialized_notification = {
            .id = mod_transport_notification_id_initialized,
            .source_id = channel_ctx->id,
        };

        channel_ctx->out_band_mailbox_ready = true;

        status = fwk_notification_notify(
            &transport_channel_initialized_notification, &notifications_sent);
    }
    return status;
}

/*
 * Framework API
 */
static int transport_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    transport_ctx.channel_ctx_table = fwk_mm_calloc(
        element_count, sizeof(transport_ctx.channel_ctx_table[0]));
    transport_ctx.channel_count = element_count;

    return FWK_SUCCESS;
}

static int transport_channel_init(
    fwk_id_t channel_id,
    unsigned int unused,
    const void *data)
{
    struct transport_channel_ctx *channel_ctx;

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];
    channel_ctx->config = (struct mod_transport_channel_config *)data;

    /* Validate channel config */
    if ((channel_ctx->config->channel_type >=
         MOD_TRANSPORT_CHANNEL_TYPE_COUNT) ||
        (channel_ctx->config->transport_type >=
         MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_COUNT)) {
        fwk_unexpected();
        return FWK_E_DATA;
    }

    /* Validate out-band mailbox address and size */
    if ((channel_ctx->config->transport_type ==
         MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_OUT_BAND) &&
        ((channel_ctx->config->out_band_mailbox_address == 0) ||
         (channel_ctx->config->out_band_mailbox_size == 0))) {
        fwk_unexpected();
        return FWK_E_DATA;
    }

#ifdef BUILD_HAS_INBAND_MSG_SUPPORT
    /* Validate in-band mailbox size */
    if ((channel_ctx->config->transport_type ==
         MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_IN_BAND) &&
        (channel_ctx->config->in_band_mailbox_size == 0)) {
        fwk_unexpected();
        return FWK_E_DATA;
    }
#endif
    channel_ctx->id = channel_id;

    switch (channel_ctx->config->transport_type) {
    case MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_OUT_BAND:
        channel_ctx->in =
            fwk_mm_alloc(1, channel_ctx->config->out_band_mailbox_size);
        channel_ctx->out =
            fwk_mm_alloc(1, channel_ctx->config->out_band_mailbox_size);
        channel_ctx->max_payload_size =
            channel_ctx->config->out_band_mailbox_size -
            sizeof(struct mod_transport_buffer);
        break;

#ifdef BUILD_HAS_INBAND_MSG_SUPPORT
    case MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_IN_BAND:
        channel_ctx->in =
            fwk_mm_alloc(1, channel_ctx->config->in_band_mailbox_size);
        channel_ctx->out =
            fwk_mm_alloc(1, channel_ctx->config->in_band_mailbox_size);
        channel_ctx->max_payload_size =
            channel_ctx->config->in_band_mailbox_size -
            sizeof(struct mod_transport_buffer);
        break;
#endif

    case MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_NONE:
        /* This channel must be used for sending/receiving events only */
        channel_ctx->in = NULL;
        channel_ctx->out = NULL;
        channel_ctx->max_payload_size = 0;
        break;

#ifdef BUILD_HAS_FAST_CHANNELS
    case MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS:
        /* This transport channel is used for Fast channels only */
        channel_ctx->in = NULL;
        channel_ctx->out = NULL;
        channel_ctx->max_payload_size = 0;
        break;
#endif

    default:
        return FWK_E_DATA;
    }

    return FWK_SUCCESS;
}

static int transport_bind(fwk_id_t id, unsigned int round)
{
    struct transport_channel_ctx *channel_ctx;
    int status;

    /* bind to driver API */
    if (round == 0) {
        if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
            return FWK_SUCCESS;
        }
        channel_ctx =
            &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(id)];
        status = fwk_module_bind(
            channel_ctx->config->driver_id,
            channel_ctx->config->driver_api_id,
            &channel_ctx->driver_api);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    /* bind to module signal API */
    if (round == 1) {
        if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
            return FWK_SUCCESS;
        }

        channel_ctx =
            &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(id)];

#ifdef BUILD_HAS_FAST_CHANNELS
        if (channel_ctx->config->transport_type ==
            MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_FAST_CHANNELS) {
            /*
             * Don't bind to signal API for fast channels since the
             * driver module will notify the client module directly.
             */
            return FWK_SUCCESS;
        }
#endif

#ifdef BUILD_HAS_MOD_SCMI
        if (fwk_id_is_equal(
                fwk_id_build_module_id(channel_ctx->service_id),
                fwk_module_id_scmi)) {
            status = fwk_module_bind(
                channel_ctx->service_id,
                FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_TRANSPORT),
                &channel_ctx->transport_signal.scmi_signal_api);

            channel_ctx->is_scmi = true;
            return status;
        }
#endif
        status = fwk_module_bind(
            channel_ctx->service_id,
            channel_ctx->config->signal_api_id,
            &channel_ctx->transport_signal.firmware_signal_api);

        channel_ctx->is_scmi = false;
        return status;
    }

    return FWK_SUCCESS;
}

static int transport_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    struct transport_channel_ctx *channel_ctx;

    enum mod_transport_api_idx api_id_type;

    /* Only allow binding to a channel (not the whole module) */
    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT)) {
        /* Tried to bind to something other than a specific channel */
        fwk_unexpected();
        return FWK_E_PARAM;
    }

    channel_ctx =
        &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(target_id)];

    api_id_type = (enum mod_transport_api_idx)fwk_id_get_api_idx(api_id);

    switch (api_id_type) {
    case MOD_TRANSPORT_API_IDX_DRIVER_INPUT:
        /* Driver input API */

        /*
         * Make sure that the element that is trying to bind to us is the
         * same element that we previously bound to.
         *
         * NOTE: We bound to an element but a sub-element should be binding
         * back to us. This means we cannot use fwk_id_is_equal() because
         * the ids have different types. For now we compare the indicies
         * manually.
         */
        if (fwk_id_get_module_idx(channel_ctx->config->driver_id) ==
                fwk_id_get_module_idx(source_id) &&
            fwk_id_get_element_idx(channel_ctx->config->driver_id) ==
                fwk_id_get_element_idx(source_id)) {
            *api = &driver_input_api;
        } else {
            /* A module that we did not bind to is trying to bind to us */
            fwk_unexpected();
            return FWK_E_ACCESS;
        }
        break;

#ifdef BUILD_HAS_MOD_SCMI
    case MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT:
        /* SCMI transport API */
        *api = &transport_mod_scmi_to_transport_api;
        channel_ctx->service_id = source_id;
        break;
#endif

    case MOD_TRANSPORT_API_IDX_FIRMWARE:
        /* transport API for Firmware messages */
        *api = &transport_firmware_api;
        channel_ctx->service_id = source_id;
        break;

#ifdef BUILD_HAS_FAST_CHANNELS
    case MOD_TRANSPORT_API_IDX_FAST_CHANNELS:
        /* Fast Channels transport API */
        *api = &transport_fast_channels_api;
        channel_ctx->service_id = source_id;
        break;
#endif

    default:
        /* Invalid API */
        fwk_unexpected();
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int transport_start(fwk_id_t id)
{
    int status = FWK_SUCCESS;
    struct transport_channel_ctx *channel_ctx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        return status;
    }

    channel_ctx = &transport_ctx.channel_ctx_table[fwk_id_get_element_idx(id)];

    if (channel_ctx->config->transport_type ==
        MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_OUT_BAND) {
#ifdef BUILD_HAS_MOD_POWER_DOMAIN
        if (fwk_id_type_is_valid(channel_ctx->config->pd_source_id)) {
            /* Register for power domain state transition notifications */
            return fwk_notification_subscribe(
                mod_pd_notification_id_power_state_transition,
                channel_ctx->config->pd_source_id,
                id);
        } else {
            return transport_mailbox_init(channel_ctx);
        }
#else
        /*
         * Initialize the mailbox immediately, if power domain module
         * is not included in the firmware build.
         */
        return transport_mailbox_init(channel_ctx);
#endif
    }
    return status;
}

#ifdef BUILD_HAS_MOD_POWER_DOMAIN
static int transport_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct transport_channel_ctx *channel_ctx;
    int status = FWK_SUCCESS;

    channel_ctx =
        &transport_ctx
             .channel_ctx_table[fwk_id_get_element_idx(event->target_id)];

    struct mod_pd_power_state_transition_notification_params *params;

    assert(fwk_id_is_equal(
        event->id, mod_pd_notification_id_power_state_transition));
    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    params = (struct mod_pd_power_state_transition_notification_params *)
                 event->params;

    if (params->state != MOD_PD_STATE_ON) {
        if (params->state == MOD_PD_STATE_OFF) {
            channel_ctx->out_band_mailbox_ready = false;
        }
    } else {
        status = transport_mailbox_init(channel_ctx);
    }
    return status;
}
#endif

const struct fwk_module module_transport = {
    .type = FWK_MODULE_TYPE_HAL,
    .api_count = (unsigned int)MOD_TRANSPORT_API_IDX_COUNT,
    .notification_count = (unsigned int)MOD_TRANSPORT_NOTIFICATION_IDX_COUNT,
    .init = transport_init,
    .element_init = transport_channel_init,
    .bind = transport_bind,
    .start = transport_start,
    .process_bind_request = transport_process_bind_request,
#ifdef BUILD_HAS_MOD_POWER_DOMAIN
    .process_notification = transport_process_notification,
#endif
};

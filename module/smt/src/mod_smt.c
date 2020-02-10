/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <internal/scmi.h>
#include <internal/smt.h>

#include <mod_power_domain.h>
#include <mod_scmi.h>
#include <mod_smt.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <string.h>

struct smt_channel_ctx {
    /* Channel identifier */
    fwk_id_t id;

    /* Channel configuration data */
    struct mod_smt_channel_config *config;

    /* Channel read and write cache memory areas */
    struct mod_smt_memory *in, *out;

    /* Message processing in progrees flag */
    volatile bool locked;

    /* Maximum payload size of the channel */
    size_t max_payload_size;

    /* Driver entity identifier */
    fwk_id_t driver_id;

    /* service bound to the channel */
    fwk_id_t service_id;

    /* Driver API */
    struct mod_smt_driver_api *driver_api;

    /* service APIs to signal incoming messages or errors */
    union signal_apis {
        struct mod_scmi_from_transport_api *scmi_api;
        struct mod_smt_from_transport_api *signal_api;
    } smt_signal;

    /* Flag indicating the service bound to the channel is of type SCMI */
    bool is_scmi_channel;

    /* Flag indicating the mailbox is ready */
    bool smt_mailbox_ready;
};

struct smt_ctx {
    /* Table of channel contexts */
    struct smt_channel_ctx *channel_ctx_table;

    /* Number of channels */
    unsigned int channel_count;
};

static struct smt_ctx smt_ctx;

/*
 * SCMI Transport API
 */
static int smt_get_secure(fwk_id_t channel_id, bool *secure)
{
    struct smt_channel_ctx *channel_ctx;

    if (secure == NULL) {
        assert(false);
        return FWK_E_PARAM;
    }

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    *secure = channel_ctx->config->policies & MOD_SMT_POLICY_SECURE;

    return FWK_SUCCESS;
}

static int smt_get_max_payload_size(fwk_id_t channel_id, size_t *size)
{
    struct smt_channel_ctx *channel_ctx;

    if (size == NULL) {
        assert(false);
        return FWK_E_PARAM;
    }

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    *size = channel_ctx->max_payload_size;

    return FWK_SUCCESS;
}

static int smt_get_message_header(fwk_id_t channel_id, uint32_t *header)
{
    struct smt_channel_ctx *channel_ctx;

    if (header == NULL) {
        assert(false);
        return FWK_E_PARAM;
    }

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    if (!channel_ctx->locked)
        return FWK_E_ACCESS;

    *header = channel_ctx->in->message_header;

    return FWK_SUCCESS;
}

static int smt_get_payload(fwk_id_t channel_id,
                           const void **payload,
                           size_t *size)
{
    struct smt_channel_ctx *channel_ctx;

    if (payload == NULL) {
        assert(false);
        return FWK_E_PARAM;
    }

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    if (!channel_ctx->locked)
        return FWK_E_ACCESS;

    *payload = channel_ctx->in->payload;

    if (size != NULL) {
        *size = channel_ctx->in->length -
            sizeof(channel_ctx->in->message_header);
    }

    return FWK_SUCCESS;
}

static int smt_write_payload(fwk_id_t channel_id,
                             size_t offset,
                             const void *payload,
                             size_t size)
{
    struct smt_channel_ctx *channel_ctx;

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    if ((payload == NULL)                         ||
        (offset  > channel_ctx->max_payload_size) ||
        (size > channel_ctx->max_payload_size)    ||
        ((offset + size) > channel_ctx->max_payload_size)) {

        assert(false);
        return FWK_E_PARAM;
    }

    if (!channel_ctx->locked)
        return FWK_E_ACCESS;

    memcpy(((uint8_t*)channel_ctx->out->payload) + offset, payload, size);

    return FWK_SUCCESS;
}

static int smt_respond(fwk_id_t channel_id, const void *payload, size_t size)
{
    struct smt_channel_ctx *channel_ctx;
    struct mod_smt_memory *memory;

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];
    memory = ((struct mod_smt_memory*)channel_ctx->config->mailbox_address);

    /* Copy the header from the write buffer */
    *memory = *channel_ctx->out;

    /* Copy the payload from either the write buffer or the payload parameter */
    memcpy(memory->payload,
           (payload == NULL ? channel_ctx->out->payload : payload),
           size);

    /*
     * NOTE: Disable interrupts for a brief period to ensure interrupts are not
     * erroneously accepted in between unlocking the context, and setting
     * the mailbox free bit. The agent should not interrupt during this
     * period anyway, but this guard is included to protect against a
     * misbehaving agent.
     */
    fwk_interrupt_global_disable();

    channel_ctx->locked = false;

    memory->length = sizeof(memory->message_header) + size;
    memory->status |= MOD_SMT_MAILBOX_STATUS_FREE_MASK;

    fwk_interrupt_global_enable();

    if (memory->flags & MOD_SMT_MAILBOX_FLAGS_IENABLED_MASK)
        channel_ctx->driver_api->raise_interrupt(channel_ctx->driver_id);

    return FWK_SUCCESS;
}

static const struct mod_scmi_to_transport_api smt_mod_scmi_to_transport_api = {
    .get_secure = smt_get_secure,
    .get_max_payload_size = smt_get_max_payload_size,
    .get_message_header = smt_get_message_header,
    .get_payload = smt_get_payload,
    .write_payload = smt_write_payload,
    .respond = smt_respond,
};

/*
 * The following API is intended to be used for NON SCMI messages.
 */
static const struct mod_smt_to_transport_api smt_mod_smt_to_transport_api = {
    .get_secure = smt_get_secure,
    .get_max_payload_size = smt_get_max_payload_size,
    .get_message_header = smt_get_message_header,
    .get_payload = smt_get_payload,
    .write_payload = smt_write_payload,
    .respond = smt_respond,
};

/*
 * Driver handler API
 */
static int smt_slave_handler(struct smt_channel_ctx *channel_ctx)
{
    struct mod_smt_memory *memory, *in, *out;
    size_t payload_size;
    int status;

    /* Check if we are already processing */
    if (channel_ctx->locked)
        return FWK_E_STATE;

    memory = ((struct mod_smt_memory*)channel_ctx->config->mailbox_address);
    in = channel_ctx->in;
    out = channel_ctx->out;

    /* Check we have ownership of the mailbox */
    if (memory->status & MOD_SMT_MAILBOX_STATUS_FREE_MASK) {
        FWK_LOG_ERR(
            "[SMT] Mailbox ownership error on channel %u",
            fwk_id_get_element_idx(channel_ctx->id));

        return FWK_E_STATE;
    }

    /* Commit to sending a response */
    channel_ctx->locked = true;

    /* Mirror mailbox contents in read and write buffers (Payload not copied) */
    *in  = *memory;
    *out = *memory;

    /* Ensure error bit is not set */
    out->status &= ~MOD_SMT_MAILBOX_STATUS_ERROR_MASK;

    /*
     * Verify:
     * 1. The length is at least as large as the message header
     * 2. The length, minus the size of the message header, is less than or
     *         equal to the maximum payload size
     *
     * Note: the payload size is permitted to be of size zero.
     */
    if ((in->length < sizeof(in->message_header)) ||
        ((in->length - sizeof(in->message_header))
         > channel_ctx->max_payload_size)) {

        out->status |= MOD_SMT_MAILBOX_STATUS_ERROR_MASK;

        if (channel_ctx->is_scmi_channel)
            status = channel_ctx->smt_signal.scmi_api->signal_error(
                         channel_ctx->service_id);
        else
            status = channel_ctx->smt_signal.signal_api->signal_error(
                         channel_ctx->service_id);

        return status;
    }

    /* Copy payload from shared memory to read buffer */
    payload_size = in->length - sizeof(in->message_header);
    memcpy(in->payload, memory->payload, payload_size);

    /* Let subscribed service handle the message */
    if (channel_ctx->is_scmi_channel)
        status = channel_ctx->smt_signal.scmi_api->signal_message(
                    channel_ctx->service_id);
    else
        status = channel_ctx->smt_signal.signal_api->signal_message(
                    channel_ctx->service_id);

    if (status != FWK_SUCCESS)
        return FWK_E_HANDLER;

    return FWK_SUCCESS;
}

static int smt_signal_message(fwk_id_t channel_id)
{
    struct smt_channel_ctx *channel_ctx;

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];

    if (!channel_ctx->smt_mailbox_ready) {
        /* Discard any message in the mailbox when not ready */
        FWK_LOG_ERR("[SMT] Message not valid");

        return FWK_SUCCESS;
    }

    switch (channel_ctx->config->type) {
    case MOD_SMT_CHANNEL_TYPE_MASTER:
        /* Not supported yet */
        assert(false);
        break;
    case MOD_SMT_CHANNEL_TYPE_SLAVE:
        return smt_slave_handler(channel_ctx);
        break;
    default:
        /* Invalid config */
        assert(false);
        break;
    }

    return FWK_SUCCESS;
}

static const struct mod_smt_driver_input_api driver_input_api = {
    .signal_message = smt_signal_message,
};

/*
 * Framework API
 */
static int smt_init(fwk_id_t module_id, unsigned int element_count,
                    const void *data)
{
    smt_ctx.channel_ctx_table = fwk_mm_calloc(element_count,
        sizeof(smt_ctx.channel_ctx_table[0]));
    smt_ctx.channel_count = element_count;

    return FWK_SUCCESS;
}

static int smt_channel_init(fwk_id_t channel_id, unsigned int unused,
                            const void *data)
{
    struct smt_channel_ctx *channel_ctx;

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(channel_id)];
    channel_ctx->config = (struct mod_smt_channel_config*)data;

    /* Validate channel config */
    if ((channel_ctx->config->type >= MOD_SMT_CHANNEL_TYPE_COUNT) ||
        (channel_ctx->config->mailbox_address == 0) ||
        (channel_ctx->config->mailbox_size == 0)) {
        assert(false);
        return FWK_E_DATA;
    }

    channel_ctx->id = channel_id;
    channel_ctx->in = fwk_mm_alloc(1, channel_ctx->config->mailbox_size);
    channel_ctx->out = fwk_mm_alloc(1, channel_ctx->config->mailbox_size);

    channel_ctx->max_payload_size = channel_ctx->config->mailbox_size -
        sizeof(struct mod_smt_memory);

    return FWK_SUCCESS;
}

static int smt_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct smt_channel_ctx *channel_ctx;

    if (round == 0) {
        if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
            return FWK_SUCCESS;

        channel_ctx = &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(id)];
        status = fwk_module_bind(channel_ctx->config->driver_id,
                                 channel_ctx->config->driver_api_id,
                                 &channel_ctx->driver_api);
        if (status != FWK_SUCCESS)
            return status;
        channel_ctx->driver_id = channel_ctx->config->driver_id;
    }

    if ((round == 1) && fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT)) {
        channel_ctx = &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(id)];

        if (fwk_id_is_equal(fwk_id_build_module_id(channel_ctx->service_id),
                        fwk_module_id_scmi)) {
            status = fwk_module_bind(channel_ctx->service_id,
                                    FWK_ID_API(FWK_MODULE_IDX_SCMI,
                                    MOD_SCMI_API_IDX_TRANSPORT),
                                    &channel_ctx->smt_signal.scmi_api);
            channel_ctx->is_scmi_channel = true;
        } else {
            status = fwk_module_bind(channel_ctx->service_id,
                                    channel_ctx->config->signal_api_id,
                                    &channel_ctx->smt_signal.signal_api);
            channel_ctx->is_scmi_channel = false;
        }
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

static int smt_process_bind_request(fwk_id_t source_id,
                                    fwk_id_t target_id,
                                    fwk_id_t api_id,
                                    const void **api)
{
    struct smt_channel_ctx *channel_ctx;

    /* Only bind to a channel (not the whole module) */
    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT)) {
        /* Tried to bind to something other than a specific channel */
        assert(false);
        return FWK_E_PARAM;
    }

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(target_id)];

    switch (fwk_id_get_api_idx(api_id)) {
    case MOD_SMT_API_IDX_DRIVER_INPUT:
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
        if (fwk_id_get_module_idx(channel_ctx->driver_id) ==
            fwk_id_get_module_idx(source_id) &&
            fwk_id_get_element_idx(channel_ctx->driver_id) ==
            fwk_id_get_element_idx(source_id)) {

            /* Ids are equal */
            *api = &driver_input_api;
        } else {
            /* A module that we did not bind to is trying to bind to us */
            assert(false);
            return FWK_E_ACCESS;
        }
        break;

    case MOD_SMT_API_IDX_SCMI_TRANSPORT:
        /* SCMI transport API */
        *api = &smt_mod_scmi_to_transport_api;
        channel_ctx->service_id = source_id;
        break;

    case MOD_SMT_API_IDX_TO_TRANSPORT:
        /* SMT transport API */
        *api = &smt_mod_smt_to_transport_api;
        channel_ctx->service_id = source_id;
        break;

    default:
        /* Invalid API */
        assert(false);
        return FWK_E_PARAM;
    }

    return FWK_SUCCESS;
}

static int smt_start(fwk_id_t id)
{
    struct smt_channel_ctx *ctx;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return FWK_SUCCESS;

    ctx = &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(id)];

    /* Register for power domain state transition notifications */
    return fwk_notification_subscribe(
        mod_pd_notification_id_power_state_transition,
        ctx->config->pd_source_id,
        id);
}

static int smt_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct mod_pd_power_state_transition_notification_params *params;
    struct smt_channel_ctx *channel_ctx;
    unsigned int notifications_sent;
    int status;

    assert(fwk_id_is_equal(event->id,
        mod_pd_notification_id_power_state_transition));
    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_ELEMENT));

    params = (struct mod_pd_power_state_transition_notification_params *)
        event->params;

    channel_ctx =
        &smt_ctx.channel_ctx_table[fwk_id_get_element_idx(event->target_id)];

    if (params->state != MOD_PD_STATE_ON) {
        if (params->state == MOD_PD_STATE_OFF)
            channel_ctx->smt_mailbox_ready = false;

        return FWK_SUCCESS;
    }

    if (channel_ctx->config->policies & MOD_SMT_POLICY_INIT_MAILBOX) {
        /* Initialize mailbox */
        *((struct mod_smt_memory *)channel_ctx->config->mailbox_address) =
            (struct mod_smt_memory) {
            .status = (1 << MOD_SMT_MAILBOX_STATUS_FREE_POS)
        };

        /* Notify that this mailbox is initialized */
        struct fwk_event smt_channels_initialized_notification = {
            .id = mod_smt_notification_id_initialized,
            .source_id = FWK_ID_NONE
        };

        channel_ctx->smt_mailbox_ready = true;

        status = fwk_notification_notify(&smt_channels_initialized_notification,
            &notifications_sent);
        if (status != FWK_SUCCESS)
            return status;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_smt = {
    .name = "smt",
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = MOD_SMT_API_IDX_COUNT,
    .notification_count = MOD_SMT_NOTIFICATION_IDX_COUNT,
    .init = smt_init,
    .element_init = smt_channel_init,
    .bind = smt_bind,
    .start = smt_start,
    .process_bind_request = smt_process_bind_request,
    .process_notification = smt_process_notification,
};

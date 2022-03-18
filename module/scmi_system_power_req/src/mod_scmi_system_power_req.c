/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_power_domain.h>
#include <mod_scmi.h>
#include <mod_scmi_system_power_req.h>

#include <fwk_assert.h>
#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_interrupt.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_string.h>

#include <stdbool.h>
#include <stdint.h>

/* Element context */
struct scmi_system_power_req_dev_ctx {
    /* Element configuration data pointer */
    const struct mod_scmi_system_power_req_dev_config *config;
};

/* Module context */
struct mod_scmi_system_power_req_ctx {
    /* token to track the sent messages */
    uint8_t token;

    /* SCMI System power requester element context table */
    struct scmi_system_power_req_dev_ctx *dev_ctx_table;

    /* Number of elements */
    unsigned int dev_count;

    /* SCMI send message API */
    const struct mod_scmi_from_protocol_req_api *scmi_api;

    /* System State - collated after Set State command responses */
    uint32_t state;

    /* System State Requested - the state that has been sent in the SCMI
     * command. */
    uint32_t state_requested;

    /* For the delayed response */
    uint32_t cookie;

    /* Whether or not the client requested a response */
    bool response_requested;

    /* Module state after a set state command */
    int32_t state_change_status;
};

static int scmi_system_power_req_state_set_handler(
    fwk_id_t service_id,
    const void *payload,
    size_t payload_size);

static int scmi_system_power_req_message_handler(
    fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id);

/*
 * Internal variables.
 */
static struct mod_scmi_system_power_req_ctx mod_ctx;

/*!
 * \brief SCMI System Power Protocol Message IDs
 */

enum scmi_sys_power_req_command_id {
    /*
     * SCMI Command ID of the System Power command
     * implemented in this module.
     */
    MOD_SCMI_SYS_POWER_REQ_STATE_SET = 0x003,
    MOD_SCMI_SYS_POWER_REQ_COMMAND_COUNT,
};

enum scmi_system_power_req_event_idx {
    /* Event to handle sync set state */
    SCMI_SPR_EVENT_IDX_SET_STATE = MOD_SCMI_SPR_EVENT_IDX_SET_STATE,
    SCMI_SPR_EVENT_IDX_SET_COMPLETE,
    SCMI_SPR_EVENT_IDX_COUNT,
};

/*
 * Parameters of the set state request event
 */
struct spr_set_state_request {
    /*
     * The composite state that defines the power state that the power domain,
     * target of the request, has to be put into and possibly the power states
     * the ancestors of the power domain have to be put into.
     */
    uint32_t state;

    /* The flags passed from the original SCMI System Power command.*/
    uint32_t flags;
};

static int (*handler_table[MOD_SCMI_SYS_POWER_REQ_COMMAND_COUNT])(
    fwk_id_t,
    const void *,
    size_t) = {
    [MOD_SCMI_SYS_POWER_REQ_STATE_SET] =
        scmi_system_power_req_state_set_handler,
};

static const unsigned int
    payload_size_table[MOD_SCMI_SYS_POWER_REQ_COMMAND_COUNT] = {
        [MOD_SCMI_SYS_POWER_REQ_STATE_SET] =
            (unsigned int)sizeof(struct scmi_sys_power_req_state_set_a2p),
    };

static_assert(
    FWK_ARRAY_SIZE(handler_table) == FWK_ARRAY_SIZE(payload_size_table),
    "[SCMI] System Power Req protocol table sizes not "
    "consistent");

/*
 * SCMI module -> SCMI system power requester module interface
 */
static int scmi_system_power_req_get_scmi_protocol_id(
    fwk_id_t protocol_id,
    uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = (uint8_t)MOD_SCMI_PROTOCOL_ID_SYS_POWER;

    return FWK_SUCCESS;
}

/*
 * System Power Requester Response handlers
 */
static int scmi_system_power_req_message_handler(
    fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    int ret_status, alt_status;

    fwk_assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(handler_table)) {
        return FWK_E_RANGE;
    }

    if (payload_size != payload_size_table[message_id]) {
        return FWK_E_PARAM;
    }

    if (handler_table[message_id] == NULL) {
        return FWK_E_PARAM;
    }
    ret_status = handler_table[message_id](service_id, payload, payload_size);

    alt_status = mod_ctx.scmi_api->response_message_handler(service_id);

    return (ret_status != FWK_SUCCESS) ? ret_status : alt_status;
}

static struct mod_scmi_to_protocol_api
    scmi_system_power_req_scmi_to_protocol_api = {
        .get_scmi_protocol_id = scmi_system_power_req_get_scmi_protocol_id,
        .message_handler = scmi_system_power_req_message_handler,
    };

/*
 * Return System Power Requester reading handler. This is the Set State response
 * handler.
 */
static int scmi_system_power_req_state_set_handler(
    fwk_id_t service_id,
    const void *payload,
    size_t payload_size)
{
    int ret_status;
    int status;

    ret_status = *((const int *)payload);

    if (ret_status == SCMI_SUCCESS) {
        mod_ctx.state = mod_ctx.state_requested;
    }

    mod_ctx.state_change_status = ret_status;

    if (mod_ctx.response_requested) {
        struct fwk_event_light req = (struct fwk_event_light){
            .id = FWK_ID_EVENT(
                FWK_MODULE_IDX_SCMI_SYSTEM_POWER_REQ,
                SCMI_SPR_EVENT_IDX_SET_COMPLETE),
            .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_SYSTEM_POWER_REQ),
        };

        status = fwk_put_event(&req);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return ret_status;
}

static int scmi_system_power_req_set_state(
    bool response_requested,
    uint32_t state,
    uint32_t flags)
{
    int status;

    struct fwk_event req;
    struct spr_set_state_request *req_params =
        (struct spr_set_state_request *)(&req.params);

    mod_ctx.state_requested = state;
    mod_ctx.response_requested = response_requested;

    req = (struct fwk_event){
        .id = system_power_requester_set_state_request,
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_SYSTEM_POWER_REQ),
        .response_requested = response_requested,
    };

    req_params->state = state;
    req_params->flags = flags;
    status = fwk_put_event(&req);

    if (status == FWK_SUCCESS) {
        return FWK_PENDING;
    } else {
        return status;
    }
}

static int process_set_state(fwk_id_t id, uint32_t state, uint32_t flags)
{
    uint8_t scmi_protocol_id = (uint8_t)MOD_SCMI_PROTOCOL_ID_SYS_POWER;
    uint8_t scmi_message_id = (uint8_t)MOD_SCMI_SYS_POWER_REQ_STATE_SET;

    struct scmi_system_power_req_dev_ctx *ctx = &(mod_ctx.dev_ctx_table[0]);

    const struct scmi_sys_power_req_state_set_a2p payload = {
        .flags = flags,
        .system_state = state,
    };

    return mod_ctx.scmi_api->scmi_send_message(
        scmi_message_id,
        scmi_protocol_id,
        mod_ctx.token++,
        ctx->config->service_id,
        (const void *)&payload,
        sizeof(payload),
        true);
}

static int scmi_system_power_req_get_state(uint32_t *state)
{
    if (state != NULL) {
        /* Set State command will cache the response */
        *state = mod_ctx.state;

        return FWK_SUCCESS;
    }
    return FWK_E_PARAM;
}

static const struct mod_system_power_requester_api
    scmi_system_power_req_driver_api = {
        .set_req_state = scmi_system_power_req_set_state,
        .get_req_state = scmi_system_power_req_get_state,
    };

/*
 * Framework handlers
 */
static int scmi_system_power_req_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *data)
{
    mod_ctx.state = MOD_PD_STATE_ON;

    /* We definitely need elements in this module. */
    if (element_count != 1) {
        return FWK_E_SUPPORT;
    }

    mod_ctx.dev_count = element_count;
    mod_ctx.dev_ctx_table =
        fwk_mm_calloc(element_count, sizeof(mod_ctx.dev_ctx_table[0]));

    return FWK_SUCCESS;
}

static int scmi_system_power_req_elem_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    struct scmi_system_power_req_dev_ctx *dev_ctx;

    if (fwk_id_get_element_idx(element_id) >= mod_ctx.dev_count) {
        return FWK_E_PARAM;
    }

    dev_ctx = &mod_ctx.dev_ctx_table[fwk_id_get_element_idx(element_id)];

    if (data == NULL) {
        return FWK_E_PANIC;
    }

    const struct mod_scmi_system_power_req_dev_config *mod_sys_pow_req_config =
        (const struct mod_scmi_system_power_req_dev_config *)data;

    dev_ctx->config = mod_sys_pow_req_config;

    return FWK_SUCCESS;
}

static int scmi_system_power_req_bind(fwk_id_t id, unsigned int round)
{
    int status = FWK_SUCCESS;

    if (round == 0) {
        if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
            status = fwk_module_bind(
                FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
                FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL_REQ),
                &mod_ctx.scmi_api);
        }
    }
    return status;
}

static int scmi_system_power_req_bind_request(
    fwk_id_t requester_id,
    fwk_id_t unused,
    fwk_id_t api_id,
    const void **api)
{
    if (fwk_id_is_equal(api_id, mod_sys_power_req_scmi_api_id)) {
        if (!fwk_id_is_equal(
                fwk_id_build_module_id(requester_id), fwk_module_id_scmi)) {
            return FWK_E_ACCESS;
        }
        *api = &scmi_system_power_req_scmi_to_protocol_api;
    } else if (fwk_id_is_equal(api_id, mod_sys_power_req_api_id)) {
        *api = &scmi_system_power_req_driver_api;
    } else {
        return FWK_E_SUPPORT;
    }

    return FWK_SUCCESS;
}

static int scmi_system_power_req_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    struct fwk_event set_req_event;
    int status;
    struct spr_set_state_request *event_params =
        (struct spr_set_state_request *)(event->params);

    enum scmi_system_power_req_event_idx event_id_type =
        (enum scmi_system_power_req_event_idx)fwk_id_get_event_idx(event->id);

    switch (event_id_type) {
    case SCMI_SPR_EVENT_IDX_SET_STATE:
        if (event->response_requested) {
            /*
             * We keep the cookie event of the request that triggers the
             * state change.
             */
            mod_ctx.cookie = event->cookie;
            resp->is_delayed_response = true;
        }
        process_set_state(event->id, event_params->state, event_params->flags);
        return FWK_SUCCESS;

    case SCMI_SPR_EVENT_IDX_SET_COMPLETE:
        status = fwk_get_delayed_response(
            event->target_id, mod_ctx.cookie, &set_req_event);
        if (status != FWK_SUCCESS) {
            return status;
        }

        event_params->state = mod_ctx.state_change_status;
        return fwk_put_event(&set_req_event);

    default:
        return FWK_E_PARAM;
    }
}

const struct fwk_module module_scmi_system_power_req = {
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .api_count = (unsigned int)MOD_SYS_POW_REQ_API_IDX_COUNT,
    .event_count = (unsigned int)SCMI_SPR_EVENT_IDX_COUNT,
    .init = scmi_system_power_req_init,
    .element_init = scmi_system_power_req_elem_init,
    .bind = scmi_system_power_req_bind,
    .process_bind_request = scmi_system_power_req_bind_request,
    .process_event = scmi_system_power_req_process_event,
};

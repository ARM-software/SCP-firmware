/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI Agent Support.
 */

#include <mod_scmi_agent.h>
#include <mod_smt.h>

#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <fwk_thread.h>

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

/* SCMI agent context */
struct scmi_agent_ctx {
    /* Pointer to agent configuration data */
    struct mod_scmi_agent_config *config;

    /* Pointer to received payload data */
    const void *payload;

    /* Size of received payload */
    size_t size;
};

/* Module context */
struct mod_scmi_agent_module_ctx {
    /* Pointer to agent configuration table */
    struct scmi_agent_ctx *agent_ctx_table;

    /* SMT API pointer */
    const struct mod_scmi_agent_to_transport_api *smt_api;
};

enum scmi_agent_event {
    MOD_SCMI_AGENT_EVENT_RUN,
    MOD_SCMI_AGENT_EVENT_COUNT,
};

static struct mod_scmi_agent_module_ctx ctx;

static int _scmi_agent_transact(fwk_id_t agent_id,
                               struct mod_smt_command_config *cmd)
{
    struct scmi_agent_ctx *agent_ctx;
    int status;

    agent_ctx = &ctx.agent_ctx_table[fwk_id_get_element_idx(agent_id)];

    /* Check if channel is free */
    if (!ctx.smt_api->is_channel_free(agent_ctx->config->transport_id)) {
        FWK_LOG_ERR("[SCMI AGENT] Channel Busy!");
        return FWK_E_BUSY;
    }

    /* Send SCMI command to platform */
    status = ctx.smt_api->send(agent_ctx->config->transport_id, cmd);
    if (status != FWK_SUCCESS) {
        ctx.smt_api->put_channel(agent_ctx->config->transport_id);
        return status;
    }

    /* Wait until platform responds */
    while (!ctx.smt_api->is_channel_free(agent_ctx->config->transport_id))
        ;

    /* Get response payload */
    status = ctx.smt_api->get_payload(agent_ctx->config->transport_id,
                                      &agent_ctx->payload, &agent_ctx->size);
    if (status != FWK_SUCCESS)
        return status;

    /* Release channel */
    status = ctx.smt_api->put_channel(agent_ctx->config->transport_id);
    if (status != FWK_SUCCESS)
        return status;

    return FWK_SUCCESS;
}

/*
 * SCMI Management Agent API interface
 */
static int agent_get_protocol_version(fwk_id_t agent_id, uint32_t *version)
{
    int status;
    struct scmi_agent_ctx *agent_ctx;
    struct mod_smt_command_config cmd = {
        .protocol_id = SCMI_PROTOCOL_ID_MANAGEMENT,
        .message_id = SCMI_MANAGEMENT_PROTOCOL_VERSION_GET,
        .payload = NULL,
        .size = 0,
    };

    if (version == NULL)
        return FWK_E_PARAM;

    status = _scmi_agent_transact(agent_id, &cmd);
    if (status != FWK_SUCCESS)
        return status;

    agent_ctx = &ctx.agent_ctx_table[fwk_id_get_element_idx(agent_id)];
    *version = *(((uint32_t *)agent_ctx->payload) + 1);

    return FWK_SUCCESS;
}

static int agent_get_clock_status(fwk_id_t agent_id, uint32_t *clock_status)
{
    int status;
    struct scmi_agent_ctx *agent_ctx;
    struct mod_smt_command_config cmd = {
        .protocol_id = SCMI_PROTOCOL_ID_MANAGEMENT,
        .message_id = SCMI_MANAGEMENT_CLOCK_STATUS_GET,
        .payload = NULL,
        .size = 0,
    };

    if (clock_status == NULL)
        return FWK_E_PARAM;

    status = _scmi_agent_transact(agent_id, &cmd);
    if (status != FWK_SUCCESS)
        return status;

    agent_ctx = &ctx.agent_ctx_table[fwk_id_get_element_idx(agent_id)];
    *clock_status = *(((uint32_t *)agent_ctx->payload) + 1);

    return FWK_SUCCESS;
}

static int agent_get_chipid_info(fwk_id_t agent_id,
                                 uint8_t *multichip_mode,
                                 uint8_t *chipid)
{
    int status;
    struct scmi_agent_ctx *agent_ctx;
    struct mod_smt_command_config cmd = {
        .protocol_id = SCMI_PROTOCOL_ID_MANAGEMENT,
        .message_id = SCMI_MANAGEMENT_CHIPID_INFO_GET,
        .payload = NULL,
        .size = 0,
    };

    if ((multichip_mode == NULL) || (chipid == NULL))
        return FWK_E_PARAM;

    status = _scmi_agent_transact(agent_id, &cmd);
    if (status != FWK_SUCCESS)
        return status;

    agent_ctx = &ctx.agent_ctx_table[fwk_id_get_element_idx(agent_id)];
    *multichip_mode = (uint8_t)(*(((uint32_t *)agent_ctx->payload) + 1));
    *chipid = (uint8_t)(*(((uint32_t *)agent_ctx->payload) + 2));

    return FWK_SUCCESS;
}

struct mod_scmi_agent_api scmi_agent_api = {
    .get_protocol_version = agent_get_protocol_version,
    .get_clock_status = agent_get_clock_status,
    .get_chipid_info = agent_get_chipid_info,
};

/*
 * Functions fulfilling the framework's module interface
 */

static int scmi_agent_init(fwk_id_t module_id, unsigned int agent_count,
    const void *unused2)
{
    ctx.agent_ctx_table = fwk_mm_calloc(agent_count,
                                        sizeof(ctx.agent_ctx_table[0]));

    return FWK_SUCCESS;
}

static int scmi_agent_element_init(fwk_id_t agent_id, unsigned int unused,
                             const void *data)
{
    struct scmi_agent_ctx *agent_ctx;
    struct mod_scmi_agent_config *config =
        (struct mod_scmi_agent_config *)data;

    if (config == NULL)
        return FWK_E_PARAM;

    agent_ctx = &ctx.agent_ctx_table[fwk_id_get_element_idx(agent_id)];
    agent_ctx->config = config;

    return FWK_SUCCESS;
}

static int scmi_agent_bind(fwk_id_t id, unsigned int round)
{
    struct scmi_agent_ctx *agent_ctx;

    if (round == 0) {
        if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
            return FWK_SUCCESS;

        agent_ctx = &ctx.agent_ctx_table[fwk_id_get_element_idx(id)];
        return fwk_module_bind(agent_ctx->config->transport_id,
                               agent_ctx->config->transport_api_id,
                               &ctx.smt_api);
    }
    return FWK_SUCCESS;
}

static int scmi_agent_start(fwk_id_t id)
{
    /* Process only in the element start stage */
    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))
        return FWK_SUCCESS;

    struct fwk_event event = {
        .source_id = id,
        .target_id = id,
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_SCMI_AGENT,
                           MOD_SCMI_AGENT_EVENT_RUN),
    };

    return fwk_thread_put_event(&event);
}

static int scmi_agent_process_bind_request(fwk_id_t source_id,
                                           fwk_id_t target_id,
                                           fwk_id_t api_id,
                                           const void **api)
{
    *api = &scmi_agent_api;
    return FWK_SUCCESS;
}

static int scmi_agent_process_event(const struct fwk_event *event,
                                    struct fwk_event *resp)
{
    int status;
    uint32_t temp = 0;

    status = agent_get_protocol_version(event->target_id, &temp);
    if (status != FWK_SUCCESS)
        return status;

    FWK_LOG_INFO(
        "[SCMI AGENT] Found management protocol version: 0x%" PRIu32, temp);

    return status;
}

const struct fwk_module module_scmi_agent = {
    .name = "SCMI AGENT",
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = MOD_SCMI_AGENT_API_IDX_COUNT,
    .event_count = MOD_SCMI_AGENT_EVENT_COUNT,
    .init = scmi_agent_init,
    .element_init = scmi_agent_element_init,
    .bind = scmi_agent_bind,
    .start = scmi_agent_start,
    .process_bind_request = scmi_agent_process_bind_request,
    .process_event = scmi_agent_process_event,
};

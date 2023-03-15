/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI Agent Support.
 */

#include <mod_n1sdp_mcp_system.h>
#include <mod_scmi.h>
#include <mod_scmi_agent.h>

#include <fwk_core.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

/* SCMI agent context */
struct scmi_agent_ctx {
    /* Pointer to agent configuration data */
    struct mod_scmi_agent_config *config;
};

/* Module context */
struct mod_scmi_agent_module_ctx {
    /* Pointer to agent configuration table */
    struct scmi_agent_ctx *agent_ctx_table;

    /* SCMI API pointer */
    const struct mod_scmi_from_protocol_req_api *scmi_api;
};

static const unsigned int
    payload_size_table[SCMI_MANAGEMENT_MESSAGE_ID_COUNT] = {
        [SCMI_MANAGEMENT_PROTOCOL_VERSION_GET] =
            PROTOCOL_VERSION_GET_RESPONSE_PAYLOAD_SIZE,
        [SCMI_MANAGEMENT_CLOCK_STATUS_GET] =
            CLOCK_STATUS_GET_RESPONSE_PAYLOAD_SIZE,
        [SCMI_MANAGEMENT_CHIPID_INFO_GET] =
            CHIPID_INFO_GET_RESPONSE_PAYLOAD_SIZE,
    };

static struct mod_scmi_agent_module_ctx ctx;

/*
 * Protocol Version
 */
static int scmi_agent_protocol_version_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    struct fwk_event event;
    struct mod_scmi_agent_protocol_version_event_param *event_param =
        (struct mod_scmi_agent_protocol_version_event_param *)event.params;

    event = (struct fwk_event){
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_MCP_SYSTEM),
        .id = FWK_ID_EVENT(
            FWK_MODULE_IDX_N1SDP_MCP_SYSTEM,
            MOD_MCP_SYSTEM_EVENT_PROTOCOL_VERSION_GET),
    };

    /* Get protocol version from payload which is second member in response */
    event_param->protocol_version = *(((uint32_t *)payload) + 1);

    return fwk_put_event(&event);
}

/*
 * Clock Status Get
 */
static int scmi_agent_clock_status_get_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    struct fwk_event event;
    struct mod_scmi_agent_clock_status_event_param *event_param =
        (struct mod_scmi_agent_clock_status_event_param *)event.params;

    event = (struct fwk_event){
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_MCP_SYSTEM),
        .id = FWK_ID_EVENT(
            FWK_MODULE_IDX_N1SDP_MCP_SYSTEM,
            MOD_MCP_SYSTEM_EVENT_CLOCK_STATUS_GET),
    };

    /* Get clock status from payload which is second member in response */
    event_param->clock_status = *(((uint32_t *)payload) + 1);

    return fwk_put_event(&event);
}

/*
 * CHIP ID Information Get
 */
static int scmi_agent_chipid_info_get_handler(
    fwk_id_t service_id,
    const uint32_t *payload)
{
    struct fwk_event event;
    struct mod_scmi_agent_chipid_info_event_param *event_param =
        (struct mod_scmi_agent_chipid_info_event_param *)event.params;

    event = (struct fwk_event){
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_MCP_SYSTEM),
        .id = FWK_ID_EVENT(
            FWK_MODULE_IDX_N1SDP_MCP_SYSTEM,
            MOD_MCP_SYSTEM_EVENT_CHIPID_INFO_GET),
    };

    /* Get multichip mode from payload which is second member in response */
    event_param->multichip_mode = (uint8_t)(*(((uint32_t *)payload) + 1));
    /* Get chipid from payload which is third member in response */
    event_param->chipid = (uint8_t)(*(((uint32_t *)payload) + 2));

    return fwk_put_event(&event);
}

static int get_scmi_protocol_id(fwk_id_t protocol_id, uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = SCMI_PROTOCOL_ID_MANAGEMENT;
    return FWK_SUCCESS;
}

static int (*const handler_table[])(fwk_id_t, const uint32_t *) = {
    [SCMI_MANAGEMENT_PROTOCOL_VERSION_GET] =
        scmi_agent_protocol_version_handler,
    [SCMI_MANAGEMENT_CLOCK_STATUS_GET] = scmi_agent_clock_status_get_handler,
    [SCMI_MANAGEMENT_CHIPID_INFO_GET] = scmi_agent_chipid_info_get_handler,
};

static int scmi_message_handler(
    fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    int handler_status, resp_status;

    fwk_assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(handler_table)) {
        return FWK_E_RANGE;
    }

    if (payload_size != payload_size_table[message_id]) {
        return FWK_E_PARAM;
    }

    handler_status = handler_table[message_id](service_id, payload);

    resp_status = ctx.scmi_api->response_message_handler(service_id);

    return (handler_status != FWK_SUCCESS) ? handler_status : resp_status;
}

static struct mod_scmi_to_protocol_api scmi_agent_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = get_scmi_protocol_id,
    .message_handler = scmi_message_handler
};

/*
 * SCMI Management Agent API interface
 */
static int agent_get_protocol_version(fwk_id_t agent_id)
{
    int status;
    struct scmi_agent_ctx *agent_ctx;

    agent_ctx = &ctx.agent_ctx_table[fwk_id_get_element_idx(agent_id)];
    status = ctx.scmi_api->scmi_send_message(
        SCMI_MANAGEMENT_PROTOCOL_VERSION_GET,
        SCMI_PROTOCOL_ID_MANAGEMENT,
        0,
        agent_ctx->config->service_id,
        NULL,
        0,
        true);

    return status;
}

static int agent_get_clock_status(fwk_id_t agent_id)
{
    int status;
    struct scmi_agent_ctx *agent_ctx;

    agent_ctx = &ctx.agent_ctx_table[fwk_id_get_element_idx(agent_id)];
    status = ctx.scmi_api->scmi_send_message(
        SCMI_MANAGEMENT_CLOCK_STATUS_GET,
        SCMI_PROTOCOL_ID_MANAGEMENT,
        0,
        agent_ctx->config->service_id,
        NULL,
        0,
        true);

    return status;
}

static int agent_get_chipid_info(fwk_id_t agent_id)
{
    int status;
    struct scmi_agent_ctx *agent_ctx;

    agent_ctx = &ctx.agent_ctx_table[fwk_id_get_element_idx(agent_id)];
    status = ctx.scmi_api->scmi_send_message(
        SCMI_MANAGEMENT_CHIPID_INFO_GET,
        SCMI_PROTOCOL_ID_MANAGEMENT,
        0,
        agent_ctx->config->service_id,
        NULL,
        0,
        true);

    return status;
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
    if ((round == 0) && fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
            FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL_REQ),
            &ctx.scmi_api);
    }
    return FWK_SUCCESS;
}

static int scmi_agent_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    if (fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI))) {
        *api = &scmi_agent_mod_scmi_to_protocol_api;

    } else {
        *api = &scmi_agent_api;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_scmi_agent = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .api_count = MOD_SCMI_AGENT_API_IDX_COUNT,
    .init = scmi_agent_init,
    .element_init = scmi_agent_element_init,
    .bind = scmi_agent_bind,
    .process_bind_request = scmi_agent_process_bind_request,
};

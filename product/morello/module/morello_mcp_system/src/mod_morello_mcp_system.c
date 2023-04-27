/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     MORELLO MCP System Support.
 */

#include "config_clock.h"
#include "morello_mcp_scp.h"
#include "morello_mcp_software_mmap.h"

#include <mod_clock.h>
#include <mod_morello_mcp_system.h>
#include <mod_pik_clock.h>
#include <mod_power_domain.h>
#include <mod_scmi_agent.h>
#include <mod_timer.h>

#include <fwk_core.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <inttypes.h>
#include <stdint.h>

/* Module context */
struct morello_mcp_system_ctx {
    /* SCMI agent API pointer */
    const struct mod_scmi_agent_api *scmi_agent_api;

    /* PIK clock API - MCP core clock */
    const struct mod_clock_drv_api *pik_coreclk_api;

    /* PIK clock API - MCP AXI clock */
    const struct mod_clock_drv_api *pik_axiclk_api;

    /* Timer API */
    struct mod_timer_api *timer_api;

    /* Timer alarm API pointer */
    const struct mod_timer_alarm_api *alarm_api;

    /* Module configuration data */
    struct mod_morello_mcp_system_module_config *module_config;
};

static struct morello_mcp_system_ctx morello_mcp_system_ctx;

static int put_self_event(enum mcp_system_event event_type)
{
    struct fwk_event event = {
        .source_id = FWK_ID_MODULE(FWK_MODULE_IDX_MORELLO_MCP_SYSTEM),
        .target_id = FWK_ID_MODULE(FWK_MODULE_IDX_MORELLO_MCP_SYSTEM),
        .id = FWK_ID_EVENT(FWK_MODULE_IDX_MORELLO_MCP_SYSTEM, event_type),
    };

    return fwk_put_event(&event);
}

static void alarm_callback(uintptr_t param)
{
#if FWK_LOG_LEVEL < FWK_LOG_LEVEL_CRIT
    enum mcp_system_event event_id_type = (enum mcp_system_event)param;
#endif
    FWK_LOG_ERR(
        "[MCP SYSTEM] For event ID - %d, No response received. Timing Out!",
        (int)event_id_type);
}

static void set_alarm(int alarm_delay, enum mcp_system_event event_id_type)
{
    morello_mcp_system_ctx.alarm_api->start(
        morello_mcp_system_ctx.module_config->alarm_id,
        alarm_delay,
        MOD_TIMER_ALARM_TYPE_ONCE,
        alarm_callback,
        event_id_type);
}

static void disable_alarm(void)
{
    morello_mcp_system_ctx.alarm_api->stop(
        morello_mcp_system_ctx.module_config->alarm_id);
}

static bool scp_handshake_wait_condition(void *unused)
{
    return (
        *(FWK_R uint32_t *)SCMI_PAYLOAD_SCP_TO_MCP_S ==
        MORELLO_SCP_MCP_HANDSHAKE_PATTERN);
}

static int get_protocol_version_response_handler(const struct fwk_event *event)
{
    uint32_t protocol_version = 0;
    struct mod_scmi_agent_protocol_version_event_param *event_param;
    event_param =
        (struct mod_scmi_agent_protocol_version_event_param *)event->params;

    disable_alarm();
    protocol_version = event_param->protocol_version;

    if (protocol_version != SCMI_PROTOCOL_VERSION_MANAGEMENT) {
        FWK_LOG_ERR(
            "Invalid protocol version - 0x%x received!",
            (unsigned int)protocol_version);
        return FWK_E_DATA;
    }

    FWK_LOG_INFO(
        "[MCP SYSTEM] Found management protocol version: 0x%x",
        (unsigned int)protocol_version);
    return put_self_event(MOD_MCP_SYSTEM_EVENT_CLOCK_STATUS_GET);
}

static int get_clock_status_response_handler(const struct fwk_event *event)
{
    int status;
    uint32_t clock_status = 0;
    struct mod_scmi_agent_clock_status_event_param *event_param;
    event_param =
        (struct mod_scmi_agent_clock_status_event_param *)event->params;

    disable_alarm();
    clock_status = event_param->clock_status;

    if (clock_status != SCP_CLOCK_STATUS_INITIALIZED) {
        FWK_LOG_ERR(
            "Invalid clock status - 0x%x received!",
            (unsigned int)clock_status);
        return FWK_E_DATA;
    }

    FWK_LOG_INFO(
        "[MCP SYSTEM] SCP clock status: 0x%x", (unsigned int)clock_status);

    status = morello_mcp_system_ctx.pik_coreclk_api->process_power_transition(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_MCP_CORECLK),
        MOD_PD_STATE_ON);
    if (status != FWK_SUCCESS) {
        return status;
    }

    status = morello_mcp_system_ctx.pik_coreclk_api->process_power_transition(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_MCP_AXICLK),
        MOD_PD_STATE_ON);
    if (status != FWK_SUCCESS) {
        return status;
    }

    FWK_LOG_INFO("[MCP SYSTEM] MCP PIK clocks configured");
    return put_self_event(MOD_MCP_SYSTEM_EVENT_CHIPID_INFO_GET);
}

static int get_chipid_info_response_handler(const struct fwk_event *event)
{
    uint8_t multichip_mode = 0;
    uint8_t chipid = 0;
    struct mod_scmi_agent_chipid_info_event_param *event_param;
    event_param =
        (struct mod_scmi_agent_chipid_info_event_param *)event->params;

    disable_alarm();
    multichip_mode = event_param->multichip_mode;
    chipid = event_param->chipid;

    if ((chipid != 0) && (multichip_mode != 1)) {
        FWK_LOG_ERR(
            "Invalid combination chipid - 0x%x multichip_mode - 0x%x received!",
            chipid,
            multichip_mode);
        return FWK_E_DATA;
    }

    FWK_LOG_INFO(
        "[MCP SYSTEM] MC Mode: 0x%x CHIPID: 0x%x", multichip_mode, chipid);
    return put_self_event(MOD_MCP_SYSTEM_EVENT_INITIALIZATION_COMPLETE);
}

/*
 * Functions fulfilling the framework's module interface
 */

static int morello_mcp_system_init(
    fwk_id_t module_id,
    unsigned int unused,
    const void *data)
{
    morello_mcp_system_ctx.module_config =
        (struct mod_morello_mcp_system_module_config *)data;

    return FWK_SUCCESS;
}

static int morello_mcp_system_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 0) {
        status = fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_AGENT),
            FWK_ID_API(
                FWK_MODULE_IDX_SCMI_AGENT, MOD_SCMI_AGENT_API_IDX_SYSTEM),
            &morello_mcp_system_ctx.scmi_agent_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = fwk_module_bind(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_MCP_CORECLK),
            FWK_ID_API(FWK_MODULE_IDX_PIK_CLOCK, MOD_PIK_CLOCK_API_TYPE_CLOCK),
            &morello_mcp_system_ctx.pik_coreclk_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = fwk_module_bind(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            FWK_ID_API(FWK_MODULE_IDX_TIMER, MOD_TIMER_API_IDX_TIMER),
            &morello_mcp_system_ctx.timer_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        status = fwk_module_bind(
            morello_mcp_system_ctx.module_config->alarm_id,
            MOD_TIMER_API_ID_ALARM,
            &morello_mcp_system_ctx.alarm_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        return fwk_module_bind(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_MCP_AXICLK),
            FWK_ID_API(FWK_MODULE_IDX_PIK_CLOCK, MOD_PIK_CLOCK_API_TYPE_CLOCK),
            &morello_mcp_system_ctx.pik_axiclk_api);
    }

    return FWK_SUCCESS;
}

static int morello_mcp_system_start(fwk_id_t id)
{
    return put_self_event(MOD_MCP_SYSTEM_EVENT_SCP_HANDSHAKE);
}

static int morello_mcp_system_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    int status = FWK_SUCCESS;
    enum mcp_system_event event_id_type = fwk_id_get_event_idx(event->id);

    switch (event_id_type) {
    case MOD_MCP_SYSTEM_EVENT_SCP_HANDSHAKE:
        FWK_LOG_INFO("[MCP SYSTEM] Waiting for handshake pattern from SCP...");
        status = morello_mcp_system_ctx.timer_api->wait(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_TIMER, 0),
            MORELLO_SCP_MCP_HANDSHAKE_TIMEOUT_MICROSEC,
            scp_handshake_wait_condition,
            NULL);

        if (status == FWK_SUCCESS) {
            FWK_LOG_INFO("[MCP SYSTEM] Handshake pattern received from SCP.");
            status = put_self_event(MOD_MCP_SYSTEM_EVENT_PROTOCOL_VERSION_GET);
        } else if (status == FWK_E_TIMEOUT) {
            FWK_LOG_ERR(
                "[MCP SYSTEM] No handhshake pattern received from SCP. Timing "
                "out!");
        } else {
            FWK_LOG_ERR("[MCP SYSTEM] Timer initiation failed!");
        }
        break;

    case MOD_MCP_SYSTEM_EVENT_PROTOCOL_VERSION_GET:
        if (fwk_id_is_equal(
                event->source_id,
                FWK_ID_MODULE(FWK_MODULE_IDX_MORELLO_MCP_SYSTEM))) {
            status =
                morello_mcp_system_ctx.scmi_agent_api->get_protocol_version(
                    FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_AGENT, 0));
            set_alarm(
                MORELLO_SCP_AGENT_SCMI_RESPONSE_TIMEOUT_MILLISEC,
                MOD_MCP_SYSTEM_EVENT_PROTOCOL_VERSION_GET);
        } else {
            status = get_protocol_version_response_handler(event);
        }
        break;

    case MOD_MCP_SYSTEM_EVENT_CLOCK_STATUS_GET:
        if (fwk_id_is_equal(
                event->source_id,
                FWK_ID_MODULE(FWK_MODULE_IDX_MORELLO_MCP_SYSTEM))) {
            status = morello_mcp_system_ctx.scmi_agent_api->get_clock_status(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_AGENT, 0));
            set_alarm(
                MORELLO_SCP_AGENT_SCMI_RESPONSE_TIMEOUT_MILLISEC,
                MOD_MCP_SYSTEM_EVENT_CLOCK_STATUS_GET);
        } else {
            status = get_clock_status_response_handler(event);
        }
        break;

    case MOD_MCP_SYSTEM_EVENT_CHIPID_INFO_GET:
        if (fwk_id_is_equal(
                event->source_id,
                FWK_ID_MODULE(FWK_MODULE_IDX_MORELLO_MCP_SYSTEM))) {
            status = morello_mcp_system_ctx.scmi_agent_api->get_chipid_info(
                FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_AGENT, 0));
            set_alarm(
                MORELLO_SCP_AGENT_SCMI_RESPONSE_TIMEOUT_MILLISEC,
                MOD_MCP_SYSTEM_EVENT_CHIPID_INFO_GET);
        } else {
            status = get_chipid_info_response_handler(event);
        }
        break;

    case MOD_MCP_SYSTEM_EVENT_INITIALIZATION_COMPLETE:
        FWK_LOG_INFO("[MCP SYSTEM] MCP Initialization completed");
        break;

    default:
        break;
    }

    return status;
}

const struct fwk_module module_morello_mcp_system = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 0,
    .event_count = MOD_MCP_SYSTEM_EVENT_COUNT,
    .init = morello_mcp_system_init,
    .bind = morello_mcp_system_bind,
    .process_event = morello_mcp_system_process_event,
    .start = morello_mcp_system_start,
};

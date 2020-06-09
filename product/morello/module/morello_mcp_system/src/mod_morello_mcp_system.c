/*
 * Arm SCP/MCP Software
 * Copyright (c) 2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     MORELLO MCP System Support.
 */

#include "config_clock.h"

#include <mod_clock.h>
#include <mod_morello_mcp_system.h>
#include <mod_pik_clock.h>
#include <mod_power_domain.h>
#include <mod_scmi_agent.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_thread.h>

#include <inttypes.h>
#include <stdint.h>

/* Module context */
struct morello_mcp_system_ctx {
    /* SCMI agent API pointer */
    const struct mod_scmi_agent_api *scmi_api;

    /* PIK clock API - MCP core clock */
    const struct mod_clock_drv_api *pik_coreclk_api;

    /* PIK clock API - MCP AXI clock */
    const struct mod_clock_drv_api *pik_axiclk_api;
};

static struct morello_mcp_system_ctx morello_mcp_system_ctx;
const struct fwk_module_config config_morello_mcp_system = { 0 };

/*
 * Functions fulfilling the framework's module interface
 */

static int morello_mcp_system_init(
    fwk_id_t module_id,
    unsigned int unused,
    const void *unused2)
{
    return FWK_SUCCESS;
}

static int morello_mcp_system_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 0) {
        status = fwk_module_bind(
            FWK_ID_MODULE(FWK_MODULE_IDX_SCMI_AGENT),
            FWK_ID_API(FWK_MODULE_IDX_SCMI_AGENT, 0),
            &morello_mcp_system_ctx.scmi_api);
        if (status != FWK_SUCCESS)
            return status;

        status = fwk_module_bind(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_MCP_CORECLK),
            FWK_ID_API(FWK_MODULE_IDX_PIK_CLOCK, MOD_PIK_CLOCK_API_TYPE_CLOCK),
            &morello_mcp_system_ctx.pik_coreclk_api);
        if (status != FWK_SUCCESS)
            return status;

        return fwk_module_bind(
            FWK_ID_ELEMENT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_MCP_AXICLK),
            FWK_ID_API(FWK_MODULE_IDX_PIK_CLOCK, MOD_PIK_CLOCK_API_TYPE_CLOCK),
            &morello_mcp_system_ctx.pik_axiclk_api);
    }

    return FWK_SUCCESS;
}

static int morello_mcp_system_start(fwk_id_t id)
{
    struct fwk_event event = {
        .source_id = id,
        .target_id = id,
        .id = FWK_ID_EVENT(
            FWK_MODULE_IDX_MORELLO_MCP_SYSTEM, MOD_MCP_SYSTEM_EVENT_RUN),
    };

    return fwk_thread_put_event(&event);
}

static int morello_mcp_system_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
    int status;
    uint32_t clock_status = 0;
    uint8_t mc_mode = 0;
    uint8_t chipid = 0;

    status = morello_mcp_system_ctx.scmi_api->get_clock_status(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_AGENT, 0), &clock_status);
    if (status != FWK_SUCCESS)
        return status;

    FWK_LOG_INFO(
        "[MCP SYSTEM] SCP clock status: 0x%x", (unsigned int)clock_status);

    status = morello_mcp_system_ctx.pik_coreclk_api->process_power_transition(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_MCP_CORECLK),
        MOD_PD_STATE_ON);
    if (status != FWK_SUCCESS)
        return FWK_SUCCESS;

    status = morello_mcp_system_ctx.pik_coreclk_api->process_power_transition(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_PIK_CLOCK, CLOCK_PIK_IDX_MCP_AXICLK),
        MOD_PD_STATE_ON);
    if (status != FWK_SUCCESS)
        return FWK_SUCCESS;

    FWK_LOG_INFO("[MCP SYSTEM] MCP PIK clocks configured");

    status = morello_mcp_system_ctx.scmi_api->get_chipid_info(
        FWK_ID_ELEMENT(FWK_MODULE_IDX_SCMI_AGENT, 0), &mc_mode, &chipid);
    if (status != FWK_SUCCESS)
        return status;

    FWK_LOG_INFO("[MCP SYSTEM] MC Mode: 0x%x CHIPID: 0x%x", mc_mode, chipid);

    return status;
}

const struct fwk_module module_morello_mcp_system = {
    .name = "MORELLO_MCP_SYSTEM",
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 0,
    .event_count = MOD_MCP_SYSTEM_EVENT_COUNT,
    .init = morello_mcp_system_init,
    .bind = morello_mcp_system_bind,
    .process_event = morello_mcp_system_process_event,
    .start = morello_mcp_system_start,
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI System Power unit test configuration.
 */

#include <Mockmod_scmi_power_domain_extra.h>

#include <mod_power_domain.h>
#include <mod_scmi.h>
#include <mod_scmi_power_domain.h>

static struct mod_pd_restricted_api pd_api_ut = {
    .get_domain_type = get_domain_type,
    .get_domain_parent_id = get_domain_parent_id,
    .set_state = set_state,
    .get_state = get_state,
    .reset = reset,
    .system_suspend = system_suspend,
    .system_shutdown = system_shutdown,
};

struct mod_scmi_from_protocol_api from_protocol_api = {
    .get_agent_count = mod_scmi_from_protocol_api_get_agent_count,
    .get_agent_id = mod_scmi_from_protocol_api_get_agent_id,
    .get_agent_type = mod_scmi_from_protocol_api_get_agent_type,
    .get_max_payload_size = mod_scmi_from_protocol_api_get_max_payload_size,
    .write_payload = mod_scmi_from_protocol_api_write_payload,
    .respond = mod_scmi_from_protocol_api_respond,
    .notify = mod_scmi_from_protocol_api_notify,
};

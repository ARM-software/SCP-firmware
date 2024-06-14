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
    .scmi_message_validation = mod_scmi_from_protocol_api_scmi_frame_validation,
};

#ifdef BUILD_HAS_AGENT_LOGICAL_DOMAIN
enum scmi_agent_index {
    SCMI_AGENT_0 = 1,
    SCMI_AGENT_1,
    SCMI_AGENT_COUNT,
};

enum scmi_power_logical_domain {
    SCMI_PD_LOGICAL_DOMAIN_0 = 0,
    SCMI_PD_LOGICAL_DOMAIN_1,
    SCMI_PD_LOGICAL_DOMAIN_2,
    SCMI_PD_LOGICAL_DOMAIN_3,
    SCMI_PD_LOGICAL_DOMAIN_4,
    SCMI_PD_LOGICAL_DOMAIN_5,
    SCMI_PD_LOGICAL_DOMAIN_COUNT,
};

const char *test_domain_names[SCMI_PD_LOGICAL_DOMAIN_COUNT] = {
    [SCMI_PD_LOGICAL_DOMAIN_0] = "PD_0", [SCMI_PD_LOGICAL_DOMAIN_1] = "PD_1",
    [SCMI_PD_LOGICAL_DOMAIN_2] = "PD_2", [SCMI_PD_LOGICAL_DOMAIN_3] = "PD_3",
    [SCMI_PD_LOGICAL_DOMAIN_4] = "PD_4", [SCMI_PD_LOGICAL_DOMAIN_5] = "PD_5",
};

struct mod_scmi_pd_agent_config agent_logical_domain_table[SCMI_AGENT_COUNT] = {
    [SCMI_AGENT_0] = {
        .domain_count = 2,
        .domains = (uint32_t [2]) {
            [0] = SCMI_PD_LOGICAL_DOMAIN_1,
            [1] = SCMI_PD_LOGICAL_DOMAIN_3,
        },
    },

    [SCMI_AGENT_1] = {
        .domain_count = 4,
        .domains = (uint32_t [4]) {
            [0] = SCMI_PD_LOGICAL_DOMAIN_0,
            [1] = SCMI_PD_LOGICAL_DOMAIN_2,
            [2] = SCMI_PD_LOGICAL_DOMAIN_3,
            [3] = SCMI_PD_LOGICAL_DOMAIN_5,
        },
    },
};

const struct mod_scmi_pd_config agent_test_config = {
    .agent_config_table = agent_logical_domain_table,
};
#endif

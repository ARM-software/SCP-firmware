/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      SCMI Power Domain unit test support.
 */

#ifndef MOD_SCMI_POWER_DOMAIN_EXTRA_H
#define MOD_SCMI_POWER_DOMAIN_EXTRA_H

#include "mod_power_domain.h"

#include <mod_scmi.h>

#include <fwk_id.h>

int get_domain_type(fwk_id_t pd_id, enum mod_pd_type *type);

int get_domain_parent_id(fwk_id_t pd_id, fwk_id_t *parent_pd_id);

int set_state(fwk_id_t pd_id, bool resp_requested, uint32_t state);

int get_state(fwk_id_t pd_id, unsigned int *state);

int reset(fwk_id_t pd_id, bool resp_requested);

int system_suspend(unsigned int state);

int system_shutdown(enum mod_pd_system_shutdown system_shsutdown);

int mod_scmi_from_protocol_api_get_agent_count(unsigned int *agent_count);

int mod_scmi_from_protocol_api_get_agent_id(
    fwk_id_t service_id,
    unsigned int *agent_id);

int mod_scmi_from_protocol_api_get_agent_type(
    uint32_t agent_id,
    enum scmi_agent_type *agent_type);

int mod_scmi_from_protocol_api_get_max_payload_size(
    fwk_id_t service_id,
    size_t *size);

int mod_scmi_from_protocol_api_write_payload(
    fwk_id_t service_id,
    size_t offset,
    const void *payload,
    size_t size);

int mod_scmi_from_protocol_api_respond(
    fwk_id_t service_id,
    const void *payload,
    size_t size);

void mod_scmi_from_protocol_api_notify(
    fwk_id_t service_id,
    int protocol_id,
    int message_id,
    const void *payload,
    size_t size);

#endif /* MOD_SCMI_POWER_DOMAIN_EXTRA_H */

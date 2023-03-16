/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Power Requester unit test support.
 */
#include <scmi_system_power.h>

#include <mod_power_domain.h>
#include <mod_scmi.h>
#include <mod_scmi_system_power.h>

int power_domain_get_domain_type(fwk_id_t pd_id, enum mod_pd_type *type);
int power_domain_get_domain_parent_id(fwk_id_t pd_id, fwk_id_t *parent_pd_id);
int power_domain_set_state(fwk_id_t pd_id, bool resp_requested, uint32_t state);
int power_domain_get_state(fwk_id_t pd_id, unsigned int *state);
int power_domain_reset(fwk_id_t pd_id, bool resp_requested);
int power_domain_system_suspend(unsigned int state);
int power_domain_system_shutdown(enum mod_pd_system_shutdown system_shutdown);

int scmi_get_agent_count(unsigned int *agent_count);
int scmi_get_agent_id(fwk_id_t service_id, unsigned int *agent_id);
int scmi_get_agent_type(uint32_t agent_id, enum scmi_agent_type *agent_type);
int scmi_get_max_payload_size(fwk_id_t service_id, size_t *size);
int scmi_write_payload(
    fwk_id_t service_id,
    size_t offset,
    const void *payload,
    size_t size);
int scmi_respond(fwk_id_t service_id, const void *payload, size_t size);
void scmi_notify(
    fwk_id_t service_id,
    int protocol_id,
    int message_id,
    const void *payload,
    size_t size);

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI power domain management protocol support.
 */

#include <string.h>
#include <fwk_assert.h>
#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>
#include <internal/scmi.h>
#include <internal/scmi_power_domain.h>
#include <mod_log.h>
#include <mod_power_domain.h>
#include <mod_scmi.h>

struct scmi_pd_ctx {
    /* Number of power domains */
    unsigned int domain_count;

    /* Log module API */
    const struct mod_log_api *log_api;

    /* SCMI module API */
    const struct mod_scmi_from_protocol_api *scmi_api;

    /* Power domain module API */
    const struct mod_pd_restricted_api *pd_api;
};

static int scmi_pd_protocol_version_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_pd_protocol_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_pd_protocol_message_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_pd_power_domain_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_pd_power_state_set_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_pd_power_state_get_handler(fwk_id_t service_id,
    const uint32_t *payload);

/*
 * Internal variables
 */

static struct scmi_pd_ctx scmi_pd_ctx;

static int (*handler_table[])(fwk_id_t, const uint32_t *) = {
    [SCMI_PROTOCOL_VERSION] =
                       scmi_pd_protocol_version_handler,
    [SCMI_PROTOCOL_ATTRIBUTES] =
                       scmi_pd_protocol_attributes_handler,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
                       scmi_pd_protocol_message_attributes_handler,
    [SCMI_PD_POWER_DOMAIN_ATTRIBUTES] =
                       scmi_pd_power_domain_attributes_handler,
    [SCMI_PD_POWER_STATE_SET] =
                       scmi_pd_power_state_set_handler,
    [SCMI_PD_POWER_STATE_GET] =
                       scmi_pd_power_state_get_handler,
};

static unsigned int payload_size_table[] = {
    [SCMI_PROTOCOL_VERSION] = 0,
    [SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
                       sizeof(struct scmi_protocol_message_attributes_a2p),
    [SCMI_PD_POWER_DOMAIN_ATTRIBUTES] =
                       sizeof(struct scmi_pd_power_domain_attributes_a2p),
    [SCMI_PD_POWER_STATE_SET] =
                       sizeof(struct scmi_pd_power_state_set_a2p),
    [SCMI_PD_POWER_STATE_GET] =
                       sizeof(struct scmi_pd_power_state_get_a2p),
};

static unsigned int scmi_dev_state_id_lost_ctx_to_pd_state[] = {
    [SCMI_PD_DEVICE_STATE_ID_OFF] = MOD_PD_STATE_OFF,
};

static unsigned int scmi_dev_state_id_preserved_ctx_to_pd_state[] = {
    [SCMI_PD_DEVICE_STATE_ID_ON] = MOD_PD_STATE_ON,
};

static uint32_t pd_state_to_scmi_dev_state[] = {
    [MOD_PD_STATE_OFF] = SCMI_PD_DEVICE_STATE_ID_OFF |
                         SCMI_PD_DEVICE_STATE_TYPE,
    [MOD_PD_STATE_ON] = SCMI_PD_DEVICE_STATE_ID_ON,
    /* In case of more supported device states review the map functions */
};

/*
 * Power domain management protocol implementation
 */

static int scmi_device_state_to_pd_state(uint32_t scmi_state,
                                         unsigned int *pd_state)
{
    uint32_t scmi_state_id;
    bool ctx_lost;

    ctx_lost = !!(scmi_state & SCMI_PD_DEVICE_STATE_TYPE);
    scmi_state_id = scmi_state & SCMI_PD_DEVICE_STATE_ID_MASK;

    if (ctx_lost) {
        if (scmi_state_id >=
            FWK_ARRAY_SIZE(scmi_dev_state_id_lost_ctx_to_pd_state))
            return FWK_E_PWRSTATE;

        *pd_state = scmi_dev_state_id_lost_ctx_to_pd_state[scmi_state_id];
    } else {
        if (scmi_state_id >=
            FWK_ARRAY_SIZE(scmi_dev_state_id_preserved_ctx_to_pd_state))
            return FWK_E_PWRSTATE;

        *pd_state = scmi_dev_state_id_preserved_ctx_to_pd_state[scmi_state_id];
    }

    return FWK_SUCCESS;
}

static int pd_state_to_scmi_device_state(unsigned int pd_state,
                                         uint32_t *scmi_state)
{
     if (pd_state >= FWK_ARRAY_SIZE(pd_state_to_scmi_dev_state))
         return FWK_E_PWRSTATE;

     *scmi_state = pd_state_to_scmi_dev_state[pd_state];
     return FWK_SUCCESS;
}

static int scmi_pd_protocol_version_handler(fwk_id_t service_id,
                                            const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_POWER_DOMAIN,
    };

    scmi_pd_ctx.scmi_api->respond(service_id,
                                  &return_values, sizeof(return_values));

    return FWK_SUCCESS;
}

static int scmi_pd_protocol_attributes_handler(fwk_id_t service_id,
                                               const uint32_t *payload)
{
    struct scmi_pd_protocol_attributes_p2a return_values = {
        .status = SCMI_SUCCESS,
    };

    return_values.attributes = scmi_pd_ctx.domain_count;

    scmi_pd_ctx.scmi_api->respond(service_id,
                                  &return_values, sizeof(return_values));

    return FWK_SUCCESS;
}

static int scmi_pd_power_domain_attributes_handler(fwk_id_t service_id,
                                                   const uint32_t *payload)
{
    int status = FWK_SUCCESS;
    const struct scmi_pd_power_domain_attributes_a2p *parameters;
    enum mod_pd_type pd_type;
    unsigned int domain_idx;
    fwk_id_t pd_id;
    unsigned int agent_id;
    enum scmi_agent_type agent_type;
    struct scmi_pd_power_domain_attributes_p2a return_values = {
        .status = SCMI_GENERIC_ERROR,
    };

    parameters = (const struct scmi_pd_power_domain_attributes_a2p *)payload;

    domain_idx = parameters->domain_id;
    if (domain_idx > UINT16_MAX) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, domain_idx);
    if (!fwk_module_is_valid_element_id(pd_id)) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    status = scmi_pd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    status = scmi_pd_ctx.scmi_api->get_agent_type(agent_id, &agent_type);
    if (status != FWK_SUCCESS)
        goto exit;

    status = scmi_pd_ctx.pd_api->get_domain_type(pd_id, &pd_type);
    if (status != FWK_SUCCESS)
        goto exit;

    switch (pd_type) {
    case MOD_PD_TYPE_CORE:
        /*
         * For core power domains, the POWER_STATE_SET command is supported
         * only asynchronously for the PSCI agent. In all other cases, reply
         * that the command is not supported either synchronously nor
         * asynchronously.
         */
        if (agent_type == SCMI_AGENT_TYPE_PSCI)
            return_values.attributes = SCMI_PD_POWER_STATE_SET_ASYNC;
        break;

    case MOD_PD_TYPE_CLUSTER:
        /*
         * For cluster power domains, the POWER_STATE_SET command is supported
         * only synchronously for the PSCI agent. In all other cases, reply
         * that the command is not supported either synchronously nor
         * asynchronously.
         */
        if (agent_type == SCMI_AGENT_TYPE_PSCI)
            return_values.attributes = SCMI_PD_POWER_STATE_SET_SYNC;
        break;

    case MOD_PD_TYPE_DEVICE:
    case MOD_PD_TYPE_DEVICE_DEBUG:
        /*
         * Support only synchronous POWER_STATE_SET for devices for any agent.
         */
        return_values.attributes = SCMI_PD_POWER_STATE_SET_SYNC;
        break;

    case MOD_PD_TYPE_SYSTEM:
        return_values.status = SCMI_NOT_FOUND;
        /* Fallthrough. */

    default:
        goto exit;
    }

    strncpy((char *)return_values.name, fwk_module_get_name(pd_id),
            sizeof(return_values.name) - 1);

    return_values.status = SCMI_SUCCESS;

exit:
    scmi_pd_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return status;
}

static int scmi_pd_protocol_message_attributes_handler(
    fwk_id_t service_id, const uint32_t *payload)
{
    const struct scmi_protocol_message_attributes_a2p *parameters;
    struct scmi_protocol_message_attributes_p2a return_values = {
        .status = SCMI_NOT_FOUND,
    };

    parameters = (const struct scmi_protocol_message_attributes_a2p *)
                  payload;

    if ((parameters->message_id < FWK_ARRAY_SIZE(handler_table)) &&
        (handler_table[parameters->message_id] != NULL))
        return_values.status = SCMI_SUCCESS;

    scmi_pd_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return FWK_SUCCESS;
}

static int scmi_power_scp_set_core_state(fwk_id_t pd_id,
                                         uint32_t composite_state)
{
    int status;

    status = scmi_pd_ctx.pd_api->set_composite_state_async(pd_id, false,
                                                           composite_state);
    if (status != FWK_SUCCESS) {
        scmi_pd_ctx.log_api->log(MOD_LOG_GROUP_ERROR,
            "[SCMI:power] Failed to send core set request (error %s (%d))\n",
            fwk_status_str(status), status);
    }

    return status;
}

static int scmi_pd_power_state_set_handler(fwk_id_t service_id,
                                           const uint32_t *payload)
{
    int status;
    const struct scmi_pd_power_state_set_a2p *parameters;
    bool is_sync;
    unsigned int agent_id;
    enum scmi_agent_type agent_type;
    unsigned int domain_idx;
    fwk_id_t pd_id;
    unsigned int pd_power_state;
    struct scmi_pd_power_state_set_p2a return_values = {
        .status = SCMI_GENERIC_ERROR
    };
    enum mod_pd_type pd_type;

    parameters = (const struct scmi_pd_power_state_set_a2p *)payload;

    is_sync = !(parameters->flags & SCMI_PD_POWER_STATE_SET_ASYNC_FLAG_MASK);

    status = scmi_pd_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    status = scmi_pd_ctx.scmi_api->get_agent_type(agent_id, &agent_type);
    if (status != FWK_SUCCESS)
        goto exit;

    domain_idx = parameters->domain_id;
    if (domain_idx > UINT16_MAX) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, domain_idx);
    if (!fwk_module_is_valid_element_id(pd_id)) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    status = scmi_pd_ctx.pd_api->get_domain_type(pd_id, &pd_type);
    if (status != FWK_SUCCESS)
        goto exit;

    if (((pd_type == MOD_PD_TYPE_CORE) ||
         (pd_type == MOD_PD_TYPE_CLUSTER)) &&
        (agent_type != SCMI_AGENT_TYPE_PSCI)) {

         return_values.status = SCMI_NOT_SUPPORTED;
         goto exit;
    }

    switch (pd_type) {
    case MOD_PD_TYPE_CORE:
        /*
         * Async/sync flag is ignored for core power domains as stated
         * by the specification.
         */
        status = scmi_power_scp_set_core_state(pd_id, parameters->power_state);
        if (status == FWK_E_PARAM)
            return_values.status = SCMI_INVALID_PARAMETERS;
        break;

    case MOD_PD_TYPE_CLUSTER:
        if (!is_sync) {
            return_values.status = SCMI_NOT_SUPPORTED;
            goto exit;
        }

        status = scmi_pd_ctx.pd_api->set_state(pd_id, parameters->power_state);
        break;

    case MOD_PD_TYPE_DEVICE:
    case MOD_PD_TYPE_DEVICE_DEBUG:
        if (!is_sync) {
            return_values.status = SCMI_NOT_SUPPORTED;
            goto exit;
        }

        status = scmi_device_state_to_pd_state(parameters->power_state,
                                               &pd_power_state);
        if (status != FWK_SUCCESS) {
            status = FWK_SUCCESS;
            return_values.status = SCMI_INVALID_PARAMETERS;
            goto exit;
        }

        status = scmi_pd_ctx.pd_api->set_state(pd_id, pd_power_state);
        break;

    case MOD_PD_TYPE_SYSTEM:
        return_values.status = SCMI_NOT_FOUND;
        /* Fallthrough. */

    default:
        goto exit;
    }

    if (status == FWK_SUCCESS)
        return_values.status = SCMI_SUCCESS;

exit:
    scmi_pd_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return status;
}

static int scmi_pd_power_state_get_handler(fwk_id_t service_id,
                                           const uint32_t *payload)
{
    int status = FWK_SUCCESS;
    const struct scmi_pd_power_state_get_a2p *parameters;
    unsigned int domain_idx;
    fwk_id_t pd_id;
    struct scmi_pd_power_state_get_p2a return_values = {
        .status = SCMI_GENERIC_ERROR
    };
    enum mod_pd_type pd_type;
    unsigned int pd_power_state;
    unsigned int power_state;

    parameters = (const struct scmi_pd_power_state_get_a2p *)payload;

    domain_idx = parameters->domain_id;
    if (domain_idx > UINT16_MAX) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    pd_id = FWK_ID_ELEMENT(FWK_MODULE_IDX_POWER_DOMAIN, domain_idx);
    if (!fwk_module_is_valid_element_id(pd_id)) {
        return_values.status = SCMI_NOT_FOUND;
        goto exit;
    }

    status = scmi_pd_ctx.pd_api->get_domain_type(pd_id, &pd_type);
    if (status != FWK_SUCCESS)
        goto exit;

    switch (pd_type) {
    case MOD_PD_TYPE_CORE:
        status = scmi_pd_ctx.pd_api->get_composite_state(pd_id, &power_state);
        break;

    case MOD_PD_TYPE_CLUSTER:
        status = scmi_pd_ctx.pd_api->get_state(pd_id, &power_state);
        break;

    case MOD_PD_TYPE_DEVICE:
    case MOD_PD_TYPE_DEVICE_DEBUG:

        status = scmi_pd_ctx.pd_api->get_state(pd_id, &pd_power_state);
        if (status != FWK_SUCCESS)
            goto exit;

        status = pd_state_to_scmi_device_state(pd_power_state,
                                               (uint32_t *) &power_state);
        break;

    case MOD_PD_TYPE_SYSTEM:
        return_values.status = SCMI_NOT_FOUND;
        /* Fallthrough. */

    default:
        goto exit;
    }

    if (status == FWK_SUCCESS) {
        return_values.status = SCMI_SUCCESS;
        return_values.power_state = power_state;
    }

exit:
    scmi_pd_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return status;
}

/*
 * SCMI module -> SCMI power module interface
 */
static int scmi_pd_get_scmi_protocol_id(fwk_id_t protocol_id,
                                        uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = SCMI_PROTOCOL_ID_POWER_DOMAIN;

    return FWK_SUCCESS;
}

static int scmi_pd_message_handler(fwk_id_t protocol_id, fwk_id_t service_id,
    const uint32_t *payload, size_t payload_size, unsigned int message_id)
{
    int32_t return_value;

    static_assert(FWK_ARRAY_SIZE(handler_table) ==
        FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] Power domain management protocol table sizes not consistent");
    assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(handler_table)) {
        return_value = SCMI_NOT_SUPPORTED;
        goto error;
    }

    if (payload_size != payload_size_table[message_id]) {
        return_value = SCMI_PROTOCOL_ERROR;
        goto error;
    }

    return handler_table[message_id](service_id, payload);

error:
    scmi_pd_ctx.scmi_api->respond(service_id,
                                  &return_value, sizeof(return_value));
    return FWK_SUCCESS;
}

static struct mod_scmi_to_protocol_api scmi_pd_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = scmi_pd_get_scmi_protocol_id,
    .message_handler = scmi_pd_message_handler
};

/*
 * Framework handlers
 */

static int scmi_pd_init(fwk_id_t module_id, unsigned int element_count,
                        const void *unused)
{
    if (element_count != 0)
        return FWK_E_SUPPORT;

    scmi_pd_ctx.domain_count = fwk_module_get_element_count(
        fwk_module_id_power_domain);
    if (scmi_pd_ctx.domain_count <= 1)
        return FWK_E_SUPPORT;

    /* Do not expose SYSTEM domain (always the last one) to agents and ... */
    scmi_pd_ctx.domain_count--;
    /* ... and expose no more than 0xFFFF number of domains. */
    if (scmi_pd_ctx.domain_count > UINT16_MAX)
        scmi_pd_ctx.domain_count = UINT16_MAX;

    return FWK_SUCCESS;
}

static int scmi_pd_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 1)
        return FWK_SUCCESS;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_LOG),
        FWK_ID_API(FWK_MODULE_IDX_LOG, 0), &scmi_pd_ctx.log_api);
    if (status != FWK_SUCCESS)
        return status;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_pd_ctx.scmi_api);
    if (status != FWK_SUCCESS)
        return status;

    return fwk_module_bind(fwk_module_id_power_domain, mod_pd_api_id_restricted,
        &scmi_pd_ctx.pd_api);
}

static int scmi_pd_process_bind_request(fwk_id_t source_id, fwk_id_t target_id,
                                        fwk_id_t api_id, const void **api)
{
    if (!fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI)))
        return FWK_E_ACCESS;

    *api = &scmi_pd_mod_scmi_to_protocol_api;

    return FWK_SUCCESS;
}

/* SCMI Power Domain Management Protocol Definition */
const struct fwk_module module_scmi_power_domain = {
    .name = "SCMI Power Domain Management Protocol",
    .api_count = 1,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_pd_init,
    .bind = scmi_pd_bind,
    .process_bind_request = scmi_pd_process_bind_request,
};

/* No elements, no module configuration data */
struct fwk_module_config config_scmi_power_domain = { 0 };

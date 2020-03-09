/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI Core Configuration Protocol Support.
 */

#include <internal/scmi.h>
#include <internal/scmi_apcore.h>

#include <mod_scmi.h>
#include <mod_scmi_apcore.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <stdint.h>

struct scmi_apcore_ctx {
    /* Module Configuration */
    const struct mod_scmi_apcore_config *config;

    /* SCMI module API */
    const struct mod_scmi_from_protocol_api *scmi_api;

    /*
     * Tracks whether an agent has requested that the configuration be locked.
     * \c true if the configuration is locked and the reset address of the CPUs
     * can no longer be altered, \c false otherwise.
     */
    bool locked;
};

static int scmi_apcore_protocol_version_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_apcore_protocol_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_apcore_protocol_message_attributes_handler(
    fwk_id_t service_id, const uint32_t *payload);
static int scmi_apcore_reset_address_set_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_apcore_reset_address_get_handler(fwk_id_t service_id,
    const uint32_t *payload);

/*
 * Internal variables.
 */
static struct scmi_apcore_ctx scmi_apcore_ctx;

static int (* const handler_table[])(fwk_id_t, const uint32_t *) = {
    [SCMI_PROTOCOL_VERSION] = scmi_apcore_protocol_version_handler,
    [SCMI_PROTOCOL_ATTRIBUTES] = scmi_apcore_protocol_attributes_handler,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        scmi_apcore_protocol_message_attributes_handler,
    [SCMI_APCORE_RESET_ADDRESS_SET] = scmi_apcore_reset_address_set_handler,
    [SCMI_APCORE_RESET_ADDRESS_GET] = scmi_apcore_reset_address_get_handler,
};

static const unsigned int payload_size_table[] = {
    [SCMI_PROTOCOL_VERSION] = 0,
    [SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        sizeof(struct scmi_protocol_message_attributes_a2p),
    [SCMI_APCORE_RESET_ADDRESS_SET] =
        sizeof(struct scmi_apcore_reset_address_set_a2p),
    [SCMI_APCORE_RESET_ADDRESS_GET] = 0,
};

/*
 * Static, Helper Functions
 */
static int set_reset_address(uint32_t address_low, uint32_t address_high)
{
    uint64_t address_composite;
    unsigned int grp_idx;
    unsigned int reg_idx;
    const struct mod_scmi_apcore_reset_register_group *reg_group;
    uintptr_t reset_reg;

    address_composite = ((uint64_t)address_high << 32) | address_low;

    /* Iterate over the reset register group structures */
    for (grp_idx = 0;
         grp_idx < scmi_apcore_ctx.config->reset_register_group_count;
         grp_idx++) {

        reg_group =
            &scmi_apcore_ctx.config->reset_register_group_table[grp_idx];
        assert(reg_group->base_register != 0);

        /* Begin with the first register in the group */
        reset_reg = reg_group->base_register;

        /* Program each reset vector register within the group */
        for (reg_idx = 0; reg_idx < reg_group->register_count; reg_idx++) {
            if (scmi_apcore_ctx.config->reset_register_width ==
                MOD_SCMI_APCORE_REG_WIDTH_32) {
                /* Treat the register as 32-bit */
                *(uint32_t *)reset_reg = address_low;
                reset_reg += sizeof(uint32_t);
            } else {
                /* Treat the register as 64-bit */
                *(uint64_t *)reset_reg = address_composite;
                reset_reg += sizeof(uint64_t);
            }
        }
    }

    return FWK_SUCCESS;
}

/*
 * Protocol Version
 */
static int scmi_apcore_protocol_version_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_APCORE,
    };

    scmi_apcore_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
    return FWK_SUCCESS;
}

/*
 * Protocol Attributes
 */
static int scmi_apcore_protocol_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_attributes_p2a return_values = {
        .status = SCMI_SUCCESS,
        .attributes = 0,
    };

    if (scmi_apcore_ctx.config->reset_register_width ==
        MOD_SCMI_APCORE_REG_WIDTH_64)
        return_values.attributes |= SCMI_APCORE_PROTOCOL_ATTRIBUTES_64BIT_MASK;

    scmi_apcore_ctx.scmi_api->respond(
        service_id,
        &return_values,
        sizeof(return_values));

    return FWK_SUCCESS;
}

/*
 * Protocol Message Attributes
 */
static int scmi_apcore_protocol_message_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    size_t response_size;
    const struct scmi_protocol_message_attributes_a2p *parameters;
    unsigned int message_id;
    struct scmi_protocol_message_attributes_p2a return_values = {
        .status = SCMI_SUCCESS,
        .attributes = 0,
    };

    parameters = (const struct scmi_protocol_message_attributes_a2p *)
        payload;
    message_id = parameters->message_id;

    if ((message_id >= FWK_ARRAY_SIZE(handler_table)) ||
        (handler_table[message_id] == NULL))
        return_values.status = SCMI_NOT_FOUND;

    response_size = (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status);

    scmi_apcore_ctx.scmi_api->respond(
        service_id, &return_values, response_size);

    return FWK_SUCCESS;
}

/*
 * Reset Address Set
 */
static int scmi_apcore_reset_address_set_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status;
    unsigned int agent_id;
    enum scmi_agent_type agent_type;
    const struct scmi_apcore_reset_address_set_a2p *parameters;
    struct scmi_apcore_reset_address_set_p2a return_values = {
        .status = SCMI_GENERIC_ERROR
    };

    parameters = (const struct scmi_apcore_reset_address_set_a2p *)payload;

    status = scmi_apcore_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    status = scmi_apcore_ctx.scmi_api->get_agent_type(agent_id, &agent_type);
    if (status != FWK_SUCCESS)
        goto exit;

    /* Only the PSCI agent may set the reset address */
    if (agent_type != SCMI_AGENT_TYPE_PSCI) {
        return_values.status = SCMI_DENIED;
        goto exit;
    }

    /* An agent previously requested that the configuration be locked */
    if (scmi_apcore_ctx.locked) {
        return_values.status = SCMI_DENIED;
        goto exit;
    }

    /*
     * Ensure that the platform has 64-bit reset vector registers if a reset
     * address utilizing more that 32 bits has been provided.
     */
    if ((parameters->reset_address_high != 0) &&
        (scmi_apcore_ctx.config->reset_register_width ==
         MOD_SCMI_APCORE_REG_WIDTH_32)) {
        return_values.status = SCMI_INVALID_PARAMETERS;
        goto exit;
    }

    /* Check for alignment */
    if (scmi_apcore_ctx.config->reset_register_width ==
        MOD_SCMI_APCORE_REG_WIDTH_32) {
        if ((parameters->reset_address_low % 4) != 0) {
            return_values.status = SCMI_INVALID_PARAMETERS;
            goto exit;
        }
    } else if ((parameters->reset_address_low % 8) != 0) {
        return_values.status = SCMI_INVALID_PARAMETERS;
        goto exit;
    }

    status = set_reset_address(
        parameters->reset_address_low, parameters->reset_address_high);
    if (status != FWK_SUCCESS)
        goto exit;

    return_values.status = SCMI_SUCCESS;

    /* Lock the configuration if requested */
    if (parameters->attributes & SCMI_APCORE_RESET_ADDRESS_SET_LOCK_MASK)
        scmi_apcore_ctx.locked = true;

exit:
    scmi_apcore_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
    return status;
}

/*
 * Reset Address Get
 */
static int scmi_apcore_reset_address_get_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status;
    unsigned int agent_id;
    const struct mod_scmi_apcore_reset_register_group *reg_group;
    uint64_t reset_address;
    enum scmi_agent_type agent_type;
    struct scmi_apcore_reset_address_get_p2a return_values = {
        .status = SCMI_GENERIC_ERROR
    };

    status = scmi_apcore_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    status = scmi_apcore_ctx.scmi_api->get_agent_type(agent_id, &agent_type);
    if (status != FWK_SUCCESS)
        goto exit;

    /* Only the PSCI agent may get the current reset address */
    if (agent_type != SCMI_AGENT_TYPE_PSCI) {
        return_values.status = SCMI_DENIED;
        goto exit;
    }

    /* The reset address is common across all reset address registers */
    reg_group = &scmi_apcore_ctx.config->reset_register_group_table[0];

    if (scmi_apcore_ctx.config->reset_register_width ==
        MOD_SCMI_APCORE_REG_WIDTH_32) {
        reset_address = *(uint32_t *)reg_group->base_register;
        return_values.reset_address_high = 0;
    } else {
        reset_address = *(uint64_t *)reg_group->base_register;
        return_values.reset_address_high = (reset_address >> 32) & UINT32_MAX;
    }

    return_values.reset_address_low = (uint32_t)reset_address;

    return_values.attributes |=
        (scmi_apcore_ctx.locked << SCMI_APCORE_RESET_ADDRESS_GET_LOCK_POS);
    return_values.status = SCMI_SUCCESS;

exit:
    scmi_apcore_ctx.scmi_api->respond(
        service_id, &return_values, sizeof(return_values));
    return status;
}

/*
 * SCMI module -> SCMI AP Core Configuration module interface
 */
static int scmi_apcore_get_scmi_protocol_id(fwk_id_t protocol_id,
    uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = SCMI_PROTOCOL_ID_APCORE;

    return FWK_SUCCESS;
}

static int scmi_apcore_message_handler(
    fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    int32_t return_value;

    static_assert(FWK_ARRAY_SIZE(handler_table) ==
        FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] Core configuration protocol table sizes not consistent");
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
    scmi_apcore_ctx.scmi_api->respond(
        service_id,
        &return_value,
        sizeof(return_value));

    return FWK_SUCCESS;
}

static struct mod_scmi_to_protocol_api scmi_apcore_mod_scmi_to_protocol_api = {
    .get_scmi_protocol_id = scmi_apcore_get_scmi_protocol_id,
    .message_handler = scmi_apcore_message_handler
};

/*
 * Framework handlers
 */

static int scmi_apcore_init(fwk_id_t module_id, unsigned int element_count,
                           const void *data)
{
    const struct mod_scmi_apcore_config *config =
        (const struct mod_scmi_apcore_config *)data;

    if (config == NULL)
        return FWK_E_PARAM;
    if (config->reset_register_group_table == NULL)
        return FWK_E_PARAM;
    if (config->reset_register_group_count == 0)
        return FWK_E_PARAM;
    if (config->reset_register_width >= MOD_SCMI_APCORE_REG_WIDTH_COUNT)
        return FWK_E_PARAM;

    scmi_apcore_ctx.config = config;

    return FWK_SUCCESS;
}

static int scmi_apcore_bind(fwk_id_t id, unsigned int round)
{
    if (round == 1)
        return FWK_SUCCESS;

    /* Bind to the SCMI module, storing an API pointer for later use. */
    return fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_apcore_ctx.scmi_api);
}

static int scmi_apcore_process_bind_request(fwk_id_t source_id,
    fwk_id_t target_id, fwk_id_t api_id, const void **api)
{
    /* Only accept binding requests from the SCMI module. */
    if (!fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI)))
        return FWK_E_ACCESS;

    *api = &scmi_apcore_mod_scmi_to_protocol_api;

    return FWK_SUCCESS;
}

/* SCMI Clock Management Protocol Definition */
const struct fwk_module module_scmi_apcore = {
    .name = "SCMI Core Configuration Protocol",
    .api_count = 1,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_apcore_init,
    .bind = scmi_apcore_bind,
    .process_bind_request = scmi_apcore_process_bind_request,
};

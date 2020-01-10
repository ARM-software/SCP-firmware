/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2020, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     SCMI CCIX_CONFIG management protocol support.
 */

#include <internal/mod_scmi_ccix_config.h>
#include <internal/scmi.h>

#include <mod_cmn600.h>
#include <mod_n1sdp_pcie.h>
#include <mod_scmi.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stdbool.h>
#include <string.h>

struct scmi_ccix_config_ctx {
    /* scmi module api */
    const struct mod_scmi_from_protocol_api *scmi_api;

    /* CMN600 CCIX config api */
    const struct mod_cmn600_ccix_config_api *cmn600_ccix_config_api;

    /* PCIe CCIX config api */
    const struct n1sdp_pcie_ccix_config_api *pcie_ccix_config_api;
};

static struct scmi_ccix_config_ctx scmi_ccix_config_ctx;

static int scmi_ccix_config_protocol_version_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_ccix_config_protocol_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_ccix_config_protocol_msg_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_ccix_config_protocol_get_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_ccix_config_protocol_set_handler(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_ccix_config_protocol_exchange_credit(fwk_id_t service_id,
    const uint32_t *payload);
static int scmi_ccix_config_protocol_enter_system_coherency(fwk_id_t service_id,
    const uint32_t *payload);

static int (*handler_table[])(fwk_id_t, const uint32_t *) = {
    [SCMI_PROTOCOL_VERSION] =
        scmi_ccix_config_protocol_version_handler,
    [SCMI_PROTOCOL_ATTRIBUTES] =
        scmi_ccix_config_protocol_attributes_handler,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        scmi_ccix_config_protocol_msg_attributes_handler,
    [SCMI_CCIX_CONFIG_SET] =
        scmi_ccix_config_protocol_set_handler,
    [SCMI_CCIX_CONFIG_GET] =
        scmi_ccix_config_protocol_get_handler,
    [SCMI_CCIX_CONFIG_EXCHANGE_PROTOCOL_CREDIT] =
        scmi_ccix_config_protocol_exchange_credit,
    [SCMI_CCIX_CONFIG_ENTER_SYSTEM_COHERENCY] =
        scmi_ccix_config_protocol_enter_system_coherency,
};

static unsigned int payload_size_table[] = {
    [SCMI_PROTOCOL_VERSION] = 0,
    [SCMI_PROTOCOL_ATTRIBUTES] = 0,
    [SCMI_PROTOCOL_MESSAGE_ATTRIBUTES] =
        sizeof(struct scmi_protocol_message_attributes_a2p),
    [SCMI_CCIX_CONFIG_SET] =
        sizeof(struct scmi_ccix_config_protocol_set_a2p),
    [SCMI_CCIX_CONFIG_GET] = 0,
    [SCMI_CCIX_CONFIG_EXCHANGE_PROTOCOL_CREDIT] =
        sizeof(struct scmi_ccix_config_protocol_credit_a2p),
    [SCMI_CCIX_CONFIG_ENTER_SYSTEM_COHERENCY] =
        sizeof(struct scmi_ccix_config_protocol_sys_coherency_a2p),
};

static int scmi_ccix_config_protocol_version_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_version_p2a return_values = {
        .status = SCMI_SUCCESS,
        .version = SCMI_PROTOCOL_VERSION_CCIX_CONFIG,
    };

    scmi_ccix_config_ctx.scmi_api->respond(service_id, &return_values,
                                      sizeof(return_values));

    return FWK_SUCCESS;
}

static int scmi_ccix_config_protocol_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    struct scmi_protocol_attributes_p2a return_values = {
        .status = SCMI_SUCCESS,
        .attributes = 0,
    };

    scmi_ccix_config_ctx.scmi_api->respond(service_id, &return_values,
                                      sizeof(return_values));

    return FWK_SUCCESS;
}

static int scmi_ccix_config_protocol_msg_attributes_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    const struct scmi_protocol_message_attributes_a2p *parameters;
    struct scmi_protocol_message_attributes_p2a return_values;

    parameters = (const struct scmi_protocol_message_attributes_a2p *)
                 payload;

    if ((parameters->message_id < FWK_ARRAY_SIZE(handler_table)) &&
        (handler_table[parameters->message_id] != NULL)) {
        return_values = (struct scmi_protocol_message_attributes_p2a) {
            .status = SCMI_SUCCESS,
            /* All commands have an attributes value of 0 */
            .attributes = 0,
        };
    } else
        return_values.status = SCMI_NOT_FOUND;

    scmi_ccix_config_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return FWK_SUCCESS;
}

static int scmi_ccix_config_protocol_get_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status = FWK_SUCCESS;
    unsigned int i;
    struct scmi_ccix_config_protocol_get_p2a return_values;
    struct mod_cmn600_ccix_host_node_config ccix_host_config;
    size_t max_payload_size;
    unsigned int agent_id;
    enum scmi_agent_type agent_type;

    return_values.status = SCMI_SUCCESS;

    status = scmi_ccix_config_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    status = scmi_ccix_config_ctx.scmi_api->get_agent_type(agent_id,
        &agent_type);
    if (status != FWK_SUCCESS)
        goto exit;

    /* Only the OSPM agent allowed */
    if (agent_type != SCMI_AGENT_TYPE_OSPM) {
        return_values.status = SCMI_DENIED;
        status = FWK_E_ACCESS;
        goto exit;
    }

    status = scmi_ccix_config_ctx.scmi_api->get_max_payload_size(service_id,
        &max_payload_size);
    if (status != FWK_SUCCESS)
        goto exit;

    if (sizeof(return_values) > max_payload_size) {
        return_values.status = SCMI_OUT_OF_RANGE;
        status = FWK_E_RANGE;
        FWK_LOG_INFO(
            "[SCMI CCIX CONFIG] max payload size is  %d", max_payload_size);
        goto exit;
    }

    scmi_ccix_config_ctx.cmn600_ccix_config_api->get_config(&ccix_host_config);

    return_values.agent_count =
        ((ccix_host_config.host_ha_count << HA_COUNT_BIT_POS) |
         (ccix_host_config.host_sa_count << SA_COUNT_BIT_POS) |
         (ccix_host_config.host_ra_count << RA_COUNT_BIT_POS));

    return_values.host_mmap_count = ccix_host_config.ccix_host_mmap_count;

    for (i = 0; i <  ccix_host_config.ccix_host_mmap_count; i++) {
        return_values.mem_pools[i].ha_id =
            ccix_host_config.ccix_host_mmap[i].ha_id;
        return_values.mem_pools[i].base_msb  =
            (uint32_t)((ccix_host_config.ccix_host_mmap[i].base &
                        ADDRESS_MSB_MASK) >> 32);
        return_values.mem_pools[i].base_lsb  =
            (uint32_t)(ccix_host_config.ccix_host_mmap[i].base &
                       ADDRESS_LSB_MASK);

        return_values.mem_pools[i].size_msb  =
            (uint32_t)((ccix_host_config.ccix_host_mmap[i].size &
                        ADDRESS_MSB_MASK) >> 32);
        return_values.mem_pools[i].size_lsb  =
            (uint32_t)(ccix_host_config.ccix_host_mmap[i].size &
                       ADDRESS_LSB_MASK);
    }

    return_values.link_properties =
        ((ccix_host_config.ccix_data_credits << DATA_CREDITS_BIT_POS) |
         (ccix_host_config.ccix_snoop_credits << SNOOP_CREDITS_BIT_POS) |
         (ccix_host_config.ccix_request_credits << REQUEST_CREDITS_BIT_POS) |
         (ccix_host_config.ccix_max_packet_size <<
          HOST_MAX_PACKET_SIZE_BIT_POS) |
         (ccix_host_config.ccix_opt_tlp << HOST_OPT_TLP_BIT_POS) |
         (ccix_host_config.ccix_msg_pack_enable << HOST_MSG_PACK_BIT_POS));

exit:
    scmi_ccix_config_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
        sizeof(return_values) : sizeof(return_values.status));

    return status;
}

static int scmi_ccix_config_protocol_set_handler(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status = FWK_SUCCESS;
    unsigned int i;
    struct scmi_ccix_config_protocol_set_p2a return_values;
    struct mod_cmn600_ccix_remote_node_config ccix_ep_config;
    size_t max_payload_size;
    unsigned int agent_id;
    enum scmi_agent_type agent_type;
    return_values.status = SCMI_SUCCESS;

    const struct scmi_ccix_config_protocol_set_a2p  * params;
    params = (const struct scmi_ccix_config_protocol_set_a2p *)payload;

    status = scmi_ccix_config_ctx.scmi_api->get_agent_id(service_id, &agent_id);
    if (status != FWK_SUCCESS)
        goto exit;

    status = scmi_ccix_config_ctx.scmi_api->get_agent_type(agent_id,
        &agent_type);
    if (status != FWK_SUCCESS)
        goto exit;

    /* Only the OSPM agent allowed */
    if (agent_type != SCMI_AGENT_TYPE_OSPM) {
        return_values.status = SCMI_DENIED;
        status = FWK_E_ACCESS;
        goto exit;
    }

    status = scmi_ccix_config_ctx.scmi_api->get_max_payload_size(service_id,
        &max_payload_size);
    if (status != FWK_SUCCESS)
        goto exit;

    if (sizeof(*params) > max_payload_size) {
        return_values.status = SCMI_OUT_OF_RANGE;
        status = FWK_E_RANGE;
        FWK_LOG_INFO(
            "[SCMI CCIX CONFIG] max payload size is  %d", max_payload_size);
        goto exit;
    }

    ccix_ep_config.remote_rnf_count =
        (uint8_t)(params->agent_count & RA_COUNT_MASK);
    ccix_ep_config.remote_ha_count =
        (uint8_t)((params->agent_count & HA_COUNT_MASK) >> HA_COUNT_BIT_POS);
    ccix_ep_config.remote_sa_count =
        (uint8_t)((params->agent_count & SA_COUNT_MASK) >> SA_COUNT_BIT_POS);
    ccix_ep_config.pcie_bus_num =
        (uint8_t)((params->config_property & EP_START_BUS_NUM_MASK) >>
                   EP_START_BUS_NUM_BIT_POS);
    ccix_ep_config.ccix_link_id =
        (uint8_t)((params->config_property & LINK_ID_MASK) >> LINK_ID_BIT_POS);
    ccix_ep_config.ccix_tc =
        (uint8_t)((params->config_property & TRAFFIC_CLASS_MASK) >>
                   TRAFFIC_CLASS_BIT_POS);
    ccix_ep_config.ccix_msg_pack_enable =
        (bool)((params->config_property & MSG_PACK_MASK) >> MSG_PACK_BIT_POS);
    ccix_ep_config.remote_ha_mmap_count = (uint8_t)(params->remote_mmap_count);
    ccix_ep_config.ccix_opt_tlp = (bool)((params->config_property &
                                          OPT_TLP_MASK) >> OPT_TLP_BIT_POS);
    ccix_ep_config.ccix_max_packet_size = (uint8_t)((params->config_property &
                                                     MAX_PACKET_SIZE_MASK) >>
                                                     MAX_PACKET_SIZE_BIT_POS);

    for (i = 0; i < ccix_ep_config.remote_ha_mmap_count; i++) {
        ccix_ep_config.remote_ha_mmap[i].ha_id =
            (uint8_t)params->mem_pools[i].ha_id;
        ccix_ep_config.remote_ha_mmap[i].base  =
            (((uint64_t)params->mem_pools[i].base_msb << 32) |
                        params->mem_pools[i].base_lsb);
        ccix_ep_config.remote_ha_mmap[i].size  =
            (((uint64_t)params->mem_pools[i].size_msb << 32) |
                        params->mem_pools[i].size_lsb);
    }
    status = scmi_ccix_config_ctx.pcie_ccix_config_api->enable_opt_tlp(
                 ccix_ep_config.ccix_opt_tlp);

    status = scmi_ccix_config_ctx.cmn600_ccix_config_api->set_config(
                 &ccix_ep_config);

exit:
    scmi_ccix_config_ctx.scmi_api->respond(service_id, &return_values,
        (return_values.status == SCMI_SUCCESS) ?
         sizeof(return_values) : sizeof(return_values.status));

    return status;
}

static int scmi_ccix_config_protocol_exchange_credit(fwk_id_t service_id,
    const uint32_t *payload)
{
    int status = FWK_SUCCESS;
    const struct scmi_ccix_config_protocol_credit_a2p  *params;
    params =  (const struct scmi_ccix_config_protocol_credit_a2p *)payload;

    status =
        scmi_ccix_config_ctx.cmn600_ccix_config_api->exchange_protocol_credit(
            (uint8_t)params->link_id);
    scmi_ccix_config_ctx.scmi_api->respond(service_id, &status,
        sizeof(status));

    return status;
}

static int scmi_ccix_config_protocol_enter_system_coherency(
    fwk_id_t service_id, const uint32_t *payload)
{
    int status = FWK_SUCCESS;
    const struct scmi_ccix_config_protocol_sys_coherency_a2p  *params;
    params =
        (const struct scmi_ccix_config_protocol_sys_coherency_a2p *)payload;

    status =
        scmi_ccix_config_ctx.cmn600_ccix_config_api->enter_system_coherency(
        (uint8_t)params->link_id);

    scmi_ccix_config_ctx.scmi_api->respond(service_id, &status,
        sizeof(status));

    return status;
}

static int scmi_ccix_config_get_scmi_protocol_id(fwk_id_t protocol_id,
    uint8_t *scmi_protocol_id)
{
    *scmi_protocol_id = SCMI_PROTOCOL_ID_CCIX_CONFIG;

    return FWK_SUCCESS;
}

static int scmi_ccix_config_message_handler(fwk_id_t protocol_id,
    fwk_id_t service_id,
    const uint32_t *payload,
    size_t payload_size,
    unsigned int message_id)
{
    int32_t return_value;

    static_assert(FWK_ARRAY_SIZE(handler_table) ==
        FWK_ARRAY_SIZE(payload_size_table),
        "[SCMI] CCIX_CONFIG management protocol table sizes not consistent");
    fwk_assert(payload != NULL);

    if (message_id >= FWK_ARRAY_SIZE(handler_table)) {
        return_value = SCMI_NOT_SUPPORTED;
        goto error;
    }

    if (payload_size != payload_size_table[message_id]) {
        /* Incorrect payload size or message is not supported */
        return_value = SCMI_PROTOCOL_ERROR;
        goto error;
    }

    return handler_table[message_id](service_id, payload);

error:
    scmi_ccix_config_ctx.scmi_api->respond(service_id, &return_value,
                                      sizeof(return_value));
    return FWK_SUCCESS;
}

static struct mod_scmi_to_protocol_api scmi_ccix_config_protocol_api = {
    .get_scmi_protocol_id = scmi_ccix_config_get_scmi_protocol_id,
    .message_handler = scmi_ccix_config_message_handler
};

/*
 * Framework interface
 */
static int scmi_ccix_config_init(fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{
    if (element_count != 0) {
        /* This module should not have any elements */
        fwk_assert(false);
        return FWK_E_SUPPORT;
    }

    return FWK_SUCCESS;
}

static int scmi_ccix_config_bind(fwk_id_t id, unsigned int round)
{
    int status;

    if (round == 1)
        return FWK_SUCCESS;

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_SCMI),
        FWK_ID_API(FWK_MODULE_IDX_SCMI, MOD_SCMI_API_IDX_PROTOCOL),
        &scmi_ccix_config_ctx.scmi_api);
    if (status != FWK_SUCCESS) {
        /* Failed to bind to SCMI module */
        fwk_assert(false);
        return status;
    }

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_N1SDP_PCIE),
        FWK_ID_API(FWK_MODULE_IDX_N1SDP_PCIE, N1SDP_PCIE_API_IDX_CCIX_CONFIG),
        &scmi_ccix_config_ctx.pcie_ccix_config_api);
    if (status != FWK_SUCCESS) {
        /* Failed to bind to N1SDP PCIE module */
        fwk_assert(false);
        return status;
    }

    status = fwk_module_bind(FWK_ID_MODULE(FWK_MODULE_IDX_CMN600),
        FWK_ID_API(FWK_MODULE_IDX_CMN600, MOD_CMN600_API_IDX_CCIX_CONFIG),
        &scmi_ccix_config_ctx.cmn600_ccix_config_api);
    if (status != FWK_SUCCESS) {
        /* Failed to bind to CMN600 module */
        fwk_assert(false);
        return status;
    }

    return FWK_SUCCESS;
}

static int scmi_ccix_config_process_bind_request(fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    if (!fwk_id_is_equal(source_id, FWK_ID_MODULE(FWK_MODULE_IDX_SCMI)))
        return FWK_E_ACCESS;

    *api = &scmi_ccix_config_protocol_api;

    return FWK_SUCCESS;
}

const struct fwk_module module_scmi_ccix_config = {
    .name = "SCMI CCIX Config Management Protocol",
    .api_count = 1,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = scmi_ccix_config_init,
    .bind = scmi_ccix_config_bind,
    .process_bind_request = scmi_ccix_config_process_bind_request,
};

/* No elements, no module configuration data */
struct fwk_module_config config_scmi_ccix_config = {0};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      System Control and Management Interface (SCMI) unit test support.
 */
#include <mod_dvfs.h>
#include <mod_scmi.h>
#include <mod_scmi_perf.h>

/*!
 * \brief Get the number of active agents.
 *
 * \param[out] agent_count Number of active agents.
 *
 * \retval ::FWK_SUCCESS The agent count was returned.
 * \retval ::FWK_E_PARAM The parameter `agent_count` is equal to `NULL`.
 */
int mod_scmi_from_protocol_api_get_agent_count(unsigned int *agent_count);

/*!
 * \brief Get the identifier of the agent associated with a service
 *
 * \param service_id Identifier of the service.
 * \param[out] agent_id Agent identifier.
 *
 * \retval ::FWK_SUCCESS The agent identifier was returned.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `service_id` parameter was not a valid system entity
 *        identifier.
 *      - The `agent_id` parameter was a null pointer value.
 * \retval ::FWK_E_INIT The service is not initialized.
 * \retval ::FWK_E_STATE The service is in an invalid state.
 */
int mod_scmi_from_protocol_api_get_agent_id(
    fwk_id_t service_id,
    unsigned int *agent_id);

/*!
 * \brief Get the type of the agent given its identifier.
 *
 * \details This API can be used by SCMI protocols to check the validity
 *          of an agent identifier.
 *
 * \param agent_id Identifier of the agent.
 * \param[out] agent_type Agent type.
 *
 * \retval ::FWK_SUCCESS The agent identifier was returned.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `agent_id` parameter was not a valid system entity
 *        identifier.
 *      - The `agent_type` parameter was a null pointer value.
 */
int mod_scmi_from_protocol_api_get_agent_type(
    uint32_t agent_id,
    enum scmi_agent_type *agent_type);

/*!
 * \brief Get the maximum permitted payload size of a channel associated
 *        with a service.
 *
 * \param service_id Service identifier.
 * \param[out] size Maximum payload size in bytes.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `service_id` parameter was not a valid system entity
 *        identifier.
 *      - The `size` parameter was a null pointer value.
 * \retval ::FWK_E_INIT The service is not initialized.
 * \retval ::FWK_E_STATE The service is in an invalid sate.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int mod_scmi_from_protocol_api_get_max_payload_size(
    fwk_id_t service_id,
    size_t *size);

/*!
 * \brief Write part of a payload through a service.
 *
 * \param service_id Service identifier.
 * \param offset Offset to begin writing at.
 * \param payload Payload data to write.
 * \param size Size of the payload data.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 * \retval ::FWK_E_PARAM An invalid parameter was encountered:
 *      - The `service_id` parameter was not a valid system entity
 *        identifier.
 *      - The offset and size given were not within the bounds of the
 *        payload area.
 * \return One of the standard error codes for implementation-defined
 *      errors.
 */
int mod_scmi_from_protocol_api_write_payload(
    fwk_id_t service_id,
    size_t offset,
    const void *payload,
    size_t size);

/*!
 * \brief Respond to an SCMI message on a service.
 *
 * \param service_id Service identifier.
 * \param payload Payload data to write, or NULL if a payload has already
 *      been written.
 * \param size Size of the payload.
 */
int mod_scmi_from_protocol_api_respond(
    fwk_id_t service_id,
    const void *payload,
    size_t size);

/*!
 * \brief Send a notification to the agent on behalf on an SCMI service.
 *
 * \param service_id Service identifier.
 * \param protocol_id Protocol identifier.
 * \param message_id Message identifier.
 * \param payload Payload data to write, or NULL if a payload has already
 *         been written.
 * \param size Size of the payload in bytes.
 */
void mod_scmi_from_protocol_api_notify(
    fwk_id_t service_id,
    int protocol_id,
    int message_id,
    const void *payload,
    size_t size);

/*!
 * \brief Send an SCMI message
 *
 * \param scmi_message_id SCMI message identifier.
 * \param scmi_protocol_id SCMI message protocol identifier.
 * \param token SCMI message token.
 * \param service_id SCMI service identifier.
 * \param payload Payload data to write
 * \param payload_size size of the payload in bytes.
 * \param request_ack_by_interrupt flag to select whether acknowledgement
 * interrupt is required for this message.
 */
int mod_scmi_from_protocol_api_scmi_send_message(
    uint8_t scmi_message_id,
    uint8_t scmi_protocol_id,
    uint8_t token,
    fwk_id_t service_id,
    const void *payload,
    size_t payload_size,
    bool request_ack_by_interrupt);

/*!
 * \brief Handle response SCMI message
 *
 * \param service_id Service identifier.
 *
 * \retval ::FWK_SUCCESS The operation succeeded.
 */
int mod_scmi_from_protocol_api_response_message_handler(fwk_id_t service_id);

/*!
 * \brief Get the current operating point of a domain.
 *
 * \param domain_id Element identifier of the domain.
 * \param [out] opp Current operating point.
 */
int mod_dvfs_domain_api_get_current_opp(
    fwk_id_t domain_id,
    struct mod_dvfs_opp *opp);

/*!
 * \brief Get the sustained operating point of a domain.
 *
 * \param domain_id Element identifier of the domain.
 * \param [out] opp Sustained operating point.
 */
int mod_dvfs_domain_api_get_sustained_opp(
    fwk_id_t domain_id,
    struct mod_dvfs_opp *opp);

/*!
 * \brief Get an operating point from its index.
 *
 * \param domain_id Element identifier of the domain.
 * \param n Index of the operating point to retrieve.
 * \param [out] opp Requested operating point.
 */
int mod_dvfs_domain_api_get_nth_opp(
    fwk_id_t domain_id,
    size_t n,
    struct mod_dvfs_opp *opp);

/*!
 * \brief Get the number of operating points of a domain.
 *
 * \param domain_id Element identifier of the domain.
 * \param [out] opp_count Number of operating points.
 */
int mod_dvfs_domain_api_get_opp_count(fwk_id_t domain_id, size_t *opp_count);

/*!
 * \brief Get the level id for the given level.
 *
 * \param domain_id Element identifier of the domain.
 * \param level Requested level.
 * \param [out] level id inside the OPP table.
 */
int mod_dvfs_domain_api_get_level_id(
    fwk_id_t domain_id,
    uint32_t level,
    size_t *level_id);

/*!
 * \brief Get the worst-case transition latency of a domain.
 *
 * \param domain_id Element identifier of the domain.
 * \param [out] latency Worst-case transition latency.
 */
int mod_dvfs_domain_api_get_latency(fwk_id_t domain_id, uint16_t *latency);

/*!
 * \brief Set the level of a domain.
 *
 * \param domain_id Element identifier of the domain.
 * \param cookie Context-specific value.
 * \param level Requested level.
 */
int mod_dvfs_domain_api_set_level(
    fwk_id_t domain_id,
    uintptr_t cookie,
    uint32_t level);

/*!
 * \brief Plugin's implementation of the 'update' API.
 */
int plugin_update(struct perf_plugins_perf_update *data);

/*!
 * \brief Plugin's implementation of the 'report' API.
 */
int plugin_report(struct perf_plugins_perf_report *data);

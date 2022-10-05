/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020-2022, Renesas Electronics Corporation. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <rcar_scmi.h>

#include <internal/scmi.h>

#include <mod_scmi.h>
#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element element_table[] = {
    [RCAR_SCMI_SERVICE_IDX_PSCI] = {
        .name = "PSCI",
        .data = &(struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                RCAR_SCMI_SERVICE_IDX_PSCI),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
            .transport_notification_init_id =
                FWK_ID_NOTIFICATION_INIT(FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_NOTIFICATION_IDX_INITIALIZED),
            .scmi_agent_id = SCMI_AGENT_ID_PSCI,
        },
    },
    [RCAR_SCMI_SERVICE_IDX_OSPM] = {
        .name = "OSPM",
        .data = &(struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                RCAR_SCMI_SERVICE_IDX_OSPM),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
            .transport_notification_init_id =
                FWK_ID_NOTIFICATION_INIT(FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_NOTIFICATION_IDX_INITIALIZED),
            .scmi_agent_id = SCMI_AGENT_ID_OSPM,
        },
    },
    [RCAR_SCMI_SERVICE_IDX_VMM] = {
        .name = "VMM",
        .data = &(struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                RCAR_SCMI_SERVICE_IDX_VMM),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
            .transport_notification_init_id =
                FWK_ID_NOTIFICATION_INIT(FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_NOTIFICATION_IDX_INITIALIZED),
            .scmi_agent_id = SCMI_AGENT_ID_VMM,
        },
    },
    [RCAR_SCMI_SERVICE_IDX_VM1] = {
        .name = "VM1",
        .data = &(struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                RCAR_SCMI_SERVICE_IDX_VM1),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
            .transport_notification_init_id =
                FWK_ID_NOTIFICATION_INIT(FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_NOTIFICATION_IDX_INITIALIZED),
            .scmi_agent_id = SCMI_AGENT_ID_VM1,
        },
    },
    [RCAR_SCMI_SERVICE_IDX_VM2] = {
        .name = "VM2",
        .data = &(struct mod_scmi_service_config) {
            .transport_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                RCAR_SCMI_SERVICE_IDX_VM2),
            .transport_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
            .transport_notification_init_id =
                FWK_ID_NOTIFICATION_INIT(FWK_MODULE_IDX_TRANSPORT,
                MOD_TRANSPORT_NOTIFICATION_IDX_INITIALIZED),
            .scmi_agent_id = SCMI_AGENT_ID_VM2,
        },
    },
    [RCAR_SCMI_SERVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *get_element_table(fwk_id_t module_id)
{
    return element_table;
}

static const struct mod_scmi_agent agent_table[] = {
    [SCMI_AGENT_ID_OSPM] = {
        .type = SCMI_AGENT_TYPE_OSPM,
        .name = "OSPM",
    },
    [SCMI_AGENT_ID_PSCI] = {
        .type = SCMI_AGENT_TYPE_PSCI,
        .name = "PSCI",
    },
    [SCMI_AGENT_ID_VMM] = {
        .type = SCMI_AGENT_TYPE_OSPM,
        .name = "VMM",
    },
    [SCMI_AGENT_ID_VM1] = {
        .type = SCMI_AGENT_TYPE_OSPM,
        .name = "VM1",
    },
    [SCMI_AGENT_ID_VM2] = {
        .type = SCMI_AGENT_TYPE_OSPM,
        .name = "VM2",
    },
};

struct fwk_module_config config_scmi = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(get_element_table),
    .data = &((struct mod_scmi_config){
        .protocol_count_max = 9,
#ifndef BUILD_HAS_MOD_RESOURCE_PERMS
        /* No protocols are disabled for PSCI agents */
        .dis_protocol_count_psci = 0,
        .dis_protocol_list_psci = NULL,
#endif
        .agent_count = FWK_ARRAY_SIZE(agent_table) - 1,
        .agent_table = agent_table,
        .vendor_identifier = "renesas",
        .sub_vendor_identifier = "renesas",
    }),
};

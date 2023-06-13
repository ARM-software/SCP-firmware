/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_mmap.h"
#include "scp_tc1_mhu.h"
#include "tc1_core.h"
#include "tc1_scmi.h"

#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static const struct fwk_element transport_element_table[
    SCP_TC1_SCMI_SERVICE_IDX_COUNT + 1] = {
    [SCP_TC1_SCMI_SERVICE_IDX_PSCI] = {
        .name = "PSCI",
        .data = &((
            struct mod_transport_channel_config){
            .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .policies =
                MOD_TRANSPORT_POLICY_INIT_MAILBOX |
                MOD_TRANSPORT_POLICY_SECURE,
            .out_band_mailbox_address = (uintptr_t)
                SCP_SCMI_PAYLOAD_S_A2P_BASE,
            .out_band_mailbox_size =
                SCP_SCMI_PAYLOAD_SIZE,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                FWK_MODULE_IDX_MHU2,
                SCP_TC1_MHU_DEVICE_IDX_SCP_AP_S_CLUS0,
                0),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MHU2,
                0),
        }),
     },
    [SCP_TC1_SCMI_SERVICE_IDX_OSPM_0] = {
        .name = "OSPM0",
        .data = &((
            struct mod_transport_channel_config){
            .channel_type =
                MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .policies =
                MOD_TRANSPORT_POLICY_INIT_MAILBOX,
            .out_band_mailbox_address = (uintptr_t)
                SCP_SCMI_PAYLOAD0_NS_A2P_BASE,
            .out_band_mailbox_size =
                SCP_SCMI_PAYLOAD_SIZE,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                FWK_MODULE_IDX_MHU2,
                SCP_TC1_MHU_DEVICE_IDX_SCP_AP_NS_HP_CLUS0,
                0),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MHU2,
                0),
        }),
    },
    [SCP_TC1_SCMI_SERVICE_IDX_OSPM_1] = {
        .name = "OSPM1",
        .data = &((
            struct mod_transport_channel_config){
            .channel_type =
                MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
            .policies =
                MOD_TRANSPORT_POLICY_INIT_MAILBOX,
            .out_band_mailbox_address = (uintptr_t)
                SCP_SCMI_PAYLOAD1_NS_A2P_BASE,
            .out_band_mailbox_size =
                SCP_SCMI_PAYLOAD_SIZE,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(
                FWK_MODULE_IDX_MHU2,
                SCP_TC1_MHU_DEVICE_IDX_SCP_AP_NS_LP_CLUS0,
                0),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MHU2,
                0),
        }),
    },
    [SCP_TC1_SCMI_SERVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *transport_get_element_table(fwk_id_t module_id)
{
    return transport_element_table;
}

const struct fwk_module_config config_transport = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(transport_get_element_table),
};

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Configuration data for module 'transport'.
 */

#include "platform_core.h"
#include "scp_cfgd_mhu3.h"
#include "scp_cfgd_transport.h"
#include "scp_clock.h"
#include "scp_fw_mmap.h"

#include <mod_atu.h>
#include <mod_mhu3.h>
#include <mod_scp_platform.h>
#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

/* Module 'transport' element count */
#define MOD_TRANSPORT_ELEMENT_COUNT (SCP_CFGD_MOD_TRANSPORT_EIDX_COUNT + 1)

/* Secure transport channel with mailbox initialization policy */
#define TRANSPORT_CH_SEC_MBX_INIT \
    (MOD_TRANSPORT_POLICY_INIT_MAILBOX | MOD_TRANSPORT_POLICY_SECURE)

/* Subsystem initialized notification id (platform notification) */
#define PLATFORM_SCP_NOTIFICATION_ID \
    FWK_ID_NOTIFICATION_INIT( \
        FWK_MODULE_IDX_SCP_PLATFORM, \
        MOD_SCP_PLATFORM_NOTIFICATION_IDX_SUBSYS_INITIALIZED)

/* Module 'transport' element configuration table */
static const struct fwk_element element_table[MOD_TRANSPORT_ELEMENT_COUNT]  = {
    [SCP_CFGD_MOD_TRANSPORT_EIDX_PSCI] = {
        .name = "PSCI",
        .data = &((
            struct mod_transport_channel_config) {
                .transport_type = MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_OUT_BAND,
                .policies = TRANSPORT_CH_SEC_MBX_INIT,
                .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
                .out_band_mailbox_address =
                    (uintptr_t) SCP_SCMI_PAYLOAD_S_A2P_BASE,
                .out_band_mailbox_size = SCP_SCMI_PAYLOAD_SIZE,
                .driver_id =
                    FWK_ID_SUB_ELEMENT_INIT(
                        FWK_MODULE_IDX_MHU3,
                        SCP_CFGD_MOD_MHU3_EIDX_SCP_AP_S_CLUS0,
                        0),
                .driver_api_id =
                    FWK_ID_API_INIT(
                        FWK_MODULE_IDX_MHU3,
                        MOD_MHU3_API_IDX_TRANSPORT_DRIVER),
                .platform_notification = {
                    .notification_id = PLATFORM_SCP_NOTIFICATION_ID,
                    .source_id = FWK_ID_MODULE_INIT(
                        FWK_MODULE_IDX_SCP_PLATFORM),
                },
        }),
    },
    [SCP_CFGD_MOD_TRANSPORT_EIDX_SYSTEM] = {
        .name = "SCP_PLATFORM_TRANSPORT",
        .data = &((
            struct mod_transport_channel_config) {
                .transport_type = MOD_TRANSPORT_CHANNEL_TRANSPORT_TYPE_NONE,
                .policies = MOD_TRANSPORT_POLICY_NONE,
                .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
                .signal_api_id =
                    FWK_ID_API_INIT(
                        FWK_MODULE_IDX_SCP_PLATFORM,
                        MOD_SCP_PLATFORM_API_IDX_TRANSPORT_SIGNAL),
                .driver_id =
                    FWK_ID_SUB_ELEMENT_INIT(
                        FWK_MODULE_IDX_MHU3,
                        SCP_CFGD_MOD_MHU3_EIDX_SCP_RSS_S,
                        1),
                .driver_api_id =
                    FWK_ID_API_INIT(
                        FWK_MODULE_IDX_MHU3,
                        MOD_MHU3_API_IDX_TRANSPORT_DRIVER),
        }),
    },
    [SCP_CFGD_MOD_TRANSPORT_EIDX_COUNT] = { 0 },
};

static const struct fwk_element *transport_get_element_table(fwk_id_t module_id)
{
    return element_table;
}

const struct fwk_module_config config_transport = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(transport_get_element_table),
};

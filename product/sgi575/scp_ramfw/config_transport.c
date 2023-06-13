/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "scp_sgi575_mhu.h"
#include "scp_sgi575_scmi.h"
#include "scp_software_mmap.h"
#include "sgi575_core.h"

#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static const struct fwk_element transport_element_table[] = {
    /* SCP_SGI575_SCMI_SERVICE_IDX_PSCI */
    { .name = "PSCI",
      .data = &((struct mod_transport_channel_config){
          .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
          .policies =
              MOD_TRANSPORT_POLICY_INIT_MAILBOX | MOD_TRANSPORT_POLICY_SECURE,
          .out_band_mailbox_address = (uintptr_t)SCP_SCMI_PAYLOAD_S_A2P_BASE,
          .out_band_mailbox_size = SCP_SCMI_PAYLOAD_SIZE,
          .driver_id = FWK_ID_SUB_ELEMENT_INIT(
              FWK_MODULE_IDX_MHU,
              SCP_SGI575_MHU_DEVICE_IDX_SCP_AP_S,
              0),
          .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
      }) },
    /* SCP_SGI575_SCMI_SERVICE_IDX_OSPM */
    { .name = "OSPM",
      .data = &((struct mod_transport_channel_config){
          .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
          .policies = MOD_TRANSPORT_POLICY_INIT_MAILBOX,
          .out_band_mailbox_address = (uintptr_t)SCP_SCMI_PAYLOAD_NS_A2P_BASE,
          .out_band_mailbox_size = SCP_SCMI_PAYLOAD_SIZE,
          .driver_id = FWK_ID_SUB_ELEMENT_INIT(
              FWK_MODULE_IDX_MHU,
              SCP_SGI575_MHU_DEVICE_IDX_SCP_AP_NS,
              0),
          .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
      }) },
    [SCP_SGI575_SCMI_SERVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *transport_get_element_table(fwk_id_t module_id)
{
    return transport_element_table;
}

const struct fwk_module_config config_transport = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(transport_get_element_table),
};

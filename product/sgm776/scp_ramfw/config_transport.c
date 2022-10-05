/*
 * Arm SCP/MCP Software
 * Copyright (c) 2015-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_power_domain.h"
#include "sgm776_core.h"
#include "sgm776_mhu.h"
#include "sgm776_scmi.h"
#include "software_mmap.h"

#include <mod_mhu2.h>
#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static const struct fwk_element transport_element_table[] = {
    /* SGM776_SCMI_SERVICE_IDX_PSCI */
    { .name = "PSCI",
      .data = &((struct mod_transport_channel_config){
          .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
          .policies =
              MOD_TRANSPORT_POLICY_INIT_MAILBOX | MOD_TRANSPORT_POLICY_SECURE,
          .out_band_mailbox_address = (uintptr_t)SCMI_PAYLOAD_S_A2P_BASE,
          .out_band_mailbox_size = SCMI_PAYLOAD_SIZE,
          .driver_id = FWK_ID_SUB_ELEMENT_INIT(
              FWK_MODULE_IDX_MHU2,
              SGM776_MHU_DEVICE_IDX_S,
              0),
          .driver_api_id = FWK_ID_API_INIT(
              FWK_MODULE_IDX_MHU2,
              MOD_MHU2_API_IDX_TRANSPORT_DRIVER),
      }) },
    /* SGM776_SCMI_SERVICE_IDX_OSPM_0 */
    { .name = "OSPM0",
      .data = &((struct mod_transport_channel_config){
          .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
          .policies = MOD_TRANSPORT_POLICY_INIT_MAILBOX,
          .out_band_mailbox_address = (uintptr_t)SCMI_PAYLOAD0_NS_A2P_BASE,
          .out_band_mailbox_size = SCMI_PAYLOAD_SIZE,
          .driver_id = FWK_ID_SUB_ELEMENT_INIT(
              FWK_MODULE_IDX_MHU2,
              SGM776_MHU_DEVICE_IDX_NS_L,
              0),
          .driver_api_id = FWK_ID_API_INIT(
              FWK_MODULE_IDX_MHU2,
              MOD_MHU2_API_IDX_TRANSPORT_DRIVER),
      }) },
    /* SGM776_SCMI_SERVICE_IDX_OSPM_1 */
    { .name = "OSPM1",
      .data = &((struct mod_transport_channel_config){
          .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
          .policies = MOD_TRANSPORT_POLICY_INIT_MAILBOX,
          .out_band_mailbox_address = (uintptr_t)SCMI_PAYLOAD1_NS_A2P_BASE,
          .out_band_mailbox_size = SCMI_PAYLOAD_SIZE,
          .driver_id = FWK_ID_SUB_ELEMENT_INIT(
              FWK_MODULE_IDX_MHU2,
              SGM776_MHU_DEVICE_IDX_NS_H,
              0),
          .driver_api_id = FWK_ID_API_INIT(
              FWK_MODULE_IDX_MHU2,
              MOD_MHU2_API_IDX_TRANSPORT_DRIVER),
      }) },
    [SGM776_SCMI_SERVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *transport_get_element_table(fwk_id_t module_id)
{
    unsigned int idx;
    struct mod_transport_channel_config *config;

    for (idx = 0; idx < SGM776_SCMI_SERVICE_IDX_COUNT; idx++) {
        config =
            (struct mod_transport_channel_config *)(transport_element_table[idx]
                                                        .data);

        config->pd_source_id = FWK_ID_ELEMENT(
            FWK_MODULE_IDX_POWER_DOMAIN,
            CONFIG_POWER_DOMAIN_SYSTOP_SYSTEM + sgm776_core_get_count() +
                sgm776_cluster_get_count());
    }

    return transport_element_table;
}

struct fwk_module_config config_transport = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(transport_get_element_table),
};

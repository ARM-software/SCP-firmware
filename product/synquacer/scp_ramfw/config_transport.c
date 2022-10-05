/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "config_mhu.h"
#include "config_power_domain.h"
#include "config_scmi.h"
#include "synquacer_core.h"
#include "synquacer_mmap.h"

#include <mod_transport.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <stdint.h>

static const struct fwk_element transport_element_table[] = {
    /* SCP_SYNQUACER_SCMI_SERVICE_IDX_PSCI */
    { .name = "PSCI",
      .data = &((struct mod_transport_channel_config){
          .channel_type = MOD_TRANSPORT_CHANNEL_TYPE_COMPLETER,
          .policies =
              MOD_TRANSPORT_POLICY_INIT_MAILBOX | MOD_TRANSPORT_POLICY_SECURE,
          .out_band_mailbox_address = (uintptr_t)MHU_PAYLOAD_S_BASE,
          .out_band_mailbox_size = 256,
          .driver_id = FWK_ID_SUB_ELEMENT_INIT(
              FWK_MODULE_IDX_MHU,
              SCP_SYNQUACER_MHU_DEVICE_IDX_SCP_AP_S,
              0),
          .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MHU, 0),
      }) },
    [SCP_SYNQUACER_SCMI_SERVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *transport_get_element_table(fwk_id_t module_id)
{
    unsigned int idx;
    struct mod_transport_channel_config *config;

    for (idx = 0; idx < SCP_SYNQUACER_SCMI_SERVICE_IDX_COUNT; idx++) {
        config =
            (struct mod_transport_channel_config *)(transport_element_table[idx]
                                                        .data);
        config->pd_source_id = FWK_ID_ELEMENT(
            FWK_MODULE_IDX_POWER_DOMAIN,
            synquacer_core_get_core_count() +
                synquacer_core_get_cluster_count() + PD_STATIC_DEV_IDX_SYSTOP);
    }

    return transport_element_table;
}

const struct fwk_module_config config_transport = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(transport_get_element_table),
};

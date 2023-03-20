/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_scmi.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_macros.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_dt_config_common.h>


#define SCMI_SVC_COMPAT arm_scp_scmi_service
#define SCMI_SVC_PROP(n, prop) FWK_DT_INST_PROP(SCMI_SVC_COMPAT, n, prop)

/* SMCI service table element generation
   
   The device tree fragment:

   scmi-service@0 {
       compatible = "arm,scp-scmi-service";
       label = "PSCI";
       service-idx = <JUNO_SCMI_SERVICE_IDX_PSCI_A2P>;
       scp-ids = <&elem_id FWK_MODULE_IDX_TRANSPORT JUNO_SCMI_SERVICE_IDX_PSCI_A2P>,
        <&elem_id FWK_MODULE_IDX_TRANSPORT MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT>,
       <&elem_id FWK_MODULE_IDX_TRANSPORT  MOD_TRANSPORT_NOTIFICATION_IDX_INITIALIZED>;
       scmi-agent-id = <JUNO_SCMI_AGENT_IDX_PSCI>;
   };

   will generate the following table entry:

  [JUNO_SCMI_SERVICE_IDX_PSCI_A2P] = {
    .name = "PSCI",
    .data = &(struct mod_scmi_service_config) {
      .transport_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_TRANSPORT, JUNO_SCMI_SERVICE_IDX_PSCI_A2P),
      .transport_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_TRANSPORT, MOD_TRANSPORT_API_IDX_SCMI_TO_TRANSPORT),
      .transport_notification_init_id = FWK_ID_NOTIFICATION_INIT(FWK_MODULE_IDX_TRANSPORT, MOD_TRANSPORT_NOTIFICATION_IDX_INITIALIZED),
      .scmi_agent_id = (unsigned int) JUNO_SCMI_AGENT_IDX_PSCI,
      .scmi_p2a_id = FWK_ID_NONE_INIT,
  },
*/

#define SCMI_SVC_ELEM_INIT(n) \
    [SCMI_SVC_PROP(n, service_idx)] = {  \
        .name = SCMI_SVC_PROP(n, label),             \
        .data = &(struct mod_scmi_service_config) { \
            .transport_id = FWK_DT_PH_IDX_SCP_ID_ELEM(SCMI_SVC_COMPAT, n, 0), \
            .transport_api_id = FWK_DT_PH_IDX_SCP_ID_API(SCMI_SVC_COMPAT, n, 1), \
            .transport_notification_init_id = FWK_DT_PH_IDX_SCP_ID_NTFY(SCMI_SVC_COMPAT, n, 2), \
            .scmi_agent_id = SCMI_SVC_PROP(n, scmi_agent_id),           \
            .scmi_p2a_id = FWK_DT_OPT_FWK_ELEM_ID(SCMI_SVC_COMPAT, n, scmi_p2a_id), \
         }, \
   },

static struct fwk_element element_table[] = {
    /* macro for array elements */
    DT_FOREACH_OKAY_INST_arm_scp_scmi_service(SCMI_SVC_ELEM_INIT)
    /* last NULL element */
    [DT_N_INST_arm_scp_scmi_service_NUM_OKAY] = { 0 },
};

#define SCMI_AT_COMPAT arm_scp_scmi_agent
#define SCMI_AT_PROP(n, prop) FWK_DT_INST_PROP(SCMI_AT_COMPAT, n , prop)

/* SCMI Agent table generation

  Device tree fragment:

  scmi-agent-pcsi {
      compatible = "arm,scp-scmi-agent";
      label = "PCSI";
      agent-type = <SCMI_AGENT_TYPE_PSCI>;
      agent-idx = <JUNO_SCMI_AGENT_IDX_PSCI>;
  };

  generates the following array entry
  
  mod_scmi_agent agent_table[] = {
     [JUNO_SCMI_AGENT_IDX_PSCI] = {
        .type = SCMI_AGENT_TYPE_PSCI,
        .name = "PSCI",
        },
 */

#define SCMI_AT_INIT(n) \
    [SCMI_AT_PROP(n, agent_idx)] = { \
        .type = SCMI_AT_PROP(n, agent_type), \
        .name = SCMI_AT_PROP(n, label), },

static const struct mod_scmi_agent agent_table[] = {
    DT_FOREACH_OKAY_INST_arm_scp_scmi_agent(SCMI_AT_INIT)    
};


#define SCMI_COMPAT arm_scp_scmi
#define SCMI_NODE_ID DT_INST(0, SCMI_COMPAT)
#define SCMI_PROP(prop) FWK_DT_INST_PROP(SCMI_COMPAT, 0 , prop)

#ifndef BUILD_HAS_MOD_RESOURCE_PERMS

#define DIS_PROTO_TABLE_ENTRY(node_id, prop, idx) \
    DT_PROP_BY_IDX(node_id, prop, idx),

/* PSCI agent has no access to clock, perf and sensor protocol

  device tree fragment:

  scmi-base {
     psci-dis-protocols = <MOD_SCMI_PROTOCOL_ID_SENSOR
                           MOD_SCMI_PROTOCOL_ID_CLOCK
                           MOD_SCMI_PROTOCOL_ID_PERF>;
  generates the table:

  uint32_t dis_protocol_list_psci[] = { 
    MOD_SCMI_PROTOCOL_ID_SENSOR,
    MOD_SCMI_PROTOCOL_ID_CLOCK,
    MOD_SCMI_PROTOCOL_ID_PERF,
  };
 */

static const uint32_t dis_protocol_list_psci[] = {
    DT_FOREACH_PROP_ELEM(SCMI_NODE_ID, psci_dis_protocols, DIS_PROTO_TABLE_ENTRY) \
};
#endif

/* access data table from dynamic element callback */
#ifdef FWK_MODULE_GEN_DYNAMIC_SCMI
const struct fwk_element *_static_get_element_table_scmi() {
    return element_table;
}
#endif

/* must only be one SCMI base config object in the device tree */
static_assert(DT_N_INST_arm_scp_scmi_NUM_OKAY == 1,
              "Error - can only have a single scmi base config object in device tree");

/* config structure */
struct fwk_module_config config_dt_scmi = {
    .data =
        &(struct mod_scmi_config){
             .protocol_count_max = SCMI_PROP(protocol_count_max),
#ifndef BUILD_HAS_MOD_RESOURCE_PERMS
            .dis_protocol_count_psci = FWK_ARRAY_SIZE(dis_protocol_list_psci),
            .dis_protocol_list_psci = dis_protocol_list_psci,
#endif
            .agent_count = FWK_ARRAY_SIZE(agent_table) - 1,
            .agent_table = agent_table,
            .vendor_identifier = SCMI_PROP(vendor_identifier),
            .sub_vendor_identifier = SCMI_PROP(sub_vendor_identifier),
        },
#ifdef FWK_MODULE_GEN_DYNAMIC_SCMI
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(dyn_get_element_table_scmi),
#else
    .elements = FWK_MODULE_STATIC_ELEMENTS_PTR(element_table),
#endif
};

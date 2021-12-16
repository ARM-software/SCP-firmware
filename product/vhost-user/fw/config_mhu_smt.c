/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <scmi_agents.h>
#include <mod_msg_smt.h>
#include <mod_vhost_mhu.h>

static const struct fwk_element mhu_element_table[] = {
    [SCMI_CHANNEL_DEVICE_IDX_PSCI] = {
        .name = "virqueue for PSCI",
        .sub_element_count = 1,
        .data = &((struct mod_vhost_mhu_channel_config) {
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT,
                                             MOD_MSG_SMT_API_IDX_DRIVER_INPUT),
        })
    },
    [SCMI_CHANNEL_DEVICE_IDX_OSPM_0] = {
        .name = "virqueue for OSPM #0",
        .sub_element_count = 2,
        .data = &((struct mod_vhost_mhu_channel_config) {
            .socket_path ="vscmi-guest0.sock",
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT,
                                             MOD_MSG_SMT_API_IDX_DRIVER_INPUT),
        })
    },
    [SCMI_CHANNEL_DEVICE_IDX_OSPM_1] = {
        .name = "virqueue for OSPM #1",
        .sub_element_count = 2,
        .data = &((struct mod_vhost_mhu_channel_config) {
            .socket_path ="vscmi-guest1.sock",
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT,
                                             MOD_MSG_SMT_API_IDX_DRIVER_INPUT),
        })
    },
    [SCMI_CHANNEL_DEVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *mhu_get_element_table(fwk_id_t module_id)
{
    return (const struct fwk_element *)mhu_element_table;
}

struct fwk_module_config config_vhost_mhu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(mhu_get_element_table),
};

static struct fwk_element smt_element_table[] = {
    [SCMI_SERVICE_IDX_PSCI] = {
        .name = "PSCI",
        .data = &((struct mod_msg_smt_channel_config) {
            .type = MOD_MSG_SMT_CHANNEL_TYPE_REQUESTER,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_VHOST_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_PSCI, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_VHOST_MHU, 0),
        })
    },
    [SCMI_SERVICE_IDX_OSPM_0] = {
        .name = "OSPM0",
        .data = &((struct mod_msg_smt_channel_config) {
            .type = MOD_MSG_SMT_CHANNEL_TYPE_REQUESTER,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_VHOST_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_OSPM_0, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_VHOST_MHU, 0),
        })
    },
    [SCMI_SERVICE_IDX_OSPM_1] = {
        .name = "OSPM1",
        .data = &((struct mod_msg_smt_channel_config) {
            .type = MOD_MSG_SMT_CHANNEL_TYPE_REQUESTER,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_VHOST_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_OSPM_1, 0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_VHOST_MHU, 0),
        })
    },
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [SCMI_SERVICE_IDX_OSPM_0_P2A] = {
        .name = "OSPM0 P2A",
        .data = &((struct mod_msg_smt_channel_config) {
            .type = MOD_MSG_SMT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_VHOST_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_OSPM_0, 1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_VHOST_MHU, 0),
        })
    },
    [SCMI_SERVICE_IDX_OSPM_1_P2A] = {
        .name = "OSPM1 P2A",
        .data = &((struct mod_msg_smt_channel_config) {
            .type = MOD_MSG_SMT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_SUB_ELEMENT_INIT(FWK_MODULE_IDX_VHOST_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_OSPM_1, 1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_VHOST_MHU, 0),
        })
    },
#endif
    [SCMI_SERVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *smt_get_element_table(fwk_id_t module_id)
{
    return (const struct fwk_element *)smt_element_table;
}

struct fwk_module_config config_msg_smt = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(smt_get_element_table),
};

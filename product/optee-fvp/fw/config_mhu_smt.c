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
#include <mod_optee_mhu.h>


static const struct fwk_element mhu_element_table[] = {
    [SCMI_CHANNEL_DEVICE_IDX_PSCI] = {
        .name = "OP-TEE for PSCI",
    .data = &((struct mod_optee_mhu_channel_config) {
            .type = MOD_OPTEE_MHU_CHANNEL_TYPE_REQUESTER,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MSG_SMT,
                                         SCMI_SERVICE_IDX_PSCI),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT,
                                             MOD_MSG_SMT_API_IDX_DRIVER_INPUT),
        })
    },
    [SCMI_CHANNEL_DEVICE_IDX_OSPM_0] = {
        .name = "OP-TEE for OSPM #0",
        .data = &((struct mod_optee_mhu_channel_config) {
            .type = MOD_OPTEE_MHU_CHANNEL_TYPE_REQUESTER,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MSG_SMT,
                                         SCMI_SERVICE_IDX_OSPM_0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT,
                                             MOD_MSG_SMT_API_IDX_DRIVER_INPUT),
        })
    },
    [SCMI_CHANNEL_DEVICE_IDX_OSPM_1] = {
        .name = "OP-TEE for OSPM #1",
        .data = &((struct mod_optee_mhu_channel_config) {
            .type = MOD_OPTEE_MHU_CHANNEL_TYPE_REQUESTER,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MSG_SMT,
                                         SCMI_SERVICE_IDX_OSPM_1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT,
                                             MOD_MSG_SMT_API_IDX_DRIVER_INPUT),
        })
    },
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [SCMI_CHANNEL_DEVICE_IDX_OSPM_0_P2A] = {
        .name = "OP-TEE for OSPM #0 P2A",
        .data = &((struct mod_optee_mhu_channel_config) {
            .type = MOD_OPTEE_MHU_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MSG_SMT,
                                         SCMI_SERVICE_IDX_OSPM_0_P2A),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT,
                                             MOD_MSG_SMT_API_IDX_DRIVER_INPUT),
        })
    },
    [SCMI_CHANNEL_DEVICE_IDX_OSPM_1_P2A] = {
        .name = "OP-TEE for OSPM #1 P2A",
        .data = &((struct mod_optee_mhu_channel_config) {
            .type = MOD_OPTEE_MHU_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_MSG_SMT,
                                         SCMI_SERVICE_IDX_OSPM_1_P2A),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_MSG_SMT,
                                             MOD_MSG_SMT_API_IDX_DRIVER_INPUT),
        })
    },
#endif
    [SCMI_CHANNEL_DEVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *mhu_get_element_table(fwk_id_t module_id)
{
    return (const struct fwk_element *)mhu_element_table;
}

struct fwk_module_config config_optee_mhu = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(mhu_get_element_table),
};

static struct fwk_element smt_element_table[] = {
    [SCMI_SERVICE_IDX_PSCI] = {
        .name = "PSCI",
        .data = &((struct mod_msg_smt_channel_config) {
            .type = MOD_MSG_SMT_CHANNEL_TYPE_REQUESTER,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_PSCI),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MHU, 0),
        })
    },
    [SCMI_SERVICE_IDX_OSPM_0] = {
        .name = "OSPM0",
        .data = &((struct mod_msg_smt_channel_config) {
            .type = MOD_MSG_SMT_CHANNEL_TYPE_REQUESTER,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_OSPM_0),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MHU, 0),
        })
    },
    [SCMI_SERVICE_IDX_OSPM_1] = {
        .name = "OSPM1",
        .data = &((struct mod_msg_smt_channel_config) {
            .type = MOD_MSG_SMT_CHANNEL_TYPE_REQUESTER,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_OSPM_1),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MHU, 0),
        })
    },
#ifdef BUILD_HAS_SCMI_NOTIFICATIONS
    [SCMI_SERVICE_IDX_OSPM_0_P2A] = {
        .name = "OSPM0 P2A",
        .data = &((struct mod_msg_smt_channel_config) {
            .type = MOD_MSG_SMT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_OSPM_0_P2A),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MHU, 0),
        })
    },
    [SCMI_SERVICE_IDX_OSPM_1_P2A] = {
        .name = "OSPM1 P2A",
        .data = &((struct mod_msg_smt_channel_config) {
            .type = MOD_MSG_SMT_CHANNEL_TYPE_COMPLETER,
            .driver_id = FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_MHU,
                                                 SCMI_CHANNEL_DEVICE_IDX_OSPM_1_P2A),
            .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MHU, 0),
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

/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <scmi_agents.h>

#include <mod_msg_smt.h>
#include <mod_optee_mbx.h>

#include <fwk_module.h>
#include <fwk_module_idx.h>

static const struct fwk_element mbx_element_table[] = {
    [SCMI_CHANNEL_DEVICE_IDX_PSCI] = {
        .name = "OP-TEE for PSCI",
        .data = &((struct mod_optee_mbx_channel_config){
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MSG_SMT,
                SCMI_SERVICE_IDX_PSCI),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MSG_SMT,
                MOD_MSG_SMT_API_IDX_DRIVER_INPUT),
        }),
    },
    [SCMI_CHANNEL_DEVICE_IDX_OSPM_0] = {
        .name = "OP-TEE for OSPM #0",
        .data = &((struct mod_optee_mbx_channel_config){
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MSG_SMT,
                SCMI_SERVICE_IDX_OSPM_0),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MSG_SMT,
                MOD_MSG_SMT_API_IDX_DRIVER_INPUT),
        }),
    },
    [SCMI_CHANNEL_DEVICE_IDX_OSPM_1] = {
        .name = "OP-TEE for OSPM #1",
        .data = &((struct mod_optee_mbx_channel_config){
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_MSG_SMT,
                SCMI_SERVICE_IDX_OSPM_1),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_MSG_SMT,
                MOD_MSG_SMT_API_IDX_DRIVER_INPUT),
        }),
    },
    [SCMI_CHANNEL_DEVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *mbx_get_element_table(fwk_id_t module_id)
{
    return (const struct fwk_element *)mbx_element_table;
}

struct fwk_module_config config_optee_mbx = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(mbx_get_element_table),
};

static struct fwk_element smt_element_table[] = {
    [SCMI_SERVICE_IDX_PSCI] = {
        .name = "PSCI",
        .data = &((struct mod_msg_smt_channel_config){
            .type = MOD_MSG_SMT_CHANNEL_TYPE_REQUESTER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_OPTEE_MBX,
                SCMI_CHANNEL_DEVICE_IDX_PSCI),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_OPTEE_MBX,
                0),
        }),
    },
    [SCMI_SERVICE_IDX_OSPM_0] = {
        .name = "OSPM0",
        .data = &((struct mod_msg_smt_channel_config){
            .type = MOD_MSG_SMT_CHANNEL_TYPE_REQUESTER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_OPTEE_MBX,
                SCMI_CHANNEL_DEVICE_IDX_OSPM_0),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_OPTEE_MBX,
                0),
        }),
    },
    [SCMI_SERVICE_IDX_OSPM_1] = {
        .name = "OSPM1",
        .data = &((struct mod_msg_smt_channel_config){
            .type = MOD_MSG_SMT_CHANNEL_TYPE_REQUESTER,
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_OPTEE_MBX,
                SCMI_CHANNEL_DEVICE_IDX_OSPM_1),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_OPTEE_MBX,
                0),
        }),
    },
    [SCMI_SERVICE_IDX_COUNT] = { 0 },
};

static const struct fwk_element *smt_get_element_table(fwk_id_t module_id)
{
    return (const struct fwk_element *)smt_element_table;
}

struct fwk_module_config config_msg_smt = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(smt_get_element_table),
};

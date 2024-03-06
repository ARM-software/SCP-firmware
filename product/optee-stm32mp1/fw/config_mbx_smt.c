/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022-2024, Linaro Limited and Contributors. All rights
 * reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <scmi_agents.h>

#include <mod_optee_mbx.h>
#include <mod_optee_smt.h>

#include <fwk_element.h>
#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>

#include <arch_main.h>

#include <stdint.h>

#define OSPM0_SMT_MAILBOX_PA   0x2ffff000
#define OSPM0_SMT_MAILBOX_SIZE SCMI_SHMEM_SIZE

static const struct fwk_element mbx_element_table[] = {
    [SCMI_CHANNEL_DEVICE_IDX_NS0] = {
        .name = "SCMI non-secure to OP-TEE channel 0",
        .data = &((struct mod_optee_mbx_channel_config){
            .driver_id = FWK_ID_ELEMENT_INIT(
                FWK_MODULE_IDX_OPTEE_SMT,
                0),
            .driver_api_id = FWK_ID_API_INIT(
                FWK_MODULE_IDX_OPTEE_SMT,
                MOD_OPTEE_SMT_API_IDX_DRIVER_INPUT)
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

static struct fwk_element smt_elt_table[] = {
    [0] = { .name = "OSPM0",
            .data = &((struct mod_optee_smt_channel_config){
                .type = MOD_OPTEE_SMT_CHANNEL_TYPE_REQUESTER,
                .mailbox_address = OSPM0_SMT_MAILBOX_PA,
                .mailbox_size = OSPM0_SMT_MAILBOX_SIZE,
                .driver_id = FWK_ID_ELEMENT_INIT(
                    FWK_MODULE_IDX_OPTEE_MBX,
                    SCMI_CHANNEL_DEVICE_IDX_NS0),
                .driver_api_id = FWK_ID_API_INIT(FWK_MODULE_IDX_OPTEE_MBX, 0),
            }),
    },
    [1] = { 0 },
};

static const struct fwk_element *smt_get_element_table(fwk_id_t module_id)
{
    fwk_assert(fwk_id_get_module_idx(module_id) == FWK_MODULE_IDX_OPTEE_SMT);

    return (const struct fwk_element *)smt_elt_table;
}

struct fwk_module_config config_optee_smt = {
    .elements = FWK_MODULE_DYNAMIC_ELEMENTS(smt_get_element_table),
};

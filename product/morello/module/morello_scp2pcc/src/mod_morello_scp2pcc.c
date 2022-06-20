/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     MORELLO SCP to PCC communications protocol driver
 */

#include "config_cdns_i2c.h"

#include <internal/morello_scp2pcc.h>

#include <mod_cdns_i2c.h>
#include <mod_morello_scp2pcc.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* Module context */
struct morello_scp2pcc_ctx {
    const struct mod_cdns_i2c_controller_api_polled *i2c_api;
};
static struct morello_scp2pcc_ctx morello_scp2pcc_ctx;

static int send_message(
    enum scp2pcc_msg_type type,
    void *req_data,
    uint16_t req_data_size,
    void *resp_data,
    uint16_t *resp_data_size)
{
    int status;
    struct scp2pcc_msg_st msg = { 0 };

    if (type >= MOD_SCP2PCC_MSG_COUNT) {
        return FWK_E_PARAM;
    }

    /* Populate the message parameters */
    msg.opcode = type;

    if (req_data && req_data_size) {
        memcpy(msg.req.data, req_data, req_data_size);
        msg.req.len = req_data_size;
    }

    status = morello_scp2pcc_ctx.i2c_api->write(
        (FWK_ID_ELEMENT(
            FWK_MODULE_IDX_CDNS_I2C, CONFIG_CDNS_I2C_ELEMENT_IDX_PCC)),
        MORELLO_SCP2PCC_I2C_ADDRESS,
        (void *)&msg,
        MORELLO_SCP2PCC_MSG_LEN,
        1);
    if (status != FWK_SUCCESS) {
        return status;
    }

    /* Fetch the response */
    status = morello_scp2pcc_ctx.i2c_api->read(
        (FWK_ID_ELEMENT(
            FWK_MODULE_IDX_CDNS_I2C, CONFIG_CDNS_I2C_ELEMENT_IDX_PCC)),
        MORELLO_SCP2PCC_I2C_ADDRESS,
        (void *)&msg,
        MORELLO_SCP2PCC_MSG_LEN);

    if (status != FWK_SUCCESS) {
        return status;
    }

    if (msg.resp.status != SCP2PCC_MSG_STATUS_SUCCESS) {
        return FWK_E_DEVICE;
    }

    if (msg.resp.len) {
        memcpy(resp_data, msg.resp.data, msg.resp.len);
        *resp_data_size = msg.resp.len;
    }

    return FWK_SUCCESS;
}

static const struct mod_morello_scp2pcc_api morello_scp2pcc_api = {
    .send = send_message,
};

static int morello_scp2pcc_init(
    fwk_id_t module_id,
    unsigned int unused,
    const void *data)
{
    return FWK_SUCCESS;
}

static int morello_scp2pcc_bind(fwk_id_t id, unsigned int round)
{
    return fwk_module_bind(
        FWK_ID_MODULE(FWK_MODULE_IDX_CDNS_I2C),
        FWK_ID_API(FWK_MODULE_IDX_CDNS_I2C, MOD_CDNS_I2C_API_CONTROLLER_POLLED),
        &morello_scp2pcc_ctx.i2c_api);
}

static int morello_scp2pcc_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    *api = &morello_scp2pcc_api;

    return FWK_SUCCESS;
}

static int morello_scp2pcc_start(fwk_id_t id)
{
    return FWK_SUCCESS;
}

const struct fwk_module module_morello_scp2pcc = {
    .api_count = 1,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = morello_scp2pcc_init,
    .bind = morello_scp2pcc_bind,
    .process_bind_request = morello_scp2pcc_process_bind_request,
    .start = morello_scp2pcc_start,
};

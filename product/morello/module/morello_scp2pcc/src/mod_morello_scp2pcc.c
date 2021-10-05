/*
 * Arm SCP/MCP Software
 * Copyright (c) 2021, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     MORELLO SCP to PCC communications protocol driver
 */

#include <internal/morello_scp2pcc.h>

#include <mod_morello_scp2pcc.h>

#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_status.h>

#include <stddef.h>
#include <stdint.h>

/* Module context */
struct morello_scp2pcc_ctx {
    /*  Pointer to module configuration */
    struct mem_msg_config *config;

    /* Sequence variable */
    unsigned int sequence;
};

static struct morello_scp2pcc_ctx scp2pcc_ctx;

static inline void wrdmemset(void *ptr, unsigned int value)
{
    ((unsigned int *)ptr)[0] = value;
}

static void wrdmemcpy(void *destination, const void *source, unsigned int num)
{
    unsigned int index;

    for (index = 0; index < num; index++) {
        ((unsigned int *)destination)[index] = ((unsigned int *)source)[index];
    }
}

static void reset_shared_memory(void)
{
    unsigned int index;
    struct mem_msg_packet *packet = NULL;
    size_t offset;

    for (index = 0; index < scp2pcc_ctx.config->shared_num_rx; index++) {
        offset = index * sizeof(struct mem_msg_packet);
        packet = (struct mem_msg_packet
                      *)(scp2pcc_ctx.config->shared_rx_buffer + offset);

        wrdmemset(&packet->type, MSG_UNUSED_MESSAGE_TYPE);
    }

    for (index = 0; index < scp2pcc_ctx.config->shared_num_tx; index++) {
        offset = index * sizeof(struct mem_msg_packet);
        packet = (struct mem_msg_packet
                      *)(scp2pcc_ctx.config->shared_tx_buffer + offset);

        wrdmemset(&packet->type, MSG_UNUSED_MESSAGE_TYPE);
    }
}

static int mem_msg_send_message(void *data, uint16_t size, uint16_t type)
{
    unsigned int index;
    struct mem_msg_packet *packet = NULL;
    size_t offset;

    if (type == SCP2PCC_TYPE_SHUTDOWN) {
        FWK_LOG_INFO("Shutdown request to PCC");
    }

    /* Check parameters. */
    if ((size > MSG_PAYLOAD_SIZE) || (type == MSG_UNUSED_MESSAGE_TYPE)) {
        FWK_LOG_INFO("Invalid parameters");
        return FWK_E_PARAM;
    }

    /* Check for alive value. */
    if (*(scp2pcc_ctx.config->shared_alive_address) != MSG_ALIVE_VALUE) {
        /* Attempt to set alive value and try again. */
        *(scp2pcc_ctx.config->shared_alive_address) = MSG_ALIVE_VALUE;
        if (*(scp2pcc_ctx.config->shared_alive_address) != MSG_ALIVE_VALUE) {
            return FWK_E_STATE;
        }

        /* If successful, reset shared memory. */
        reset_shared_memory();
    }

    /* Find unused TX packet. */
    for (index = 0; index < scp2pcc_ctx.config->shared_num_tx; index++) {
        offset = index * sizeof(struct mem_msg_packet);
        /* Get pointer to packet. */
        packet = (struct mem_msg_packet
                      *)(scp2pcc_ctx.config->shared_tx_buffer + offset);

        if (packet->type == MSG_UNUSED_MESSAGE_TYPE) {
            /* Unused packet found, copy data payload. */
            if (data != NULL) {
                wrdmemcpy((void *)&packet->payload, data, (size / 4));
            }

            /* Set size. */
            wrdmemset((void *)&packet->size, size);

            /* Set sequence. */
            wrdmemset((void *)&packet->sequence, scp2pcc_ctx.sequence);

            scp2pcc_ctx.sequence++;

            /* Set type last since it is a sort of valid indicator. */
            wrdmemset((void *)&packet->type, type);

            return FWK_SUCCESS;
        }
    }

    return FWK_E_NOMEM;
}

static const struct mod_morello_scp2pcc_api morello_scp2pcc_api = {
    .send = mem_msg_send_message,
};

static int morello_scp2pcc_init(
    fwk_id_t module_id,
    unsigned int unused,
    const void *data)
{
    if (data == NULL) {
        return FWK_E_PARAM;
    }

    scp2pcc_ctx.config = (struct mem_msg_config *)data;

    *(scp2pcc_ctx.config->shared_alive_address) = MSG_ALIVE_VALUE;

    return FWK_SUCCESS;
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
    /* Clear out shared buffers. */
    reset_shared_memory();

    return FWK_SUCCESS;
}

const struct fwk_module module_morello_scp2pcc = {
    .api_count = 1,
    .type = FWK_MODULE_TYPE_PROTOCOL,
    .init = morello_scp2pcc_init,
    .process_bind_request = morello_scp2pcc_process_bind_request,
    .start = morello_scp2pcc_start,
};

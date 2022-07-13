/*
 * Arm SCP/MCP Software
 * Copyright (c) 2022, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Message Handling Unit (MHU) Device Driver.
 */

#include <stddef.h>
#include <stdint.h>
#include <fwk_id.h>
#include <fwk_mm.h>
#include <fwk_arch.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_core.h>
#include <fwk_status.h>
#include <mod_optee_mhu.h>
#include <mod_optee_smt.h>
#include <mod_msg_smt.h>

#include <kernel/mutex.h>

/* MHU device context */
struct mhu_device_ctx {
    /* Channel configuration data */
    struct mod_optee_mhu_channel_config *config;

    fwk_id_t shmem_id;

    union {
        struct mod_optee_smt_driver_input_api *smt;
        struct mod_msg_smt_driver_input_api *msg;
    } shmem_api;

    size_t *shm_out_size;

    struct mutex lock;
};

/* MHU context */
struct mhu_ctx {
    /* Table of device contexts */
    struct mhu_device_ctx *device_ctx_table;

    /* Number of devices in the device context table*/
    unsigned int device_count;
};

static struct mhu_ctx mhu_ctx;

#if BUILD_HAS_MOD_OPTEE_SMT
void optee_mhu_signal_smt_message(fwk_id_t device_id)
{
    struct mhu_device_ctx *device_ctx;
    unsigned int device_idx = fwk_id_get_element_idx(device_id);

    if (device_idx < mhu_ctx.device_count) {
        fwk_assert(fwk_id_get_module_idx(device_ctx->shmem_id) ==
                   FWK_MODULE_IDX_OPTEE_SMT);

        device_ctx = &mhu_ctx.device_ctx_table[device_idx];

        /* Lock the channel until the message has been processed */
        if (device_ctx->config->type == MOD_OPTEE_MHU_CHANNEL_TYPE_REQUESTER) {
            mutex_lock(&device_ctx->lock);
        }

        device_ctx->shmem_api.smt->signal_message(device_ctx->shmem_id);

        if (device_ctx->config->type == MOD_OPTEE_MHU_CHANNEL_TYPE_COMPLETER) {
            /* Wait until notification is ready */
            mutex_lock(&device_ctx->lock);
        }
    } else {
        fwk_unexpected();
    }
}
#endif

#if BUILD_HAS_MOD_MSG_SMT
void optee_mhu_signal_msg_message(fwk_id_t device_id, void *in_buf,
                                  size_t in_size, void *out_buf,
                                  size_t *out_size)
{
    struct mhu_device_ctx *device_ctx;
    unsigned int device_idx = fwk_id_get_element_idx(device_id);

    if (device_idx < mhu_ctx.device_count) {
        device_ctx = &mhu_ctx.device_ctx_table[device_idx];

        fwk_assert(fwk_id_get_module_idx(device_ctx->shmem_id) ==
                   FWK_MODULE_IDX_MSG_SMT);


        /* Lock the channel until the message has been processed */
        if (device_ctx->config->type == MOD_OPTEE_MHU_CHANNEL_TYPE_REQUESTER) {
            mutex_lock(&device_ctx->lock);
        }

        device_ctx->shm_out_size = out_size;
        device_ctx->shmem_api.msg->signal_message(device_ctx->shmem_id,
                                                  in_buf, in_size,
                                                  out_buf, *out_size);

        if (device_ctx->config->type == MOD_OPTEE_MHU_CHANNEL_TYPE_COMPLETER) {
            /* Wait until notification is ready */
            mutex_lock(&device_ctx->lock);
        }
    } else {
        fwk_unexpected();
    }
}
#endif

int optee_mhu_get_devices_count(void)
{
    return mhu_ctx.device_count;
}

fwk_id_t optee_mhu_get_device(unsigned int id)
{
    if (id >= mhu_ctx.device_count)
        return (fwk_id_t)FWK_ID_NONE_INIT;

    return (fwk_id_t)FWK_ID_ELEMENT_INIT(FWK_MODULE_IDX_OPTEE_MHU, id);
}

/*
 * Mailbox module driver API
 */

/*
 * Provide a raise interrupt interface to the Mailbox driver
 */
static int raise_smt_interrupt(fwk_id_t channel_id)
{
    size_t idx = fwk_id_get_element_idx(channel_id);
    struct mhu_device_ctx *channel_ctx = &mhu_ctx.device_ctx_table[idx];

    /* Release the channel as the message has been processed */
    mutex_unlock(&channel_ctx->lock);

    /* There should be a message in the mailbox */
    return FWK_SUCCESS;
}

const struct mod_optee_smt_driver_api mhu_smt_api = {
    .raise_interrupt = raise_smt_interrupt,
};

static int raise_shm_notification(fwk_id_t channel_id, size_t size)
{
    size_t idx = fwk_id_get_element_idx(channel_id);
    struct mhu_device_ctx *channel_ctx = &mhu_ctx.device_ctx_table[idx];

    *channel_ctx->shm_out_size = size;

    /* Release the channel as the message has been processed */
    mutex_unlock(&channel_ctx->lock);

    return FWK_SUCCESS;
}

const struct mod_msg_smt_driver_ouput_api mhu_shm_api = {
    .raise_notification = raise_shm_notification,
};

/*
 * Framework handlers
 */

static int mhu_init(fwk_id_t module_id, unsigned int device_count,
                    const void *data)
{

    if (device_count == 0)
        return FWK_E_PARAM;

    mhu_ctx.device_ctx_table = fwk_mm_calloc(device_count,
                                             sizeof(*mhu_ctx.device_ctx_table));
    if (mhu_ctx.device_ctx_table == NULL) {
        return FWK_E_NOMEM;
    }

    mhu_ctx.device_count = device_count;

    return FWK_SUCCESS;
}

static int mhu_device_init(fwk_id_t device_id, unsigned int slot_count,
                           const void *data)
{
    size_t elt_idx = fwk_id_get_element_idx(device_id);
    struct mhu_device_ctx *device_ctx = &mhu_ctx.device_ctx_table[elt_idx];

    device_ctx->config = (struct mod_optee_mhu_channel_config*)data;

    /* Validate channel config */
    if (device_ctx->config->type >= MOD_OPTEE_MHU_CHANNEL_TYPE_COUNT) {
        assert(false);
        return FWK_E_DATA;
    }

    mutex_init(&device_ctx->lock);
    if (device_ctx->config->type == MOD_OPTEE_MHU_CHANNEL_TYPE_COMPLETER) {
        /* lock the notification message */
        mutex_lock(&device_ctx->lock);
    }

    /*
     * Request the creation of an execution context for the device so we can
     * paralellize unrelated request.
     */
    return FWK_INIT_CTX;
}

static int mhu_bind(fwk_id_t id, unsigned int round)
{
    int status;
    struct mhu_device_ctx *device_ctx;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    if (round == 1) {
        device_ctx = &mhu_ctx.device_ctx_table[fwk_id_get_element_idx(id)];

        status = fwk_module_bind(device_ctx->config->driver_id,
                                 device_ctx->config->driver_api_id,
                                 &device_ctx->shmem_api);
        if (status != FWK_SUCCESS) {
            return status;
        }

        device_ctx->shmem_id = device_ctx->config->driver_id;
    }

    return FWK_SUCCESS;
}

static int mhu_process_bind_request(fwk_id_t source_id,
                                    fwk_id_t target_id,
                                    fwk_id_t api_id,
                                    const void **api)
{
    size_t elt_idx;

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT)) {
        return FWK_E_ACCESS;
    }

    if (fwk_id_get_api_idx(api_id) != 0) {
        return FWK_E_PARAM;
    }

    elt_idx = fwk_id_get_element_idx(target_id);
    if (elt_idx >= mhu_ctx.device_count) {
        return FWK_E_PARAM;
    }

    switch (fwk_id_get_module_idx(source_id)) {
#if BUILD_HAS_MOD_OPTEE_SMT
    case FWK_MODULE_IDX_OPTEE_SMT:
        *api = &mhu_smt_api;
        break;
#endif
#if BUILD_HAS_MOD_MSG_SMT
    case FWK_MODULE_IDX_MSG_SMT:
        *api = &mhu_shm_api;
        break;
#endif
    default:
        return FWK_E_PANIC;
    }

    return FWK_SUCCESS;
}

static int mhu_start(fwk_id_t id)
{
    return FWK_SUCCESS;
}

/* OPTEE_MHU module definition */
const struct fwk_module module_optee_mhu = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = 1,
    .init = mhu_init,
    .element_init = mhu_device_init,
    .bind = mhu_bind,
    .start = mhu_start,
    .process_bind_request = mhu_process_bind_request,
};

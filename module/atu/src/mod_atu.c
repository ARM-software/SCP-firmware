/*
 * Arm SCP/MCP Software
 * Copyright (c) 2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *      Address Translation Unit (ATU) device driver.
 */

#include <internal/atu.h>

#include <mod_atu.h>

#include <fwk_id.h>
#include <fwk_module.h>
#include <fwk_status.h>

/* ATU module context */
struct mod_atu_ctx atu_ctx;

/*
 * Framework handlers
 */
static int mod_atu_init(
    fwk_id_t module_id,
    unsigned int device_count,
    const void *unused)
{
    if (device_count == 0) {
        return FWK_E_PARAM;
    }
    atu_ctx.device_ctx_table =
        fwk_mm_calloc(device_count, sizeof(atu_ctx.device_ctx_table[0]));

    atu_ctx.device_count = device_count;

    return FWK_SUCCESS;
}

static int mod_atu_device_init(
    fwk_id_t device_id,
    unsigned int sub_element_count,
    const void *data)
{
    struct mod_atu_device_config *config;
    struct atu_device_ctx *device_ctx;

    config = (struct mod_atu_device_config *)data;

    if (config == NULL) {
        return FWK_E_DATA;
    }

    device_ctx = &atu_ctx.device_ctx_table[fwk_id_get_element_idx(device_id)];
    device_ctx->config = config;

    /* Pick implementation specific framework helper functions */
    if (config->is_atu_delegated) {
#if defined(BUILD_HAS_ATU_DELEGATE)
        atu_get_delegate_ops(&device_ctx->atu_ops);
#endif
    } else {
#if defined(BUILD_HAS_ATU_MANAGE)
        atu_get_manage_ops(&device_ctx->atu_ops);
#endif
    }

    /* Initialize the shared module context pointer */
    device_ctx->atu_ops->atu_init_shared_ctx(&atu_ctx);

    return device_ctx->atu_ops->atu_device_init(device_id);
}

static int mod_atu_bind(fwk_id_t id, unsigned int round)
{
    struct atu_device_ctx *device_ctx;

    if ((round != 0) || (fwk_id_is_type(id, FWK_ID_TYPE_MODULE))) {
        return FWK_SUCCESS;
    }

    device_ctx = &atu_ctx.device_ctx_table[fwk_id_get_element_idx(id)];

    if (device_ctx->atu_ops->atu_bind != NULL) {
        /* Invoke implementation specific framework helper function */
        return device_ctx->atu_ops->atu_bind(device_ctx);
    }

    return FWK_SUCCESS;
}

static int mod_atu_process_bind_request(
    fwk_id_t source_id,
    fwk_id_t target_id,
    fwk_id_t api_id,
    const void **api)
{
    enum mod_atu_api_idx api_id_type;
    struct atu_device_ctx *device_ctx;
    int status;

    if (!fwk_id_is_type(target_id, FWK_ID_TYPE_ELEMENT)) {
        /*  Only binding to an element is allowed */
        fwk_unexpected();
        return FWK_E_ACCESS;
    }

    device_ctx = &atu_ctx.device_ctx_table[fwk_id_get_element_idx(target_id)];

    api_id_type = (enum mod_atu_api_idx)fwk_id_get_api_idx(api_id);

    switch (api_id_type) {
    case MOD_ATU_API_IDX_ATU:
        *api = device_ctx->atu_api;
        status = FWK_SUCCESS;
        break;

#if defined(BUILD_HAS_ATU_DELEGATE)
    case MOD_ATU_API_IDX_TRANSPORT_SIGNAL:
        if (!fwk_id_is_equal(source_id, device_ctx->config->transport_id)) {
            status = FWK_E_ACCESS;
        } else {
            atu_get_signal_api(api);
            status = FWK_SUCCESS;
        }
        break;
#endif

    default:
        status = FWK_E_PARAM;
    }

    return status;
}

static int mod_atu_start(fwk_id_t id)
{
    struct atu_device_ctx *device_ctx;

    if (fwk_id_is_type(id, FWK_ID_TYPE_MODULE)) {
        return FWK_SUCCESS;
    }

    device_ctx = &atu_ctx.device_ctx_table[fwk_id_get_element_idx(id)];

    if (device_ctx->atu_ops->atu_start != NULL) {
        /* Invoke implementation specific framework handler function */
        return device_ctx->atu_ops->atu_start(device_ctx);
    }

    return FWK_SUCCESS;
}

/* ATU module definition */
const struct fwk_module module_atu = {
    .type = FWK_MODULE_TYPE_DRIVER,
    .api_count = MOD_ATU_API_IDX_COUNT,
    .init = mod_atu_init,
    .element_init = mod_atu_device_init,
    .bind = mod_atu_bind,
    .process_bind_request = mod_atu_process_bind_request,
    .start = mod_atu_start,
};

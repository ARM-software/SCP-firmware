/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019-2022, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Description:
 *     Reset domain HAL
 */

#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <mod_reset_domain.h>

/*
 * Module and devices contexts for Reset Domain
 */

/* Device context */
struct rd_dev_ctx {
    const struct mod_reset_domain_dev_config *config;
    struct mod_reset_domain_drv_api *driver_api;
};

/* Module context */
struct mod_rd_ctx {
    const struct mod_reset_domain_config *config;
    struct rd_dev_ctx *dev_ctx_table;
    unsigned int dev_count;
};

/*
 * Internal variables
 */
static struct mod_rd_ctx module_reset_ctx;

/*
 * API functions
 */
static int set_reset_state(fwk_id_t reset_dev_id,
                           enum mod_reset_domain_mode mode,
                           uint32_t reset_state,
                           uintptr_t cookie)
{
    struct rd_dev_ctx *reset_ctx;
    unsigned int reset_domain_idx = fwk_id_get_element_idx(reset_dev_id);

    reset_ctx = &module_reset_ctx.dev_ctx_table[reset_domain_idx];

    return reset_ctx->driver_api->set_reset_state(reset_ctx->config->driver_id,
                                                  mode, reset_state, cookie);
}

/* HAL API */
static const struct mod_reset_domain_api reset_api = {
    .set_reset_state = set_reset_state,
};

#ifdef BUILD_HAS_NOTIFICATION
static int reset_issued_notify(fwk_id_t dev_id,
                               uint32_t reset_state,
                               uintptr_t cookie)
{
    int domain_id = -1;
    unsigned int i;
    struct rd_dev_ctx *reset_ctx;
    unsigned int notification_count;
    struct fwk_event notification_event = {
        .id = module_reset_ctx.config->notification_id,
        .source_id = fwk_module_id_reset_domain,
    };

    struct mod_reset_domain_notification_event_params* params =
        (struct mod_reset_domain_notification_event_params*)
        notification_event.params;

    /* Loop through device context table to get the associated domain_id */
    for (i = 0; i < module_reset_ctx.dev_count; i++) {
        reset_ctx = &module_reset_ctx.dev_ctx_table[i];
        if (fwk_id_is_equal(reset_ctx->config->driver_id, dev_id)) {
            domain_id = (int)i;
            break;
        }
    }

    if (domain_id < 0)
        return FWK_E_PARAM;

    params->domain_id = (uint32_t)domain_id;
    params->reset_state = reset_state;
    params->cookie = cookie;

    return fwk_notification_notify(&notification_event, &notification_count);
}
#endif /* BUILD_HAS_NOTIFICATION */

static int rd_process_event(
    const struct fwk_event *event,
    struct fwk_event *resp)
{
#ifdef BUILD_HAS_NOTIFICATION
    struct mod_reset_domain_autoreset_event_params* params =
        (struct mod_reset_domain_autoreset_event_params*)event->params;
#endif

    if (!fwk_id_is_equal(mod_reset_domain_autoreset_event_id,
                         event->id))
        return FWK_E_SUPPORT;

#ifdef BUILD_HAS_NOTIFICATION
    return reset_issued_notify(params->dev_id,
                               params->reset_state, params->cookie);
#else
    return FWK_SUCCESS;
#endif
}

/*
 * Framework handlers
 */
static int rd_init(fwk_id_t module_id,
                   unsigned int dev_count,
                   const void *data)
{
    module_reset_ctx.config = (struct mod_reset_domain_config *)data;
    module_reset_ctx.dev_ctx_table = fwk_mm_calloc(dev_count,
                                             sizeof(struct rd_dev_ctx));
    module_reset_ctx.dev_count = dev_count;

    return FWK_SUCCESS;
}

static int rd_element_init(fwk_id_t element_id,
                           unsigned int sub_dev_count,
                           const void *data)
{
    struct rd_dev_ctx *reset_ctx = NULL;

    reset_ctx = &module_reset_ctx.dev_ctx_table[
        fwk_id_get_element_idx(element_id)];

    reset_ctx->config = (const struct mod_reset_domain_dev_config *)data;

    return FWK_SUCCESS;
}

static int rd_bind(fwk_id_t id, unsigned int round)
{
    struct rd_dev_ctx *reset_ctx = NULL;

    /* Nothing to do during subsequent round of calls */
    if (round != 0)
        return FWK_SUCCESS;

    if (!fwk_id_is_type(id, FWK_ID_TYPE_ELEMENT))
        return FWK_SUCCESS;

    reset_ctx = module_reset_ctx.dev_ctx_table + fwk_id_get_element_idx(id);

    return fwk_module_bind(reset_ctx->config->driver_id,
                           reset_ctx->config->driver_api_id,
                           &reset_ctx->driver_api);
}

static int rd_process_bind_request(fwk_id_t source_id,
                                   fwk_id_t target_id,
                                   fwk_id_t api_id,
                                   const void **api)
{
    enum mod_reset_domain_api_type api_id_type =
        (enum mod_reset_domain_api_type)fwk_id_get_api_idx(api_id);

    switch (api_id_type) {
    case MOD_RESET_DOMAIN_API_TYPE_HAL:
        *api = &reset_api;
        break;

    default:
        return FWK_E_ACCESS;
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_reset_domain = {
    .type = FWK_MODULE_TYPE_HAL,
    .api_count = (unsigned int)MOD_RESET_DOMAIN_API_COUNT,
#ifdef BUILD_HAS_NOTIFICATION
    .notification_count = (unsigned int)MOD_RESET_DOMAIN_NOTIFICATION_IDX_COUNT,
#endif
    .event_count = (unsigned int)MOD_RESET_DOMAIN_EVENT_IDX_COUNT,
    .init = rd_init,
    .element_init = rd_element_init,
    .bind = rd_bind,
    .process_bind_request = rd_process_bind_request,
    .process_event = rd_process_event,
};

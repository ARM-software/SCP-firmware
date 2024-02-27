/*
 * Arm SCP/MCP Software
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include <noc_s3.h>
#include <noc_s3_psam.h>
#include <noc_s3_reg.h>

#include <mod_noc_s3.h>

#include <fwk_assert.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_macros.h>
#include <fwk_mm.h>
#include <fwk_module.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <stdlib.h>

struct mod_noc_s3_element_ctx {
    /* Points to the configuration of the element. */
    struct mod_noc_s3_element_config *config;
    /* NoC S3 device handler. */
    struct mod_noc_s3_dev noc_s3_dev;
};

struct mod_noc_s3_ctx {
    /* List of the element's context. */
    struct mod_noc_s3_element_ctx *element_ctx;
    /* Number of elements. */
    unsigned int element_count;
};

struct mod_noc_s3_ctx noc_s3_ctx;

static int mod_noc_s3_init(
    fwk_id_t module_id,
    unsigned int element_count,
    const void *unused)
{
    if (element_count == 0) {
        /* Configuration will be done during runtime. */
        return FWK_SUCCESS;
    }

    noc_s3_ctx.element_ctx =
        fwk_mm_calloc(element_count, sizeof(struct mod_noc_s3_element_ctx));
    if (noc_s3_ctx.element_ctx == NULL) {
        return FWK_E_NOMEM;
    }

    noc_s3_ctx.element_count = element_count;

    return FWK_SUCCESS;
}

static int mod_noc_s3_element_init(
    fwk_id_t element_id,
    unsigned int unused,
    const void *data)
{
    struct mod_noc_s3_element_config *config;
    unsigned int idx;

    config = (struct mod_noc_s3_element_config *)data;
    idx = fwk_id_get_element_idx(element_id);
    noc_s3_ctx.element_ctx[idx].config = config;
    noc_s3_ctx.element_ctx[idx].noc_s3_dev.periphbase = config->periphbase;

    return FWK_SUCCESS;
}

static int mod_noc_s3_start(fwk_id_t id)
{
    struct mod_noc_s3_element_config *config;
    struct mod_noc_s3_dev *dev;
    unsigned int element_id;
    int status;

    if (fwk_id_get_type(id) == FWK_ID_TYPE_MODULE) {
        return FWK_SUCCESS;
    }

    if (!fwk_module_is_valid_element_id(id)) {
        return FWK_E_PARAM;
    }

    element_id = fwk_id_get_element_idx(id);
    config = noc_s3_ctx.element_ctx[element_id].config;
    dev = &noc_s3_ctx.element_ctx[element_id].noc_s3_dev;
    if (!fwk_id_is_equal(config->plat_notification.source_id, FWK_ID_NONE)) {
        status = fwk_notification_subscribe(
            config->plat_notification.notification_id,
            config->plat_notification.source_id,
            id);
        if (status != FWK_SUCCESS) {
            return status;
        }
    } else {
        status = program_static_mapped_regions(config, dev);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

static int mod_noc_s3_process_notification(
    const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct mod_noc_s3_element_config *config;
    struct mod_noc_s3_dev *dev;
    unsigned int element_id;
    int status;

    element_id = fwk_id_get_element_idx(event->target_id);
    config = noc_s3_ctx.element_ctx[element_id].config;
    dev = &noc_s3_ctx.element_ctx[element_id].noc_s3_dev;
    if (fwk_id_is_equal(event->id, config->plat_notification.notification_id)) {
        status = program_static_mapped_regions(config, dev);
        if (status != FWK_SUCCESS) {
            fwk_trap();
            return status;
        }

        /* Element is initialized, unsubscribe from the notification. */
        status = fwk_notification_unsubscribe(
            event->id, event->source_id, event->target_id);
        if (status != FWK_SUCCESS) {
            return status;
        }
    }

    return FWK_SUCCESS;
}

static struct mod_noc_s3_memmap_api noc_s3_memmap_api = {
    .map_region_in_psam = map_region_in_psam,
    .unmap_region_in_psam = unmap_region_in_psam,
};

static int mod_noc_s3_process_bind_request(
    fwk_id_t requester_id,
    fwk_id_t targer_id,
    fwk_id_t api_id,
    const void **api)
{
    enum mod_noc_s3_api_idx api_idx;
    int status;

    api_idx = fwk_id_get_api_idx(api_id);
    switch (api_idx) {
    case MOD_NOC_S3_API_SETUP_PSAM:
        *api = &noc_s3_memmap_api;
        status = FWK_SUCCESS;
        break;
    default:
        status = FWK_E_DATA;
    };

    return status;
}

const struct fwk_module module_noc_s3 = {
    .api_count = MOD_NOC_S3_API_COUNT,
    .type = FWK_MODULE_TYPE_DRIVER,
    .init = mod_noc_s3_init,
    .element_init = mod_noc_s3_element_init,
    .start = mod_noc_s3_start,
    .process_bind_request = mod_noc_s3_process_bind_request,
    .process_notification = mod_noc_s3_process_notification,
};

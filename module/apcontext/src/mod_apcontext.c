/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2023, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_apcontext.h>
#include <mod_clock.h>

#include <fwk_assert.h>
#include <fwk_event.h>
#include <fwk_id.h>
#include <fwk_log.h>
#include <fwk_module.h>
#include <fwk_module_idx.h>
#include <fwk_notification.h>
#include <fwk_status.h>

#include <inttypes.h>
#include <string.h>

#define MODULE_NAME "[APContext]"

/* Module context structure */
struct apcontext_ctx {
    /*
     * Number of notifications subscribed and to wait for before zeroing the
     * AP context memory region.
     */
    unsigned int wait_on_notifications;
};

/* Module context */
static struct apcontext_ctx ctx;

static void apcontext_zero(void)
{
    const struct mod_apcontext_config *config;

    config = fwk_module_get_data(fwk_module_id_apcontext);

    FWK_LOG_INFO(
        MODULE_NAME " Zeroing AP context area [0x%" PRIxPTR " - 0x%" PRIxPTR
                    "]",
        config->base,
        config->base + config->size);

    memset((void *)config->base, 0, config->size);
}

/*
 * Framework handlers
 */

static int apcontext_init(fwk_id_t module_id, unsigned int element_count,
    const void *data)
{
    const struct mod_apcontext_config *config = data;

    /* This module does not support elements */
    if (element_count != 0)
        return FWK_E_PARAM;

    if (config->base == 0)
        return FWK_E_DATA;

    if (config->size == 0)
        return FWK_E_DATA;

    return FWK_SUCCESS;
}

static int apcontext_start(fwk_id_t id)
{
    const struct mod_apcontext_config *config =
        fwk_module_get_data(fwk_module_id_apcontext);
    int status;

    if (!fwk_id_is_equal(config->clock_id, FWK_ID_NONE)) {
        status = fwk_notification_subscribe(
            mod_clock_notification_id_state_changed, config->clock_id, id);
        if (status != FWK_SUCCESS) {
            FWK_LOG_CRIT(MODULE_NAME
                         "Failed to subscribe to clock "
                         "notification!");
            return status;
        }
        ctx.wait_on_notifications++;
    }

    if ((fwk_id_type_is_valid(config->platform_notification.source_id)) &&
        (!fwk_id_is_equal(
            config->platform_notification.source_id, FWK_ID_NONE))) {
        status = fwk_notification_subscribe(
            config->platform_notification.notification_id,
            config->platform_notification.source_id,
            id);
        if (status != FWK_SUCCESS) {
            FWK_LOG_CRIT(MODULE_NAME
                         "Failed to subscribe to platform "
                         "notification!");
            return status;
        }
        ctx.wait_on_notifications++;
    }

    if (ctx.wait_on_notifications == 0) {
        apcontext_zero();
    }

    return FWK_SUCCESS;
}

static int apcontext_process_notification(const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    const struct mod_apcontext_config *config =
        fwk_module_get_data(fwk_module_id_apcontext);
    struct clock_notification_params *params;
    int status;

    fwk_assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE));
    fwk_assert(ctx.wait_on_notifications != 0);

    if (fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed)) {
        params = (struct clock_notification_params *)event->params;
        if (params->new_state == MOD_CLOCK_STATE_RUNNING) {
            /* Unsubscribe to the notification */
            status = fwk_notification_unsubscribe(
                event->id, event->source_id, event->target_id);
            if (status != FWK_SUCCESS) {
                FWK_LOG_CRIT(MODULE_NAME
                             "Failed to unsubscribe clock "
                             "notification");
                return status;
            }
            ctx.wait_on_notifications--;
        }
    }

    if (fwk_id_is_equal(
            event->id, config->platform_notification.notification_id)) {
        /* Unsubscribe to the notification */
        status = fwk_notification_unsubscribe(
            event->id, event->source_id, event->target_id);
        if (status != FWK_SUCCESS) {
            FWK_LOG_CRIT(MODULE_NAME
                         "Failed to unsubscribe platform "
                         "notification");
            return status;
        }
        ctx.wait_on_notifications--;
    }

    if (ctx.wait_on_notifications == 0) {
        /* Zero AP context area */
        apcontext_zero();
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_apcontext = {
    .type = FWK_MODULE_TYPE_SERVICE,
    .init = apcontext_init,
    .start = apcontext_start,
    .process_notification = apcontext_process_notification,
};

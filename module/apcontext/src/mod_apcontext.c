/*
 * Arm SCP/MCP Software
 * Copyright (c) 2018-2020, Arm Limited and Contributors. All rights reserved.
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

    if (fwk_id_is_equal(config->clock_id, FWK_ID_NONE)) {
        apcontext_zero();
        return FWK_SUCCESS;
    }

    /* Register the module for clock state notifications */
    return fwk_notification_subscribe(
        mod_clock_notification_id_state_changed,
        config->clock_id,
        id);
}

static int apcontext_process_notification(const struct fwk_event *event,
    struct fwk_event *resp_event)
{
    struct clock_notification_params *params;

    assert(fwk_id_is_equal(event->id, mod_clock_notification_id_state_changed));
    assert(fwk_id_is_type(event->target_id, FWK_ID_TYPE_MODULE));

    params = (struct clock_notification_params *)event->params;

    /*
     * Zero AP context area when the system is initialized for the first time
     * only
     */
    if (params->new_state == MOD_CLOCK_STATE_RUNNING) {
        apcontext_zero();

        /* Unsubscribe to the notification */
        return fwk_notification_unsubscribe(event->id, event->source_id,
                                            event->target_id);
    }

    return FWK_SUCCESS;
}

const struct fwk_module module_apcontext = {
    .name = "APContext",
    .type = FWK_MODULE_TYPE_SERVICE,
    .init = apcontext_init,
    .start = apcontext_start,
    .process_notification = apcontext_process_notification,
};

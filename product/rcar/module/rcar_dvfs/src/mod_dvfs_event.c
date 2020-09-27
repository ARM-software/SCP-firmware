/*
 * Renesas SCP/MCP Software
 * Copyright (c) 2020, Renesas Electronics Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <mod_dvfs_private.h>

#include <fwk_macros.h>

static int event_set_opp(
    const struct fwk_event *event,
    struct fwk_event *response)
{
    return FWK_E_SUPPORT;
}

static int event_set_frequency_limits(
    const struct fwk_event *event,
    struct fwk_event *response)
{
    return FWK_E_SUPPORT;
}

int __mod_dvfs_process_event(
    const struct fwk_event *event,
    struct fwk_event *response)
{
    typedef int (*handler_t)(
        const struct fwk_event *event, struct fwk_event *response);

    static const handler_t handlers[] = {
        [MOD_DVFS_EVENT_IDX_SET_FREQUENCY] = event_set_opp,
        [MOD_DVFS_EVENT_IDX_SET_FREQUENCY_LIMITS] = event_set_frequency_limits,
    };

    handler_t handler;

    /* Ensure we have a handler implemented for this event */
    handler = handlers[fwk_id_get_event_idx(event->id)];
    if (handler == NULL)
        return FWK_E_PARAM;

    /* Delegate event handling to the relevant handler */
    return handler(event, response);
}

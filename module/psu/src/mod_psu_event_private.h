/*
 * Arm SCP/MCP Software
 * Copyright (c) 2017-2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MOD_PSU_EVENT_PRIVATE_H
#define MOD_PSU_EVENT_PRIVATE_H

#include <fwk_event.h>
#include <fwk_id.h>

/* "Set enabled" event */
struct mod_psu_event_params_set_enabled {
    bool enable;
};

/* "Set voltage" event */
struct mod_psu_event_params_set_voltage {
    uintmax_t voltage;
};

/* Event handler */
int __mod_psu_process_event(
    const struct fwk_event *event,
    struct fwk_event *response);

#endif /* MOD_PSU_EVENT_PRIVATE_H */
